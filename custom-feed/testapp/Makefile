# Highlevel package makefile for Hotgen Recoil
# Developer :- Rajesh Gunasekaran (rajg@hotgen.com)

include $(TOPDIR)/rules.mk

PKG_NAME:=testapp
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_FLAGS:=nonshared
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Network
  TITLE:=Userspace application for the Hotgen Recoil
  MAINTAINER:=Mark Fisher <fish@hotgen.com>
  DEPENDS:=+uhttpd
endef

define Package/$(PKG_NAME)/description
 This package contains the userspace application which manages the player in the recoil network
endef

CONFIGURE_VARS+= \
    CC="$(TOOLCHAIN_DIR)/bin/$(TARGET_CC)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

