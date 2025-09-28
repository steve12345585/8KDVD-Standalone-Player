# DVDCSS
DVDCSS_VERSION := 1.5.0
DVDCSS_URL := $(VIDEOLAN)/libdvdcss/$(DVDCSS_VERSION)/libdvdcss-$(DVDCSS_VERSION).tar.bz2
LIBDVDCSS_GITURL:=https://code.videolan.org/videolan/libdvdcss.git
LIBDVDCSS_BRANCH:=master
LIBDVDCSS_GITVERSION:=236350672911500999b1577f98c4740224999777

ifeq ($(call need_pkg,"libdvdcss"),)
PKGS_FOUND += dvdcss
endif

$(TARBALLS)/libdvdcss-$(LIBDVDCSS_GITVERSION).tar.xz:
	$(call download_git,$(LIBDVDCSS_GITURL),$(LIBDVDCSS_BRANCH),$(LIBDVDCSS_GITVERSION))

.sum-dvdcss: libdvdcss-$(LIBDVDCSS_GITVERSION).tar.xz
	$(call check_githash,$(LIBDVDCSS_GITVERSION))
	touch $@

# $(TARBALLS)/libdvdcss-$(DVDCSS_VERSION).tar.bz2:
# 	$(call download,$(DVDCSS_URL))

# .sum-dvdcss: libdvdcss-$(DVDCSS_VERSION).tar.bz2


# dvdcss: libdvdcss-$(DVDCSS_VERSION).tar.bz2 .sum-dvdcss
dvdcss: libdvdcss-$(LIBDVDCSS_GITVERSION).tar.xz .sum-dvdcss
	$(UNPACK)
	$(call update_autoconfig,.)
	$(MOVE)

DVDCSS_CONF := --disable-doc --disable-maintainer-mode

.dvdcss: dvdcss
	$(REQUIRE_GPL)
	$(RECONF)
	$(MAKEBUILDDIR)
	$(MAKECONFIGURE) $(DVDCSS_CONF)
	+$(MAKEBUILD)
	+$(MAKEBUILD) install
	touch $@
