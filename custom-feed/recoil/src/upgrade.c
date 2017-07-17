//
//  Copyright (c) 2017, Aurlis Ltd (www.aurlis.com)
//  filename    :- upgrade.c
//  description :- Recoil Network Upgrade service
//  author      :- Rajesh Gunasekaran   (rajesh@aurlis.com)
//

/*
* ------------------------------------ Header files  -----------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

//#define __DEBUG
//#define __DEBUG_EXT_INFO
//#define __DEBUG_IN
//#define __DEBUG_OUT
#include "recoilnetwork.h"
#if defined(FORCE_RANDOM_MAC)
#include <time.h>
#endif

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/

#define MAX_VERSION_STRING_SIZE             32
#define VERSION_TEMPLATE                    "Version: %3d.%3d-%3d"
#define VERSION_STRING_COMPARE_SIZE         9  // "Version: "
#define VERSION_VARIABLES_TO_MATCH          3
#define TAG_START_MARKER                    '<'
#define TAG_END_MARKER                      '>'
#define TIMEOUT_TO_CHECK_INCOMING_TRAFFIC   100000      // uSeconds
#define HELPER_THREAD_READ_BUFFER           (32*1024)   // bytes

#define HOTGEN_FIRMWARE_PARTITION_1_OFFSET  0x030000
#define HOTGEN_FIRMWARE_PARTITION_2_OFFSET  0x410000
#define HOTGEN_FIRMWARE_BANK_MASK	    0x3
#define HOTGEN_MTD_PARTITION_DEVICE         "/dev/mtd1"
#define HOTGEN_MTD_PARTITION_SIZE           0x10000
#define HOTGEN_MTD_PARTITION_ERASE_SIZE     0x10000
/*
* ------------------------------------ User data types -----------------------------------------------------------
*/
typedef enum                    // TAG INDEX MUST MATCH THE ABOVE STRING TAG ARRAY INDEX
{
    TAG_HEAD,                   // DONT CHANGE THIS TAG'S POSITION
    TAG_TYPE,
    TAG_NAME,
    TAG_SIZE,
    TAG_PATH,
    TAG_MD5SUM,
    TAG_BLOCK_COUNT,
    TAG_BLOCK_SIZE,
    TAG_COMPRESSED,
    TAG_UNCOMP_ALGORITHM,
    TAG_UNCOMP_SIZE,
    TAG_UNCOMP_MD5SUM,
    TAG_UNCOMP_PATH,
    TAG_UNCOMP_NAME,
    TAG_TAIL                    // DONT CHANGE THIS TAG'S POSITION
} OTA_TAGS;

typedef struct tplink_hotgen_header {
	uint32_t	flags;			/* header flags 	*/
	uint32_t	F1_offset;		/* offset to firmware 1 */
	uint32_t	F2_offset;		/* offset to firmware 2 */
} Hotgen_Header_t;

typedef enum
{
	bank_min = 0,
	bank_1 = 1,
	bank_2 = 2,
	bank_max = 3
}HotgenFirmwareBank;

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/
// standard version message for the clients; +1 for null char, but not sent to the network
static uint8_t gVersionMessage[SIZE_OF_HEADER + MAX_VERSION_STRING_SIZE + 1];
static uint8_t gVersionMessage_Length = 0;
static pthread_mutex_t gUpgrade_mutex;  // protects multiple clients triggering the download
static UTILS_Thread_t gHelperThread;    // thread id for the Helper thread - which is responsible for incoming upgrade payload management
static OTA_Info_t ota;        // global structure contains the current ota download params
static char* gOtaTags [] = {
    "ota",                    // head tag
    "type",                   // ota payload type - firmware / package
    "name",                   // file name
    "size",                   // file size in bytes
    "path",                   // file path to download
    "md5sum",                 // md5 checksum value of the file
    "block_count",            // no of blocks to be transferred from the network client
    "block_size",             // maximum size of each block
    "compressed",             // is the payload compressed ?
    "uncomp_algorithm",       // uncompression algorthim                      (valid if compressed)
    "uncomp_size",            // uncompressed file size in bytes              (valid if compressed)
    "uncomp_md5sum",          // md5 checksum value of the uncompressed file  (valid if compressed)
    "uncomp_path",            // file path to uncompress                      (valid if compressed)
    "uncomp_name",            // uncompressed file name                       (valid if compressed)
    "/ota"                    // tail tag   
};

/*
* ------------------------------------ Private Functions ------------------------------------------------------------------
*/
bool nwupgrade_IsTagValid (char* pTag, uint8_t* pTagId)
{
    uint8_t id = TAG_HEAD;
    DEBUG_IN;

    DEBUG_EINFO("pTag:%s", pTag);
    while (id < TAG_TAIL)
    {
        if (0 == strcmp (pTag, gOtaTags[id]))
        {
            *pTagId = id;
            DEBUG_INFO("Match:%s %d", gOtaTags[id], id);
            
            DEBUG_OUT;
            return true;
        }
        else {
            DEBUG_EINFO("NoMatch:%s %d", gOtaTags[id], id);
            id++;
        }
    }

    DEBUG_OUT;
    return false;
}

bool nwupgrade_FindOtaTag (uint8_t* buffer, uint32_t size, uint32_t *index, char* pTag)
{
    uint32_t start = 0; // no tag starts at '0' so it is safe
    uint32_t tindex = *index;
    bool retval = false;

    DEBUG_IN;

    DEBUG_EINFO("index:%d", tindex);
    //usleep(100000);

    while (tindex < size)
    {
        DEBUG_EINFO("buffer[%d] = %c", tindex, buffer[tindex]);
        if (0 == start) 
        {
            if (TAG_START_MARKER == buffer[tindex])
            {
                start = tindex;
            }
        }
        else if (TAG_END_MARKER == buffer[tindex])
        {
            // copy on the tag string without '<' & '>' i.e. for <ota> => pTag = ota
            memcpy(pTag, &buffer[start+1], ((tindex - start)-1));
            //pTag[(tindex - start)+2] = '\0';
            *index = tindex+1;
            DEBUG_EINFO("pTag[%s] %d %d %d %d", pTag, start, (tindex - start)-1, pTag[(tindex - start)], *index);
            retval = true;
            break;
        }
        tindex++;
    }
    
    *index = tindex;

    DEBUG_OUT;
    return retval;
}

bool nwupgrade_ValidateUpgradeParams (uint8_t* buffer, uint32_t size)
{
    bool retval = false;
    uint32_t index = NWUPGRADE_PARAM_OFFSET;
    char tag[NWUPGRADE_MAX_TAG_SIZE+1], *end;
    bool start = false;
    uint8_t tagId = (uint8_t)(TAG_TAIL + 1);
    bool parsed = false;

    DEBUG_IN;

    DEBUG_INFO("buffer:%p, size:%d", buffer, size);

    // parse the buffer for ota download param tags
    while (true)
    {
        memset(tag, 0, NWUPGRADE_MAX_TAG_SIZE+1);
        if (nwupgrade_FindOtaTag(buffer, size, &index, tag))
        {
            if (!start)
            {
                if (0 == strcmp (tag, gOtaTags[TAG_HEAD]))
                {
                    start = true;
                }
                else
                {
                    DEBUG_ERROR("Upgrade params xml data is corrupt/invalid");
                    break;
                }
            }
            else if (0 == strcmp (tag, gOtaTags[TAG_TAIL]))
            {
                DEBUG_INFO("Upgrade params xml parsing completed");
                parsed = true;
                break;
            }
            else if (nwupgrade_IsTagValid(tag, &tagId) && ((tagId > TAG_HEAD) && (tagId < TAG_TAIL)))
            {
                DEBUG_EINFO("Found tag variable... lets find tag value");
                memset(tag, 0, NWUPGRADE_MAX_TAG_SIZE+1);
                if (nwupgrade_FindOtaTag(buffer, size, &index, tag))
                {
                    switch(tagId)
                    {
                        case TAG_TYPE:
                            if (strcmp(tag, OTA_TYPE_FIRMWARE) == 0) { ota.type = (uint8_t)type_firmware; }
                            else if (strcmp(tag, OTA_TYPE_PACKAGE) == 0) { ota.type = (uint8_t)type_package; }
                            else { ota.type = (uint8_t)type_last; }
                        break;
                      
                        case TAG_NAME:
                            strcpy (ota.file.filename, tag);
                        break;
                        
                        case TAG_SIZE:
                            ota.file.filesize = strtol(tag, &end, 10);
                        break;
                        
                        case TAG_PATH:
                            strcpy (ota.file.path, tag);
                        break;
                        
                        case TAG_MD5SUM:
                            strcpy (ota.file.md5sum, tag);
                        break;
                        
                        case TAG_BLOCK_COUNT:
                            ota.bcount = strtol(tag, &end, 10);
                        break;
                        
                        case TAG_BLOCK_SIZE:
                            ota.bsize = strtol(tag, &end, 10);
                        break;
                        
                        case TAG_COMPRESSED:
                            if ((strcmp(tag, "true") == 0) || (strcmp(tag, "TRUE") == 0)) { ota.compress = ota_compressed; }
                            else if ((strcmp(tag, "false") == 0) || (strcmp(tag, "FALSE") == 0)) { ota.compress = ota_uncompressed; }
                            else { 
                                DEBUG_ERROR("Unknown TAG VALUE (%s) for TAG(compressed) in the OTA params", tag);
                                ota.compress = ota_last; 
                            }
                        break;
                        
                        case TAG_UNCOMP_ALGORITHM:
                            if ((strcmp(tag, "lzma") == 0) || (strcmp(tag, "LZMA") == 0)) { ota.file.algo = algo_lzma; ota.ufile.algo = algo_none;}
                            else { 
                                DEBUG_ERROR("Unknown TAG VALUE (%s) for TAG(uncomp_algorithm) in the OTA params", tag);
                                ota.file.algo = algo_last; 
                            }                        
                        break;
                        
                        case TAG_UNCOMP_SIZE:
                            ota.ufile.filesize = strtol(tag, &end, 10);
                        break;
                        
                        case TAG_UNCOMP_MD5SUM:
                            strcpy (ota.ufile.md5sum, tag);
                        break;
                        
                        case TAG_UNCOMP_PATH:
                            strcpy (ota.ufile.path, tag);
                        break;
                        
                        case TAG_UNCOMP_NAME:
                            strcpy (ota.ufile.filename, tag);
                        break;

                        default:
                            DEBUG_ERROR("Unknown TAG(%s,%d) found in the OTA params", tag, tagId);                        
                        break;
                    }                    
                }
                else
                {
                    DEBUG_ERROR("Upgrade params xml data is corrupt/invalid");
                }
            }
        }
        else if (index > size)
        {
            DEBUG_ERROR("Upgrade params xml data is corrupt/invalid");
            break;
        }
    }

    // verify the param's - do we have sufficient information to proceed ??
    if (parsed)
    {
        char filename[256];
        DEBUG_INFO ("type:%d\tname:%s\tsize:%d\tpath:%s\tmd5sum:%s\tBCnt:%d\tBSize:%d\tcompressed:%d\tuncomp_algorithm:%d\tuncomp_size:%d\tuncomp_md5sum:%s\tuncomp_name:%s\tuncomp_path:%s\tuncomp_algorithm:%d", 
            ota.type, (char*)ota.file.filename, ota.file.filesize, (char*)ota.file.path,
            (char*)ota.file.md5sum, ota.bcount, ota.bsize, ota.compress, ota.file.algo,
            ota.ufile.filesize, (char*)ota.ufile.md5sum, (char*)ota.ufile.filename, (char*)ota.ufile.path, ota.ufile.algo);

        if (ota.type == (uint8_t)type_last || ota.compress == ota_last || ota.file.algo == algo_last)
        {
            DEBUG_ERROR("Upgrade params xml data is partially corrupt/invalid type(%d) compress(%d) algo(%d)", 
                ota.type , ota.compress, ota.file.algo);
        }
#ifndef DISABLE_PAYLOAD        
        else if (0 > sprintf (filename, "%s/%s", ota.file.path, ota.file.filename))
        {
            DEBUG_ERROR("Creating download file path failed(%s, %s)", ota.file.path, ota.file.filename); 
        }        
        else if ((NULL == ota.file.fhndl) && (NULL == (ota.file.fhndl = fopen(filename, "w+"))))
        {
            DEBUG_ERROR("Creating download file failed(%d, %s)", errno, strerror(errno)); 
        }
        else if (0 > sprintf (filename, "%s/%s", ota.ufile.path, ota.ufile.filename))
        {
            DEBUG_ERROR("Creating download file path failed(%s, %s)", ota.ufile.path, ota.ufile.filename); 
        }        
        else if ((ota_compressed == ota.compress) && (NULL == (ota.ufile.fhndl = fopen(filename, "w+"))))
        {
            DEBUG_ERROR("Creating uncompressed download file failed(%d, %s)", errno, strerror(errno)); 
        }
#endif        
        else
        {
            ota.upgrade_complete_signal = false;
            retval = true;
        }        
    }

    DEBUG_OUT;
    return retval;
}

bool nwupgrade_ValidatePayloadCRC (uint8_t* buffer, uint32_t size, uint32_t u32Crc)
{
    DEBUG_IN;

    DEBUG_OUT;
    return true;
}

bool nwupgrade_ValidateAndAcceptPayload(uint8_t* buffer, uint32_t size)
{
    UpgradePayloadHeader_t  *header = (UpgradePayloadHeader_t*)&buffer[SIZE_OF_HEADER];
    bool retval = false;
    uint32_t to_write = 0;    

    DEBUG_IN;

    header->size    = ntohl(header->size);
    header->crc32   = ntohl(header->crc32);
    header->num     = ntohs(header->num);

    if (header->num != ota.next_block)
    {
        DEBUG_ERROR("Unexpected payload block arrived blockNo(%d) exp_blockNo(%d)", header->num, ota.next_block);
    }
    else if ((header->crc_flag == (uint8_t)crc_enabled) &&
             (!nwupgrade_ValidatePayloadCRC(&buffer[SIZE_OF_HEADER], (sizeof(UpgradePayloadHeader_t) + header->size), header->crc32)))
    {
        DEBUG_ERROR("Payload block(%d) failed on the CRC check", header->num);
    }
    else if (size > (SIZE_OF_HEADER + sizeof(UpgradePayloadHeader_t) + header->size))
    {
        DEBUG_ERROR("Expecting only one block at a time, unknown payload data in the block - size:%d b.header:%d payload:%d", 
            size, (int32_t)sizeof(UpgradePayloadHeader_t), header->size);             
    }
    else if (size == (SIZE_OF_HEADER + sizeof(UpgradePayloadHeader_t) + header->size)) // full payload
    {
        to_write = header->size;
    }
    else // partial payload
    {
        to_write = size - (SIZE_OF_HEADER + sizeof(UpgradePayloadHeader_t));
        ota.partial = header->size - to_write;
        DEBUG_INFO("Partial block arrived payload_total(%d) this(%d) partial(%d)", header->size, to_write, ota.partial);
    }

    if (to_write > 0)
    {
        uint32_t wrote = fwrite (&buffer[SIZE_OF_HEADER + sizeof(UpgradePayloadHeader_t)], 1, to_write, ota.file.fhndl);
        if (wrote != to_write)
        {
            DEBUG_ERROR("Unable to write all the data to the file exp:%d wrote:%d", to_write, wrote);
        }
        else
        {
            ota.received += wrote;
            DEBUG_MIL("Wrote (%d) bytes to the file from block(%d)... total written(%d) bytes", wrote, header->num, ota.received);
            retval = true;
        }
    }

    DEBUG_OUT;
    return retval;
}

void nwupgrade_HandleUpgradePayload (int hServer, uint8_t clientId, uint8_t* buffer, uint32_t size, bool *pConnected)
{
    ServerUpgradeResponse_t response;
    uint32_t sent;   

    DEBUG_IN;

    pthread_mutex_lock(&gUpgrade_mutex);

    UTILS_SetHeader(&response.header, NetworkDiscovery, (SERVER_UPGRADE_RESPONSE_SIZE - SIZE_OF_HEADER));
    response.type = NWDISCOVERY_LAST;

    // check whether an upgrade is in progress !!
    if (ota.client == NWDISCOVERY_INVALID_CLIENT_ID)
    {
        // return error - since there is no upgrade in progress return an error
        response.type = NWDISCOVERY_NWSERVER_UPGRADE_FAILED;
        response.reason = (uint8_t)FAILED_NO_UPGRADE;
        DEBUG_ERROR("No upgrade is currently in progress, but payload has arrived from client(%d)", clientId);
    }
    else if (ota.client != clientId)
    {
        // return error - since this network client is not the source for the current ota in progress
        response.type = NWDISCOVERY_NWSERVER_UPGRADE_FAILED;            // an upgrade is in progress
        response.reason = (uint8_t)FAILED_INPROGRESS;        
        DEBUG_WARN("No upgrade is currently in progress with this client(%d) valid_client(%d)", clientId, ota.client);
    }
    else // trusted network client
    {
        uint32_t to_write, wrote = 0;

        if (ota.partial > 0)
        {
            DEBUG_ERROR("Expected partial data size:%d arrived:%d", ota.partial, size);

            if (ota.partial < size)
            {
                to_write = ota.partial;
            }
            else
            {
                to_write = size;
            }

            wrote = fwrite (buffer, 1, to_write, ota.file.fhndl);
            ota.partial = ota.partial - wrote;
            if (wrote != to_write)
            {
                DEBUG_ERROR("Unable to write all the data to the file exp:%d wrote:%d", to_write, wrote);
                response.type = NWDISCOVERY_NWSERVER_UPGRADE_FAILED;            // an upgrade is in progress
                response.reason = (uint8_t)FAILED_SYSTEM_ERROR;
            }
            else
            {
                ota.received = ota.received + wrote;                
                DEBUG_MIL("Wrote (%d) bytes to the file from partial block(%d) ... total written(%d) bytes", 
                    wrote, ota.next_block, ota.received);
                if (ota.partial == 0) // on a block complete we send the ack
                {
                    response.type   = NWDISCOVERY_NWSERVER_UPGRADE_PAYLOAD_ACK;
                    response.reason = (uint8_t)FAILED_NONE;
                    ota.next_block  = ota.next_block + 1;
                }                
            }
        }

        if ((size - wrote) > 0)
        {
            if (!UTILS_VerifyPacketHeader (&buffer[wrote], 0, NetworkDiscovery, 0, NULL)) // dont verify payload length
            {
                DEBUG_ERROR("Network packet header verification failed");
                RECOIL_SendError(true, ota.hClient, (uint8_t)ota.client, UPGRADE_PAYLOAD_ERROR, NULL, 0);
            }
            else if (buffer[wrote+SIZE_OF_HEADER] != (uint8_t)NWDISCOVERY_NWSERVER_UPGRADE_PAYLOAD)
            {
                DEBUG_ERROR("Network packet type(%d) failed", buffer[wrote+SIZE_OF_HEADER]);
                RECOIL_SendError(true, ota.hClient, (uint8_t)ota.client, UPGRADE_PAYLOAD_ERROR, NULL, 0);
            }
            else if ((size - wrote) < (SIZE_OF_HEADER + sizeof(UpgradePayloadHeader_t)))
            {
                DEBUG_INFO("Header is broken not handled now size:%d", size);
                response.type = NWDISCOVERY_NWSERVER_UPGRADE_FAILED;
                response.reason = (uint8_t)FAILED_SYSTEM_ERROR;
            }
            else if (nwupgrade_ValidateAndAcceptPayload(&buffer[wrote], (size - wrote)))
            {
                if (ota.partial == 0) // on a block complete we send the ack
                {
                    response.type = NWDISCOVERY_NWSERVER_UPGRADE_PAYLOAD_ACK;
                    response.reason = (uint8_t)FAILED_NONE;
                    ota.next_block  = ota.next_block + 1;
                    DEBUG_INFO("new payload arrived from client(%d) with success", clientId);
                }
            }
            else
            {
                response.type = NWDISCOVERY_NWSERVER_UPGRADE_FAILED;            // an upgrade is in progress
                response.reason = (uint8_t)FAILED_INVALID_PARAMS;
                DEBUG_INFO("new payload arrived from client(%d) with errors", clientId);
            }
        }            
    }

    if (response.type != NWDISCOVERY_LAST)
    {
        if (UTILS_TCP_SendData(hServer, (uint8_t*)&response, SIZE_OF_NETWORK_UPGRADE_RESPONSE, &sent, pConnected))
        {
            if (sent == SIZE_OF_NETWORK_UPGRADE_RESPONSE)
            {
                DEBUG_INFO("Payload response message sent to client(%d)", clientId);
            }
            else
            {
                DEBUG_INFO("Unable to send the full packet to client(%d) setn(%d) exp(%d)", clientId, sent, SIZE_OF_NETWORK_UPGRADE_RESPONSE);
            }
        }
        else
        {
            DEBUG_ERROR("Unable to send the payload response message to client(%d) sent(%d)", clientId, sent);
        }
    }        

    pthread_mutex_unlock(&gUpgrade_mutex);
    DEBUG_OUT;    
}

// correct the position to the starting of the block boundary
// so we can restart from the beginning of the next expected block
void nwupgrade_RearrangeFirmwareUpgradePosition()
{
    long current = ftell(ota.file.fhndl), new = 0;
    ota.received = ota.next_block * ota.bsize;
    fseek(ota.file.fhndl, ota.received, SEEK_SET);
    new = ftell(ota.file.fhndl);
    ota.partial = 0;
    DEBUG_INFO("Firmware upgrade file is repositioned to (%d) from (%d) received(%d)", (uint32_t)new, (uint32_t)current, ota.received);

    return;
}

void* nwupgrade_HelperThread (void *pArg)
{
    OTA_Info_t *ota = (OTA_Info_t*)pArg; // socket to accept incoming connections
    uint8_t *buffer = NULL;
    int nfds = 0;       // nfds is the highest-numbered file descriptor in readfds/writefds/exceptfds plus 1. 
    fd_set fds;         // We only use readfds, fds is the handle used to watch for any characters become available for reading in the attached socket handles
    struct timeval tv;
    int activity, RxSize;    
    int client_size = sizeof(struct sockaddr_in);
    struct sockaddr_in client;

    DEBUG_IN;

    // allocate buffer to hold the payload data arriving from network client
    if (NULL == (buffer = (uint8_t*)malloc(HELPER_THREAD_READ_BUFFER)))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", (uint32_t)HELPER_THREAD_READ_BUFFER);
        gHelperThread.run = false;
    }

    while (gHelperThread.run) // while the system is allowed to run
    {
        DEBUG_EINFO("No new request, sleep until we recheck again...");

        /* wait for 100mS to check whether we got any traffic in the connected clients */
        tv.tv_sec = 0;
        tv.tv_usec = TIMEOUT_TO_CHECK_INCOMING_TRAFFIC;
        FD_ZERO(&fds);

        FD_SET (ota->hServer, &fds);
        nfds = ota->hServer;
        if (ota->hClient != INVALID_RETURN_VALUE)
        {
            FD_SET (ota->hClient, &fds);
            if (ota->hClient > nfds)
            {
                nfds = ota->hClient;
            }
        }

        // check whether we got any activity from the network
        if (SYSTEM_CALL_ERROR == (activity = select(nfds+1, &fds, NULL, NULL, &tv)))
        {
            DEBUG_ERROR("select() failed (%d, %s) nfds(%d)", errno, strerror(errno), nfds);
        }
        else 
        {
			if (activity)
			{
				DEBUG_INFO("Somedata is available on the network");
				if (FD_ISSET(ota->hServer, &fds))
				{
					int32_t tmpHandle;
					if (SYSTEM_CALL_ERROR != (tmpHandle = accept (ota->hServer, (struct sockaddr *) &client, (socklen_t*) &client_size)))
					{
						DiscoveryClientInfo_t info;

						DEBUG_INFO("Client(%s) Incoming request to join the upgrade link", inet_ntoa(client.sin_addr));
						// check whether it is the accepted client
#ifdef ENABLE_SAME_IP
						if (NWDISCOVERY_GetClientInfoFromIP((uint8_t*)inet_ntoa(client.sin_addr), &info, CRITERIA_NONE))
#else
						if (NWDISCOVERY_GetClientInfoFromIP((uint8_t*)inet_ntoa(client.sin_addr), &info))
#endif
						{
							if (info.clientId == ota->client)
							{
								if (ota->hClient == INVALID_RETURN_VALUE)
								{
									DEBUG_MIL("Client(%s) has%sestablished upgrade connection", 
										inet_ntoa(client.sin_addr), (ota->received) ? " re" : " ");
									ota->hClient = tmpHandle;
									if (ota->received)
									{
										nwupgrade_RearrangeFirmwareUpgradePosition();
									}                                
								}
								else
								{
									DEBUG_WARN("Client(%s) has already got an upgrade connection", inet_ntoa(client.sin_addr));
									RECOIL_SendError(true, tmpHandle, ota->client, UPGRADE_ALREADY_CONNECTED, NULL, 0);
									close(tmpHandle);
								}
							}
							else
							{
								// this client already got a discovery link throw an error
								DEBUG_ERROR("BS is already connected to another network client(%d) for upgrade", ota->client);
								RECOIL_SendError(true, tmpHandle, ota->client, UPGRADE_ALREADY_CONNECTED, NULL, 0);
								close(tmpHandle);
							}
						}
					}
					else
					{
						DEBUG_ERROR("Unable to accept the incoming connection on port(%s)", gNetworkUpgradePort);
					}
				}
				
				if ((INVALID_RETURN_VALUE != ota->hClient) && (FD_ISSET(ota->hClient, &fds)))
				{
					// read the incoming data
					if (SYSTEM_CALL_ERROR == (RxSize = recv(ota->hClient, buffer, HELPER_THREAD_READ_BUFFER, 0)))
					{
						// TODO - check this is valid ?? we got an activity but no data, could this be valid ?
						if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
						{
							DEBUG_WARN("Nothing to read from the socket..");
						}
						else
						{
							DEBUG_ERROR("recv() failed(%d, %s) for client(%d)", errno, strerror(errno), ota->client);
						}
					}
					else if (RxSize == 0)
					{
						DEBUG_ERROR("Client(%d) disconnected unexpectedly", ota->client);
						close(ota->hClient);
						ota->hClient = INVALID_RETURN_VALUE;
					}                    
					else
					{
						bool connected = true;
						DEBUG_INFO("Client(%d) has sent a packet of size(%d)...", ota->client, RxSize);
						nwupgrade_HandleUpgradePayload(ota->hClient, ota->client, buffer, (uint32_t)RxSize, &connected);
						if (!connected)
						{
							close(ota->hClient);
							ota->hClient = INVALID_RETURN_VALUE;
						}
					}
				}
			}
		}
    }

    if (buffer != NULL)
    {
        free(buffer);
    }

    DEBUG_MIL("Network Upgrade [helper thread] shutdown complete...");

    DEBUG_OUT;
    pthread_exit(NULL);    
}


bool nwupgrade_InitialiseDownload(void)
{
    int status;
    bool retval = false;

    DEBUG_IN;

    // thread must run on creation
    gHelperThread.run = true;

    // reset to default
    ota.client  = NWDISCOVERY_INVALID_CLIENT_ID;
    ota.hClient = INVALID_RETURN_VALUE;
    ota.next_block = 0;
    ota.received = 0;
    ota.partial = 0;
    ota.upgrade_complete_signal = false;
    
    // Create a TCP socket
    if (0 != (status = UTILS_TCP_Create(gNetworkUpgradePort, true, 1, &ota.hServer)))
    {
        DEBUG_FATAL("UTILS_TCP_Create failed(%s:%d) for port(%s)", gai_strerror(status), status, gNetworkUpgradePort);
    }
    // create the helper thread    
    else if (SYSTEM_CALL_ERROR == (status = UTILS_CreateThread (
            &gHelperThread.thread, DEFAULT_THREAD_STACK_SIZE, nwupgrade_HelperThread, (void*) &ota)))
    {
        DEBUG_FATAL("Helper thread create failed (%d, %s)", errno, strerror(errno));
    }
    else
    {
        retval = true;        
    }

    DEBUG_OUT;
    return retval;
}

void nwupgrade_HandleUpgradeRequest (int hServer, uint32_t clientId, uint8_t* buffer, uint32_t size, bool *pConnected)
{
    ServerUpgradeResponse_t response;
    uint32_t sent;

    DEBUG_IN;

    pthread_mutex_lock(&gUpgrade_mutex);

    UTILS_SetHeader(&response.header, NetworkDiscovery, (SERVER_UPGRADE_RESPONSE_SIZE - SIZE_OF_HEADER));
    response.type = NWDISCOVERY_NWSERVER_UPGRADE_FAILED;    // an upgrade is in progress

    // check whether an upgrade is in progress !!
    if (ota.client == NWDISCOVERY_INVALID_CLIENT_ID)
    {
        if (nwupgrade_ValidateUpgradeParams(buffer, size))
        {
            if (nwupgrade_InitialiseDownload())
            {
                ota.client = (uint8_t)clientId;
                response.type = NWDISCOVERY_NWSERVER_UPGRADE_ACK;
                response.reason = (uint8_t)FAILED_NONE;
                DEBUG_MIL("RECOIL Firmware Upgrade request accepted from client(%d)", clientId);
            }
            else
            {
                response.reason = (uint8_t)FAILED_SYSTEM_ERROR;
                DEBUG_FATAL("Upgrade request from client(%d) failed, unable to initialise upgrade helper", clientId);
            }                        
        }
        else
        {
            response.reason = (uint8_t)FAILED_INVALID_PARAMS;
            DEBUG_ERROR("Upgrade request with invalid ota params from client(%d) is rejected", clientId);
        }
    }
    else
    {
        response.reason = (uint8_t)FAILED_INPROGRESS;
        DEBUG_WARN("Upgrade is currently in progress with client(%d), new request from client(%d) is rejected", ota.client, clientId);
    }
    pthread_mutex_unlock(&gUpgrade_mutex);

    if ((UTILS_TCP_SendData(hServer, (uint8_t*)&response, SIZE_OF_NETWORK_UPGRADE_RESPONSE, &sent, pConnected)) && 
        (sent == SIZE_OF_NETWORK_UPGRADE_RESPONSE))
    {
        DEBUG_INFO("Upgrade request response sent to client(%d)", clientId);
    }
    else
    {
        DEBUG_ERROR("Unable to send the upgrade response message to client(%d)", clientId);
    }

    DEBUG_OUT;    
}

void nwupgrade_HandleVersionRequest (int hServer, uint32_t clientId, bool *pConnected)
{
    DEBUG_IN;

    if (gVersionMessage_Length != 0)
    {
        uint32_t sent;
        if ((UTILS_TCP_SendData(hServer, gVersionMessage, gVersionMessage_Length, &sent, pConnected)) && (sent == gVersionMessage_Length))
        {
            DEBUG_INFO("Version message sent to client(%d)", clientId);
        }
        else
        {
            DEBUG_ERROR("Unable to send the version info message to client(%d)", clientId);
        }
    }
    else
    {
        RECOIL_SendError(true, hServer, (uint8_t)clientId, UPGRADE_VERSION_ERROR, NULL, 0);
    }

    DEBUG_OUT;
    return;
}

bool nwupgrade_UpdateHotgenPartitionWithNewFirmwareInfo (void)
{
    bool retval = false;
    mtd_info_t mtd_info;        // the MTD structure
    erase_info_t erase_info;            // the erase block structure
    int fd, status=0;
    HotgenFirmwareBank newBank;
    Hotgen_Header_t tHeader = {0};

    // if fails, change file permission on this device
    if (SYSTEM_CALL_ERROR == (fd = open(HOTGEN_MTD_PARTITION_DEVICE, O_RDWR)))
    {
        DEBUG_ERROR("OPEN (%s) failed(%d, %s)", HOTGEN_MTD_PARTITION_DEVICE, errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = ioctl(fd, MEMGETINFO, &mtd_info)))
    {
        DEBUG_ERROR("MEMGETINFO ioctl %s failed(%d, %s)", HOTGEN_MTD_PARTITION_DEVICE, errno, strerror(errno));
    }
    else if ((mtd_info.size != HOTGEN_MTD_PARTITION_SIZE) || 
             (mtd_info.erasesize != HOTGEN_MTD_PARTITION_ERASE_SIZE))
    {
        DEBUG_ERROR("HOTGEN partition info(%x, %x) failed", mtd_info.size, mtd_info.erasesize);
    }
    else if (SYSTEM_CALL_ERROR == (status = lseek(fd, 0, SEEK_SET)))
    {
        DEBUG_ERROR("LSEEK on %s failed(%d, %s)", HOTGEN_MTD_PARTITION_DEVICE, errno, strerror(errno));
    }
    else if (sizeof(Hotgen_Header_t) != (status = read(fd, (void*)&tHeader, sizeof(Hotgen_Header_t))))
    {
        DEBUG_ERROR("READ from %s failed(%d, %s, %d)", HOTGEN_MTD_PARTITION_DEVICE, errno, strerror(errno), status);
    }
    else
    {
        if ((tHeader.flags & HOTGEN_FIRMWARE_BANK_MASK) == bank_1)
        {
            newBank = bank_2;
        }
        else if ((tHeader.flags & HOTGEN_FIRMWARE_BANK_MASK) == bank_2)
        {
            newBank = bank_1;
        }
        else
        {
            DEBUG_ERROR("HOTGEN partition %s corrupted - force fix activated", HOTGEN_MTD_PARTITION_DEVICE);
            newBank = bank_2;      // default to second bank, since uboot defaults to first bank on corruption to hotgen partition
            tHeader.F1_offset = HOTGEN_FIRMWARE_PARTITION_1_OFFSET;
            tHeader.F2_offset = HOTGEN_FIRMWARE_PARTITION_2_OFFSET;
        }

        DEBUG_MIL("HOTGEN partition Type(%x) Size(%x b) EraseSize(%x b) BootFlags(%x) Firmware#1Offset(%x) Firmware#2Offset(%x)",
            mtd_info.type, mtd_info.size, mtd_info.erasesize, tHeader.flags, tHeader.F1_offset, tHeader.F2_offset);        
        
        tHeader.flags = ((tHeader.flags & (~HOTGEN_FIRMWARE_BANK_MASK)) | newBank);
        DEBUG_INFO("new tHeader.flags = %x", tHeader.flags);        
        erase_info.start = 0;
        erase_info.length = mtd_info.erasesize;

        // chip does not support lock & unlock to ignore it
        // ioctl(fd, MEMUNLOCK, &erase_info); 
        if (SYSTEM_CALL_ERROR == (status = ioctl(fd, MEMERASE, &erase_info)))
        {
            DEBUG_ERROR("MEMERASE ioctl %s failed(%d, %s)", HOTGEN_MTD_PARTITION_DEVICE, errno, strerror(errno));
        }      
        else if (SYSTEM_CALL_ERROR == (status = lseek(fd, 0, SEEK_SET)))
        {
            DEBUG_ERROR("LSEEK on %s failed(%d, %s)", HOTGEN_MTD_PARTITION_DEVICE, errno, strerror(errno));
        }               
        else if (sizeof(Hotgen_Header_t) != (status = write(fd, (void*)&tHeader, sizeof(Hotgen_Header_t))))
        {
            DEBUG_ERROR("WRITE to %s failed(%d, %s, %d)", HOTGEN_MTD_PARTITION_DEVICE, errno, strerror(errno), status);
        }
        else // unable to verify this information since, it only returns data from cache, but on reboot the updated information are 
        {    // correclty present, hence no verification is attempted and all resulted in failure
            DEBUG_MIL("HOTGEN partition updated successfully");            
            retval = true;
        }
    }

    if (fd) {
        close(fd);
    }    

    DEBUG_OUT;    
    return retval;
}

bool nwupgrade_VerifyMD5Sum (char* path, char* filename, char* md5sum)
{
    bool retval = false;
    FILE *fp;
    char command[128];    
    char buffer[256];

    DEBUG_IN;

    sprintf(command, "md5sum %s/%s", path, filename);
    DEBUG_EINFO("command: %s", command);
    
    /* Open the command for reading. */
    if (NULL == (fp = popen(command, "r")))
    {
        DEBUG_ERROR("Unable to run the md5sum command");
    }
    else
    {
        /* Read the output a line at a time - output it. */
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL)
        {
            DEBUG_EINFO("line: %s", buffer);
            if (0 == strncmp (buffer, md5sum, sizeof(md5sum)))
            {
                retval = true;
                DEBUG_INFO("md5sum matched the expected value(%s) act(%s)", md5sum, buffer);
                break;
            }
            else
            {
                DEBUG_ERROR("md5sum did not match exp(%s) act(%s)", md5sum, buffer);
                break;
            }
        }

        /* close */
        pclose(fp);
    }

    DEBUG_OUT;    
    return retval;
}

#if defined(FORCE_RANDOM_MAC)
void nwupgrade_UpdateWirelessDeviceMAC (void)
{
    uint32_t mac_offset = 0x1FC00, random;

    srand(time(NULL));
    random = rand();
    sprintf(command, "echo -n -e \\\\x%02X\\\\x%02X\\\\x%02X\\\\x%02X\\\\x%02X\\\\x%02X | dd seek=%u of=/dev/mtdblock0 count=6 bs=1 conv=notrunc",
            0xF0,
            0xDE,
            0xAD,
            random&0xFF,
            (random>>8)&0xFF,
            (random>>16)&0xFF,
            mac_offset);
    DEBUG_INFO("Randomising with %s", command);

    if (NULL == (fp = popen(command, "r")))
    {
        DEBUG_ERROR("Unable to run the mac randomisation command");
    }
    else
    {
        DEBUG_MIL("MAC randomisation successful");
        pclose(fp);
    }
    
    sprintf(command, "uci set wireless.@wifi-iface[0].ssid=\"RecoilGameHub_%02X%02X%02X\"; uci commit wireless",
           random&0xFF,
           (random>>8)&0xFF,
           (random>>16)&0xFF);
    DEBUG_INFO("Set SSID with %s", command);

    if (NULL == (fp = popen(command, "r")))
    {
        DEBUG_ERROR("Unable to set SSID");
    }
    else
    {
        DEBUG_MIL("SSID randomisation successful");
        pclose(fp);
    }

    return;
}
#endif //FORCE_RANDOM_MAC

bool nwupgrade_InstallPackage(char* file)
{
    bool retval = false;
    FILE *fp;
    char command[128];    
    char buffer[256];
    char verify1[] = "Upgrading recoil on root from";
    char verify1_downgrade[] = "Downgrading recoil on root from";
    char verify2[] = "Configuring recoil.";    

    DEBUG_IN;

#if defined(FORCE_RANDOM_MAC)
    nwupgrade_UpdateWirelessDeviceMAC();
#endif //FORCE_RANDOM_MAC

#if defined(ALLOW_RECOIL_DOWNGRADE)
    sprintf(command, "opkg install --force-downgrade %s", file);
#else
    sprintf(command, "opkg install %s", file);
#endif

#ifdef ENABLE_BIG_ENDIAN_BUILD
    /* Open the command for reading. */
    if (NULL == (fp = popen(command, "r")))
    {
        DEBUG_ERROR("Unable to run the opkg install command");
    }
    else if (fgets(buffer, sizeof(buffer)-1, fp) == NULL)
    {
        DEBUG_ERROR("Unable to get the install status#1");
    }
    else if ((0 != strncmp (buffer, verify1, sizeof(verify1)-1)) && 
             (0 != strncmp (buffer, verify1_downgrade, sizeof(verify1_downgrade)-1)))
    {
        DEBUG_ERROR("verify first status line failed. '%s' does not match '%s' nor '%s'.", buffer, verify1, verify1_downgrade);
    }
    else if (fgets(buffer, sizeof(buffer)-1, fp) == NULL)
    {
        DEBUG_ERROR("Unable to get the install status#2");
    }
    else if (0 != strncmp (buffer, verify2, sizeof(verify2)-1))
    {
        DEBUG_ERROR("verify second status line(%s != %s) failed", buffer, verify2);
    }
    else
    {
        DEBUG_MIL("Successfully installed package(%s) - complete", file);
        retval = true;        
    }

    /* close */
    if (fp)
    {
        pclose(fp);
    }
#else
    DEBUG_MIL("command (%s)", command);
    DEBUG_MIL("Successfully installed package(%s)", file);
    retval = true;
#endif

    DEBUG_OUT;
    return retval;
}

// mtd -r write /tmp/openwrt-ar71xx-generic-wzr-hp-ag300h-squashfs-sysupgrade.bin upgrade

bool nwupgrade_InstallFirmware(char* file, uint32_t filesize, char* partition)
{
#if 1
    bool retval = false;
    FILE *fp;
    char command[128];
#if 0 // Corresponding to a later #if
    char buffer[256];
    uint8_t loop;
#endif
    int8_t time = 25;
    DEBUG_IN;

    sprintf(command, "mtd write %s upgrade", file);
    DEBUG_INFO("command(%s)", command);

#if 1
    /* Open the command for reading. */
    if (NULL == (fp = popen(command, "r")))
    {
        DEBUG_WARN("Unable to open the firmware write logging command");
    }
    else
    {
#if 0        
        while (time > 0)
        {
            if (fgets(buffer, sizeof(buffer)-1, fp) != NULL)
            {
                DEBUG_INFO("buffer[%s]", buffer);
            }
            else
            {
                DEBUG_WARN("fgets returned NULL");
            }
            sleep(1);
            time--;
        }
#else        
        sleep(25);        
#endif  
        pclose(fp);
    }

    time = 10;
    sprintf(command, "mtd verify %s upgrade", file);
    DEBUG_INFO("command(%s)", command);
    if (NULL == (fp = popen(command, "r")))
    {
        DEBUG_WARN("Unable to open the upgrade verify logging command");
    }
    else
    {
#if 0
        while (time > 0)
        {
            if (fgets(buffer, sizeof(buffer)-1, fp) != NULL)
            {
                DEBUG_INFO("buffer[%s]", buffer);
            }
            else
            {
                DEBUG_WARN("fgets returned NULL");
            }
            sleep(1);
            time--;
        }
#else
        sleep(5);
#endif        
        pclose(fp);
    }        
#endif
#if 0
    else
    {
        sprintf(command, "mtd verify %s upgrade", file);
        DEBUG_INFO("command(%s)", command);
        for (loop = 0; loop < 5; loop++)
        {
            if (fgets(buffer, sizeof(buffer)-1, fp) != NULL)
            {
                if (0 == strcmp (buffer, "Success"))
                {
                    DEBUG_MIL("Successfully installed firmware(%s)", file);
                    retval = true;
                    break;
                }
                else
                {
                    DEBUG_ERROR("Unmatched expected string %s", buffer);
                }
            }
        }
    }

    /* close */
    if (fp)
    {
        pclose(fp);
    }

#else
    DEBUG_MIL("Successfully installed firmware(%s)", file);
    retval = true;
#endif
#else
    mtd_info_t mtd_info;           // the MTD structure
    erase_info_t ei;               // the erase block structure
    int dstfd, status;
    bool retval = false;
    FILE *srcfd = NULL;
    uint8_t *buffer = NULL;

    if (SYSTEM_CALL_ERROR == (dstfd = open(partition, O_RDWR)))
    {
        DEBUG_ERROR("Opening firmware partition failed - error(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = (int)lseek(dstfd, 0, SEEK_SET)))
    {
        DEBUG_ERROR("Seeking to start failed - error(%d, %s)", errno, strerror(errno));
    }     
    else if (SYSTEM_CALL_ERROR == (status = ioctl(dstfd, MEMGETINFO, &mtd_info)))
    {
        DEBUG_ERROR("MEMGETINFO failed - error(%d, %s)", errno, strerror(errno));
    }
    else if (NULL == (srcfd = fopen(file, "r")))
    {
        DEBUG_ERROR("Opening firmware file failed - error(%d, %s)", errno, strerror(errno));
    }    
    else if (NULL == (buffer = malloc((size_t)mtd_info.erasesize))
    {
        DEBUG_ERROR("NOMEMORY failed - size(%d)", (int32_t)mtd_info.erasesize);
    }
    else
    {
        bool err = false;
        int32_t read, blockwrite, totalwrite = 0;

        // dump it for a sanity check, should match what's in /proc/mtd
        DEBUG_INFO("MTD Type: %x\nMTD total size: %x bytes\nMTD erase size: %x bytes\n", mtd_info.type, mtd_info.size, mtd_info.erasesize);

        ei.length = mtd_info.erasesize;   //set the erase block size
        for(ei.start = 0; ei.start < mtd_info.size; ei.start += ei.length)
        {
            if (SYSTEM_CALL_ERROR == (status = ioctl(dstfd, MEMUNLOCK, &ei)))
            {
                DEBUG_ERROR("MEMUNLOCK(%#x) failed - error(%d, %s)", ei.start, errno, strerror(errno));
            }            
            else if (SYSTEM_CALL_ERROR == (status = ioctl(dstfd, MEMERASE, &ei)))
            {
                DEBUG_ERROR("MEMERASE(%#x) failed - error(%d, %s)", ei.start, errno, strerror(errno));
            }
            else
            {
                DEBUG_INFO("Erased Block %#x\n", ei.start);                

                if (totalwrite < filesize) // only if we got more data to write
                {
                    if ((filesize - totalwrite) > mtd_info.erasesize)
                    {
                        blockwrite = mtd_info.erasesize;
                    }
                    else
                    {
                        blockwrite = filesize - totalwrite;    
                    }

                    if ((read != blockwrite) && (read = fread((void*)buffer, 1, (size_t)blockwrite, srcfd)))
                    {
                        DEBUG_ERROR("FILEREAD failed req(%d) recv(%d)", blockwrite, (int32_t)read);
                        status = SYSTEM_CALL_ERROR;
                    }
                    else if (SYSTEM_CALL_ERROR == (status = (int32_t)write(dstfd, buffer, (size_t)blockwrite)))
                    {
                        DEBUG_ERROR("MEMWRITE(%#x) failed - error(%d, %s)", ei.start, errno, strerror(errno));
                    }
                    else if (status != blockwrite) // TODO - can we retry here !! with delay ??
                    {                        
                        DEBUG_ERROR("MEMWRITE_ALL(%#x) failed exp(%d) wrote(%d)", ei.start, blockwrite, status);
                        status = SYSTEM_CALL_ERROR;
                    }
                    else
                    {
                        DEBUG_INFO("MEMWRITE_ALL(%#x) size(%d) success", ei.start, status);
                    }

                    if (status == SYSTEM_CALL_ERROR)
                    {
                        break;
                    }
                }
                else
                {
                    DEBUG_INFO("Nothing to write for this block(%#x) totalwrite(%d) filesize(%d)", ei.start, totalwrite, filesize);
                }
            }
        }

        if (status != SYSTEM_CALL_ERROR)
        {
            if (SYSTEM_CALL_ERROR == (status = lseek(dstfd, 0, SEEK_SET)))
            {
                DEBUG_ERROR("MEMUNLOCK(%#x) failed - error(%d, %s)", ei.start, errno, strerror(errno));
            }

        }

            ;               // go to the first block
            read(fd, read_buf, sizeof(read_buf)); // read 20 bytes 
    }

#endif

    DEBUG_OUT;
    return retval;
}

bool nwupgrade_InstallSoftware (void)
{
    char  filename[256];
    uint32_t size;
    bool retval = false;

    DEBUG_IN;

    if (ota.compress == ota_compressed)
    {
        sprintf (filename, "%s/%s", ota.ufile.path, ota.ufile.filename);
        size = ota.ufile.filesize;
    }
    else
    {
        sprintf (filename, "%s/%s", ota.file.path, ota.file.filename);
        size = ota.file.filesize;
    }

    if (ota.type == type_package)
    {
        retval = nwupgrade_InstallPackage(filename);
    }
    else // firmware
    {
        retval = nwupgrade_InstallFirmware(filename, size, "/dev/mtd5");
        if (retval)
        {
            retval = nwupgrade_UpdateHotgenPartitionWithNewFirmwareInfo();
        }
    }

    DEBUG_OUT;
    return retval;
}

bool nwupgrade_VerifyUncompression ()
{
    uint32_t wrote;
    bool retval = false;
    char buffer[] = "Uncompression is successfully completed here";
    DEBUG_IN;

    if ((wrote = fwrite (buffer, 1, sizeof(buffer), ota.ufile.fhndl)) == sizeof(buffer))
    {
        DEBUG_MIL("Compressed source file is uncompressed successfully");
        retval = true;        
    }
    else
    {
        DEBUG_ERROR("source file uncompression failed exp(%d) wrote(%d)", (int32_t)sizeof(buffer), wrote); 
    }

    if (ota.ufile.fhndl)
    {
        if (0 == fclose(ota.ufile.fhndl))
        {
            DEBUG_INFO("Uncompressed file updated successfully");
        }
        else
        {
            DEBUG_ERROR("Uncompressed file close failed");
        }
    }

    DEBUG_OUT;
    return retval;
}

void nwupgrade_HandleUploadComplete(int hServer, uint32_t clientId, uint8_t* buffer, uint32_t size, bool *pConnected)
{
    ServerUpgradeResponse_t response;
    uint32_t sent = 0;

    DEBUG_IN;

    DEBUG_MIL("Upload complete message arrived from client(%d)", clientId);

    UTILS_SetHeader(&response.header, NetworkDiscovery, (SERVER_UPGRADE_RESPONSE_SIZE - SIZE_OF_HEADER));
    response.type = NWDISCOVERY_NWSERVER_UPGRADE_FAILED;

    pthread_mutex_lock(&gUpgrade_mutex);
    if (ota.client != clientId)
    {
        response.reason = (uint8_t)FAILED_INPROGRESS;
        DEBUG_WARN("Upgrade inprogress with client(%d), unexpected complete from client(%d) is rejected", ota.client, clientId);
    }
    else
    {
        if (ota.upgrade_complete_signal)
        {
            // just log, no need to send anything to client
            DEBUG_MIL("Duplicate upload complete message arrived !!");
            response.type = NWDISCOVERY_LAST;            
        }
        else
        {
            ota.upgrade_complete_signal = true;
#ifndef DISABLE_PAYLOAD
            // do we have the full file to proceed ?
            if (ota.file.filesize != ota.received)
            {
                response.reason = (uint8_t)FAILED_TRANSFER_ERROR;
                DEBUG_ERROR("Did not receive all the payload as expected recv(%d) exp(%d)", ota.received, ota.file.filesize);    
            }
            else if (0 != fclose(ota.file.fhndl))
            {
#warning HIJACK_ON_CLOSE_FIX_IT
                response.reason = (uint8_t)FAILED_SYSTEM_ERROR;
                DEBUG_ERROR("Unable to close the file handle error(%d, %s)", errno, strerror(errno));      
            }
            else if (!nwupgrade_VerifyMD5Sum(ota.file.path, ota.file.filename, ota.file.md5sum))
            {
                response.reason = (uint8_t)FAILED_MD5SUM_ERROR;
                DEBUG_ERROR("Downloaded file failed on MD5Sum check");
            }
            else 
#endif    
            if ((ota.compress == ota_compressed) && !nwupgrade_VerifyUncompression())
            {
                response.reason = (uint8_t)FAILED_UNCOMPRESSION_ERROR;
                DEBUG_ERROR("Downloaded file failed on MD5Sum check");    
            }
            else if (!nwupgrade_InstallSoftware())
            {
                response.reason = (uint8_t)FAILED_INSTALL_ERROR;
                DEBUG_ERROR("Upgrade failed on Installation");
            }
            else
            {
                response.type = NWDISCOVERY_NWSERVER_UPGRADE_COMPLETE;
                response.reason = (uint8_t)FAILED_NONE;
                DEBUG_MIL("RECOIL Firmware Upgrade completed !!!");
            }
        }            
    }
    pthread_mutex_unlock(&gUpgrade_mutex);

    if (response.type != NWDISCOVERY_LAST)
    {
        if ((UTILS_TCP_SendData(hServer, (uint8_t*)&response, SIZE_OF_NETWORK_UPGRADE_RESPONSE, &sent, pConnected)) && 
            (sent == SIZE_OF_NETWORK_UPGRADE_RESPONSE))
        {
            DEBUG_INFO("Upgrade complete response sent");
        }
        else
        {
            DEBUG_ERROR("Upgrade complete response send failed");
        }
    }

    DEBUG_OUT;
    return;
}

void NWUPGRADE_HandleNodePacket (int hServer, uint32_t clientId, uint8_t* buffer, uint32_t size, bool *pConnected)
{
    // since the packet header is already verified we dont need to validate header
    uint8_t packet_type = buffer[SIZE_OF_HEADER];

    DEBUG_IN;
    switch (packet_type)
    {
        case NWDISCOVERY_NWSERVER_VERSION:
            nwupgrade_HandleVersionRequest(hServer, clientId, pConnected);
        break;

        case NWDISCOVERY_NWSERVER_UPGRADE:
            nwupgrade_HandleUpgradeRequest(hServer, clientId, buffer, size, pConnected);
        break;

        case NWDISCOVERY_NWSERVER_UPGRADE_UPLOAD_COMPLETE:
            nwupgrade_HandleUploadComplete(hServer, clientId, buffer, size, pConnected);
        break;

        default:
         DEBUG_ERROR("Unexpected packet_type(%d) arrived to NWUPGRADE module", packet_type);
    }

    DEBUG_OUT;
    return;
}

/*
* ------------------------------------ Public Functions ------------------------------------------------------------------
*/

bool NWUPGRADE_Start (void)
{
    bool retval = false;
    char buffer[MAX_VERSION_STRING_SIZE];
    int revision, major, minor;
    ProtocolHeaderV1_write *header;
    FILE *hndl;

    DEBUG_IN;

    // initialise upgrade source network client id as invalid
    ota.client = NWDISCOVERY_INVALID_CLIENT_ID;

    if (SYSTEM_CALL_ERROR == pthread_mutex_init (&gUpgrade_mutex, NULL))
    {
        DEBUG_FATAL("Upgrade client mutex create failed(%d, %s)", errno, strerror(errno));
    }
    else if (NULL != (hndl = fopen(NWUPGRADE_VERSION_INFO_FILE, "r")))
    {
        while (fgets(buffer, MAX_VERSION_STRING_SIZE, hndl) != NULL)
        {
            if (0 == strncmp (buffer, VERSION_TEMPLATE, VERSION_STRING_COMPARE_SIZE))
            {
				buffer[strcspn(buffer, "\r\n")] = 0; // Remove trailing newlines, for tidier debug output
                DEBUG_INFO("match : %s", buffer);
                if (VERSION_VARIABLES_TO_MATCH == sscanf((const char *)buffer, VERSION_TEMPLATE, &revision, &major, &minor))
                {
                    header = (ProtocolHeaderV1_write*)gVersionMessage;
                    gVersionMessage_Length = SIZE_OF_HEADER + SIZE_OF_NWDISCOVERY_PACKET_TYPE + (strlen(buffer));
                    UTILS_SetHeader(header, NetworkDiscovery, (gVersionMessage_Length - SIZE_OF_HEADER));
                    gVersionMessage[SIZE_OF_HEADER] = (uint8_t)(NWDISCOVERY_NWSERVER_VERSION_INFO & 0xFF);
                    strncpy ((char*)&gVersionMessage[SIZE_OF_HEADER+1], buffer, strlen(buffer));
#ifdef __DEBUG
                    {
                        uint8_t *p;
                        p = gVersionMessage;
                        DEBUG_INFO("Version[%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x]", 
                            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
                    }
#endif
                    DEBUG_MIL("RECOIL Version found revision(%03d) major(%03d) minor(%03d)", revision, major, minor);
                    break;
                }
                else
                {
                    DEBUG_EINFO("RECOIL Version sscanf failed.");
				}
            }
            else
            {
                DEBUG_EINFO ("no match = %s", buffer);
            }
        }
        fclose(hndl);
    }
    else
    {
        DEBUG_FATAL("Cannot open upgrade file (%s) because (%d,%s)", NWUPGRADE_VERSION_INFO_FILE, errno, strerror(errno));
	}

    if (gVersionMessage_Length > 0)
    {
        DEBUG_MIL("Network Upgrade started successfully...");
        retval = true;
    }
    else
    {
        DEBUG_MIL("Network Upgrade start failed ...");    
    }

    DEBUG_OUT;
    return retval;
}

bool NWUPGRADE_Stop (void)
{
    bool retval = true;
    int status;
    void *H_retval = NULL;
    
    DEBUG_IN;
    DEBUG_MIL ("Request to shut down [network upgrade] arrived...");

    // signal helper thread to terminate
    // Is the thead running? It may not have been started.
    if ((ota.client != INVALID_RETURN_VALUE) && (gHelperThread.run))
    {   
        gHelperThread.run = false;
        if (0 != (status = pthread_join(gHelperThread.thread, &H_retval)))
        {
            DEBUG_ERROR("Unable to wait for helper thread to exit...err(%d, %s)", errno, strerror(errno));
            retval = false;
        }
    }

    if (retval)
    {
        DEBUG_MIL ("Request to shut down [network upgrade] is complete...");    
    }

    DEBUG_OUT;
    return retval;
}

