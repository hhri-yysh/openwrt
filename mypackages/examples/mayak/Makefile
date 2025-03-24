include $(TOPDIR)/rules.mk

PKG_NAME:=mayak
PKG_VERSION:=1.0
PKG_RELEASE:=1

SOURCE_DIR:=$(TOPDIR)/mayak

include $(INCLUDE_DIR)/package.mk

define Package/mayak
	SECTION:=examples
	CATEGORY:=Examples
	TITLE:=Network utility
endef

define Package/mayak/description
	A network diagnostic utility, for testing intermediate nodes availability on the network path from client machine.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/mayak/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/mayak $(1)/usr/bin
endef

$(eval $(call BuildPackage,mayak))

