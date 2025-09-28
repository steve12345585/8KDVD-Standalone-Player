# DVDNAV

LIBDVDNAV_VERSION := 6.1.1
LIBDVDNAV_URL := $(VIDEOLAN)/libdvdnav/$(LIBDVDNAV_VERSION)/libdvdnav-$(LIBDVDNAV_VERSION).tar.bz2

ifdef BUILD_DISCS
ifdef GPL
ifndef HAVE_WINSTORE
PKGS += dvdnav
endif
endif
endif
ifeq ($(call need_pkg,"dvdnav >= 5.0.3"),)
PKGS_FOUND += dvdnav
endif

$(TARBALLS)/libdvdnav-$(LIBDVDNAV_VERSION).tar.bz2:
	$(call download,$(LIBDVDNAV_URL))

.sum-dvdnav: libdvdnav-$(LIBDVDNAV_VERSION).tar.bz2

dvdnav: libdvdnav-$(LIBDVDNAV_VERSION).tar.bz2 .sum-dvdnav
	$(UNPACK)
	$(APPLY) $(SRC)/dvdnav/0001-configure-don-t-use-ms-style-packing.patch
	# turn asserts/exit into silent discard
	$(APPLY) $(SRC)/dvdnav/0001-play-avoid-assert-and-exit-and-bogus-PG-link.patch
	$(APPLY) $(SRC)/dvdnav/0002-play-avoid-assert-and-exit-and-bogus-Cell-link.patch
	# fix some bogus accesses
	$(APPLY) $(SRC)/dvdnav/0001-Check-the-the-title-parts-read-are-available.patch
	$(APPLY) $(SRC)/dvdnav/0002-Fix-access-to-title-not-found-in-array.patch
	$(call pkg_static,"misc/dvdnav.pc.in")
	$(MOVE)

DEPS_dvdnav = dvdread $(DEPS_dvdread)

.dvdnav: dvdnav
	$(REQUIRE_GPL)
	$(RECONF)
	$(MAKEBUILDDIR)
	$(MAKECONFIGURE)
	+$(MAKEBUILD)
	+$(MAKEBUILD) install
	touch $@
