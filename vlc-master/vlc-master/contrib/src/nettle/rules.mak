# Nettle

NETTLE_VERSION := 3.9
NETTLE_URL := $(GNU)/nettle/nettle-$(NETTLE_VERSION).tar.gz

ifeq ($(call need_pkg,"nettle >= 3.4.1"),)
PKGS_FOUND += nettle
endif

ifdef HAVE_WIN32
NETTLE_CONF += --disable-pic
ifeq ($(ARCH),arm)
NETTLE_CONF += --disable-assembler
endif
endif
NETTLE_CONF += --disable-documentation

$(TARBALLS)/nettle-$(NETTLE_VERSION).tar.gz:
	$(call download_pkg,$(NETTLE_URL),nettle)

.sum-nettle: nettle-$(NETTLE_VERSION).tar.gz

nettle: nettle-$(NETTLE_VERSION).tar.gz .sum-nettle
	$(UNPACK)
	$(call update_autoconfig,.)
	$(MOVE)

DEPS_nettle = gmp $(DEPS_gmp)

# GMP requires either GPLv2 or LGPLv3
.nettle: nettle
ifndef GPL
	$(REQUIRE_GNUV3)
endif
	$(MAKEBUILDDIR)
	$(MAKECONFIGURE) $(NETTLE_CONF)
	+$(MAKEBUILD) install
	touch $@
