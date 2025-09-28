MEDIALIBRARY_VERSION := 0.13.2
MEDIALIBRARY_URL := https://code.videolan.org/videolan/medialibrary/-/archive/$(MEDIALIBRARY_VERSION)/medialibrary-$(MEDIALIBRARY_VERSION).tar.bz2

PKGS += medialibrary
ifeq ($(call need_pkg,"medialibrary >= 0.13.0"),)
PKGS_FOUND += medialibrary
endif

DEPS_medialibrary = sqlite $(DEPS_sqlite)

$(TARBALLS)/medialibrary-$(MEDIALIBRARY_VERSION).tar.bz2:
	$(call download_pkg,$(MEDIALIBRARY_URL),medialibrary)

.sum-medialibrary: medialibrary-$(MEDIALIBRARY_VERSION).tar.bz2

medialibrary: medialibrary-$(MEDIALIBRARY_VERSION).tar.bz2 .sum-medialibrary
	$(UNPACK)
	$(MOVE)

.medialibrary: medialibrary crossfile.meson
	$(MESONCLEAN)
	$(MESON) -Dlibvlc=disabled -Dlibtool_workaround=true
	+$(MESONBUILD)
	touch $@

