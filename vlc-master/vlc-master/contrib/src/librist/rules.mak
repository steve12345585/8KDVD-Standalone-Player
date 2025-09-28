# librist

LIBRIST_VERSION := v0.2.7
LIBRIST_URL := https://code.videolan.org/rist/librist/-/archive/$(LIBRIST_VERSION)/librist-$(LIBRIST_VERSION).tar.gz

ifdef BUILD_NETWORK
PKGS += librist
endif

DEPS_librist =
ifdef HAVE_WIN32
DEPS_librist += winpthreads $(DEPS_winpthreads)
endif
ifdef HAVE_WINSTORE
# librist uses wincrypt
DEPS_librist += alloweduwp $(DEPS_alloweduwp)
endif

ifeq ($(call need_pkg,"librist >= 0.2"),)
PKGS_FOUND += librist
endif

LIBRIST_CONF = -Dbuilt_tools=false -Dtest=false
ifdef HAVE_WIN32
LIBRIST_CONF += -Dhave_mingw_pthreads=true
endif

$(TARBALLS)/librist-$(LIBRIST_VERSION).tar.gz:
	$(call download_pkg,$(LIBRIST_URL),librist)

.sum-librist: librist-$(LIBRIST_VERSION).tar.gz

librist: librist-$(LIBRIST_VERSION).tar.gz .sum-librist
	$(UNPACK)
	$(APPLY) $(SRC)/librist/librist-fix-libcjson-meson.patch
	$(APPLY) $(SRC)/librist/win32-timing.patch
	$(APPLY) $(SRC)/librist/0001-meson-don-t-force-the-Windows-version-if-it-s-higher.patch
	$(MOVE)

.librist: librist crossfile.meson
	$(MESONCLEAN)
	$(MESON) $(LIBRIST_CONF)
	+$(MESONBUILD)
	touch $@
