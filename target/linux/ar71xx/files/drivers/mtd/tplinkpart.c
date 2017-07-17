/*
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * History of file change (latest first)
 *
 * rajesh@aurlis.com - 31052017 - Modified for Hotgen Recoil project
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/magic.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#define TPLINK_NUM_PARTS				7
#define TPLINK_KERNEL_OFFS				0x30000		/* offset to first firmware bank 	*/
#define TPLINK_HOTGEN_OFFS				0x20000		/* offset to hotgen header partition 	*/

#define TPLINK_HEADER_V1				0x01000000
#define TPLINK_HEADER_V2				0x02000000
#define MD5SUM_LEN				        16
#define TPLINK_ART_LEN					0x10000

#define TPLINK_FLASH_SIZE	    		0x800000    /* 8MB 	*/
#define TPLINK_UBOOT_SIZE	    		0x20000	    /* 128KB 	*/
#define TPLINK_HOTGEN_SIZE      		0x10000	    /* 64KB 	*/
#define TPLINK_FIRMWARE_PARTITION_SIZE	        0x3e0000    /* 62 blocks of 64KB */

#define HOTGEN_FIRMWARE_BANK_MASK		0x3
struct tplink_hotgen_header {
	uint32_t	flags;			/* header flags         */
	uint32_t	F1_offset;		/* offset to firmware 1 */
	uint32_t	F2_offset;		/* offset to firmware 2 */
}__attribute__ ((packed));

typedef enum
{
	bank_min = 0,
	bank_1 = 1,
	bank_2 = 2,
	bank_max = 3
}HotgenFirmwareBank;

struct tplink_fw_header {
	uint32_t	version;	/* header version */
	char		vendor_name[24];
	char		fw_version[36];
	uint32_t	hw_id;		/* hardware id */
	uint32_t	hw_rev;		/* hardware revision */
	uint32_t	unk1;
	uint8_t		md5sum1[MD5SUM_LEN];
	uint32_t	unk2;
	uint8_t		md5sum2[MD5SUM_LEN];
	uint32_t	unk3;
	uint32_t	kernel_la;	/* kernel load address */
	uint32_t	kernel_ep;	/* kernel entry point */
	uint32_t	fw_length;	/* total length of the firmware */
	uint32_t	kernel_ofs;	/* kernel data offset */
	uint32_t	kernel_len;	/* kernel data length */
	uint32_t	rootfs_ofs;	/* rootfs data offset */
	uint32_t	rootfs_len;	/* rootfs data length */
	uint32_t	boot_ofs;	/* bootloader data offset */
	uint32_t	boot_len;	/* bootloader data length */
	uint8_t		pad[360];
} __attribute__ ((packed));

static struct tplink_fw_header* tplink_read_header(struct mtd_info *mtd, size_t offset)
{
	struct tplink_fw_header *header;
	size_t header_len;
	size_t retlen;
	int ret;
	u32 t;

	header = vmalloc(sizeof(*header));
	if (!header)
		goto err;

	header_len = sizeof(struct tplink_fw_header);
	ret = mtd_read(mtd, offset, header_len, &retlen, (unsigned char *) header);
	if (ret)
	{
		goto err_free_header;
	}
	if (retlen != header_len)
	{
		goto err_free_header;
	}

	/* sanity checks */
	t = be32_to_cpu(header->version);
	if ((t != TPLINK_HEADER_V1) && (t != TPLINK_HEADER_V2))
	{
		goto err_free_header;
	}

	t = be32_to_cpu(header->kernel_ofs);
	if (t != header_len)
	{
		goto err_free_header;
	}

	return header;

err_free_header:
	vfree(header);
err:
	return NULL;
}

static struct tplink_fw_header* tplink_read_hotgen_header(struct mtd_info *mtd, size_t *offset, HotgenFirmwareBank *bank)
{
	struct tplink_hotgen_header *header;
	size_t header_len;
	size_t retlen;
	int ret;
	u32 flags, f1offset, f2offset;

	*bank = bank_min; /* default to invalid */

	header = vmalloc(sizeof(*header));
	if (!header)
		goto err;

	header_len = sizeof(struct tplink_hotgen_header);
	ret = mtd_read(mtd, *offset, header_len, &retlen, (unsigned char *) header);
	if (ret)
	{
		goto err_free_header;
	}	

	if (retlen != header_len)
	{
		goto err_free_header;
	}	

	f1offset = be32_to_cpu(header->F1_offset);
	if (f1offset >= (8*1024*1024))
		goto err_free_header;
	
	f2offset = be32_to_cpu(header->F2_offset);
	if (f2offset >= (8*1024*1024))
		goto err_free_header;

	flags = be32_to_cpu(header->flags);

	if ((flags & HOTGEN_FIRMWARE_BANK_MASK) == bank_1)
	{
		*offset = f1offset;
		*bank = bank_1;
	}
	else if ((flags & HOTGEN_FIRMWARE_BANK_MASK) == bank_2)
	{
		*offset = f2offset;
		*bank = bank_2;
	}
	else
	{
		goto err_free_header;
	}
	vfree(header);
	return tplink_read_header (mtd, *offset);

err_free_header:
	vfree(header);
err:
	return NULL;
}

static int tplink_check_rootfs_magic(struct mtd_info *mtd, size_t offset)
{
	u32 magic;
	size_t retlen;
	int ret;

	ret = mtd_read(mtd, offset, sizeof(magic), &retlen,
		       (unsigned char *) &magic);
	if (ret)
		return ret;

	if (retlen != sizeof(magic))
		return -EIO;

	if (le32_to_cpu(magic) != SQUASHFS_MAGIC &&
	    magic != 0x19852003)
		return -EINVAL;

	return 0;
}

static int tplink_parse_partitions_offset(
				struct mtd_info *master,
				struct mtd_partition **pparts,
				struct mtd_part_parser_data *data, /* not used */
				size_t offset)
{
	struct mtd_partition *parts;
	struct tplink_fw_header *header;
	int nr_parts;
	size_t art_offset;
	size_t rootfs_offset;
	size_t squashfs_offset;
	int ret;
	HotgenFirmwareBank bank;

	nr_parts = TPLINK_NUM_PARTS;
	parts = kzalloc(nr_parts * sizeof(struct mtd_partition), GFP_KERNEL);
	if (!parts) {
		ret = -ENOMEM;
		goto err;
	}

	header = tplink_read_hotgen_header(master, &offset, &bank);	/* offset is hotgen partition start address */
	if (!header) {
		printk(KERN_ERR "%s: no TP-Link header found\n", master->name);
		ret = -ENODEV;
		goto err_free_parts;
	}
	else if (bank <= bank_min || bank >= bank_max) {
		printk(KERN_ERR "Hotgen header partition corrupted !!! bank(%d)\n", bank);
		ret = -ENODEV;
		goto err_free_parts;
	}
	else {
		printk(KERN_INFO "Hotgen firmware to boot from bank(%d) offset(%x)\n", bank, offset);
	}

	squashfs_offset = offset + sizeof(struct tplink_fw_header) + be32_to_cpu(header->kernel_len);

	ret = tplink_check_rootfs_magic(master, squashfs_offset);
	if (ret == 0)
		rootfs_offset = squashfs_offset;
	else
		rootfs_offset = offset + be32_to_cpu(header->rootfs_ofs);

	art_offset = TPLINK_FLASH_SIZE - TPLINK_ART_LEN;

	parts[0].name = "u-boot";
	parts[0].offset = 0;
	parts[0].size = TPLINK_UBOOT_SIZE;
	parts[0].mask_flags = MTD_WRITEABLE;

	parts[1].name = "hotgen";
	parts[1].offset = parts[0].size;
	parts[1].size = TPLINK_HOTGEN_SIZE;

	parts[2].name = "kernel";
	parts[2].offset = offset;
	parts[2].size = (rootfs_offset - offset);

	parts[3].name = "rootfs";
	parts[3].offset = rootfs_offset;
	parts[3].size = ((offset + TPLINK_FIRMWARE_PARTITION_SIZE) - rootfs_offset);

	parts[4].name = "active";
	parts[4].offset = offset;
	parts[4].size = TPLINK_FIRMWARE_PARTITION_SIZE;
	parts[4].mask_flags = MTD_WRITEABLE;

	parts[5].name = "upgrade";
	if (bank == bank_1)	{
		parts[5].offset = (offset + TPLINK_FIRMWARE_PARTITION_SIZE);
	}
	else if (bank == bank_2)
	{
		parts[5].offset = (offset - TPLINK_FIRMWARE_PARTITION_SIZE);
	}
	parts[5].size = TPLINK_FIRMWARE_PARTITION_SIZE;

	parts[6].name = "art";
	parts[6].offset = art_offset;
	parts[6].size = TPLINK_ART_LEN;
	parts[6].mask_flags = MTD_WRITEABLE;

	vfree(header);

	*pparts = parts;
	return nr_parts;

err_free_parts:
	kfree(parts);
err:
	*pparts = NULL;
	return ret;
}

static int tplink_parse_partitions(struct mtd_info *master,
				   struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	return tplink_parse_partitions_offset(master, pparts, data, TPLINK_HOTGEN_OFFS);
}

static int tplink_parse_64k_partitions(struct mtd_info *master,
				   struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	return tplink_parse_partitions_offset(master, pparts, data, TPLINK_HOTGEN_OFFS);
}

static struct mtd_part_parser tplink_parser = {
	.owner		= THIS_MODULE,
	.parse_fn	= tplink_parse_partitions,
	.name		= "tp-link",
};

static struct mtd_part_parser tplink_64k_parser = {
	.owner		= THIS_MODULE,
	.parse_fn	= tplink_parse_64k_partitions,
	.name		= "tp-link-64k",
};

static int __init tplink_parser_init(void)
{
	register_mtd_parser(&tplink_parser);
	register_mtd_parser(&tplink_64k_parser);

	return 0;
}

module_init(tplink_parser_init);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
