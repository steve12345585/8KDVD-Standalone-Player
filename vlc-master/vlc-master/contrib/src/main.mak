# Main makefile for VLC 3rd party libraries ("contrib")
# Copyright (C) 2003-2011 the VideoLAN team
#
# This file is under the same license as the vlc package.

all: install

SRC := $(TOPSRC)/src
SRC_BUILT := $(TOPSRC_BUILT)/src
TARBALLS := $(TOPSRC)/tarballs
VLC_TOOLS ?= $(TOPSRC)/../extras/tools/build

CMAKE_GENERATOR ?= Ninja

PKGS_ALL := $(patsubst $(SRC)/%/rules.mak,%,$(wildcard $(SRC)/*/rules.mak))
DATE := $(shell date +%Y%m%d)
VPATH := $(TARBALLS)

# Default Qt version
QTBASE_VERSION_MAJOR := 6.8
QTBASE_VERSION := $(QTBASE_VERSION_MAJOR).3

# Common download locations
GNU ?= https://ftpmirror.gnu.org/gnu
SF := https://downloads.sourceforge.net/project
VIDEOLAN := https://downloads.videolan.org/pub/videolan
CONTRIB_VIDEOLAN := https://downloads.videolan.org/pub/contrib
VIDEOLAN_GIT := https://git.videolan.org/git
GITHUB := https://github.com
GOOGLE_CODE := https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com
GNUGPG := https://www.gnupg.org/ftp/gcrypt
QT := https://download.qt.io/official_releases/qt/$(QTBASE_VERSION_MAJOR)
XIPH := https://ftp.osuosl.org/pub/xiph/releases
XORG := https://www.x.org/releases/individual

#
# Machine-dependent variables
#

PREFIX ?= $(TOPDST)/$(HOST)
PREFIX := $(abspath $(PREFIX))
BUILDPREFIX ?= $(PREFIX)/..
BUILDPREFIX := $(abspath $(BUILDPREFIX))
BUILDBINDIR ?= $(BUILDPREFIX)/bin

SYSTEM_PATH := $(abspath $(VLC_TOOLS)/bin):$(PATH)
PATH :=$(abspath $(BUILDBINDIR)):$(abspath $(BUILDBINDIR)/../libexec):$(SYSTEM_PATH)
export PATH

ifneq ($(HOST),$(BUILD))
HAVE_CROSS_COMPILE = 1
endif

ifeq ($(findstring mingw32,$(BUILD)),mingw32)
MSYS_BUILD := 1
endif
ifeq ($(findstring msys,$(BUILD)),msys)
MSYS_BUILD := 1
endif

#
# Default values for tools
#
ifdef HAVE_CROSS_COMPILE
MAYBEHOST := $(HOST)-
else
MAYBEHOST :=
endif
ifneq ($(findstring $(origin CC),undefined default),)
CC := $(MAYBEHOST)gcc
endif
ifneq ($(findstring $(origin CXX),undefined default),)
CXX := $(MAYBEHOST)g++
endif
ifneq ($(findstring $(origin LD),undefined default),)
LD := $(MAYBEHOST)ld
endif
ifneq ($(findstring $(origin AR),undefined default),)
ifeq ($(shell $(MAYBEHOST)gcc-ar --version >/dev/null 2>&1 || echo Prehistory),)
AR := $(MAYBEHOST)gcc-ar
else
AR := $(MAYBEHOST)ar
endif
endif
ifeq ($(shell $(MAYBEHOST)gcc-nm --version >/dev/null 2>&1 || echo Prehistory),)
NM ?= $(MAYBEHOST)gcc-nm
else
NM ?= $(MAYBEHOST)nm
endif
ifeq ($(shell $(MAYBEHOST)gcc-ranlib --version >/dev/null 2>&1 || echo Prehistory),)
RANLIB ?= $(MAYBEHOST)gcc-ranlib
else
RANLIB ?= $(MAYBEHOST)ranlib
endif
STRIP ?= $(MAYBEHOST)strip
WIDL ?= $(MAYBEHOST)widl
WINDRES ?= $(MAYBEHOST)windres

ifdef HAVE_ANDROID
ifneq ($(findstring $(origin CC),undefined default),)
CC :=  clang
endif
ifneq ($(findstring $(origin CXX),undefined default),)
CXX := clang++
endif
endif

# -fno-stack-check is a workaround for a possible
# bug in Xcode 11 or macOS 10.15+
ifdef HAVE_DARWIN_OS
EXTRA_CFLAGS += -fno-stack-check
XCODE_FLAGS += OTHER_CFLAGS=-fno-stack-check
endif

ifdef HAVE_MACOSX
EXTRA_CXXFLAGS += -stdlib=libc++
ifeq ($(ARCH),aarch64)
XCODE_FLAGS += -arch arm64
else
XCODE_FLAGS += -arch $(ARCH)
endif
endif

ifdef HAVE_EMSCRIPTEN
EXTRA_CFLAGS += -pthread
EXTRA_CXXFLAGS += -pthread
endif

CCAS=$(CC) -c

LN_S = ln -s
ifdef HAVE_WIN32
MINGW_W64_VERSION := $(shell echo "__MINGW64_VERSION_MAJOR" | $(CC) $(CFLAGS) -E -include _mingw.h - | tail -n 1)
ifneq ($(MINGW_W64_VERSION),)
HAVE_MINGW_W64 := 1
mingw_at_least = $(shell [ $(MINGW_W64_VERSION) -gt $(1) ] && echo true)
endif
ifndef HAVE_CROSS_COMPILE
LN_S = cp -R
endif
endif

ifdef HAVE_ANDROID
# Android NDK has vulkan headers but vulkan.h from the sysroot but it is not
# the actual vulkan version from the Android NDK, the proper one is located
# in this third_party folder
EXTRA_CFLAGS += -isystem$(ANDROID_NDK)/sources/third_party/vulkan/src/include
endif

ifdef HAVE_SOLARIS
ifeq ($(ARCH),x86_64)
EXTRA_CFLAGS += -m64
EXTRA_LDFLAGS += -m64
else
EXTRA_CFLAGS += -m32
EXTRA_LDFLAGS += -m32
endif
endif

ifneq ($(findstring clang, $(shell $(CC) --version 2>/dev/null)),)
HAVE_CLANG := 1
CLANG_VERSION := $(shell $(CC) --version | head -1 | grep -o '[0-9]\+\.' | head -1 | cut -d '.' -f 1)
clang_at_least = $(shell [ $(CLANG_VERSION) -ge $(1) ] && echo true)
else
clang_at_least = $(shell echo false)
endif

cppcheck = $(shell printf '$(2)' | $(CC) $(CFLAGS) -E -dM - 2>/dev/null | grep -E $(1))

EXTRA_CFLAGS += -I$(PREFIX)/include
CPPFLAGS := $(CPPFLAGS) $(EXTRA_CFLAGS)
CFLAGS := $(CFLAGS) $(EXTRA_CFLAGS)
CXXFLAGS := $(CXXFLAGS) $(EXTRA_CFLAGS) $(EXTRA_CXXFLAGS)
LDFLAGS := $(LDFLAGS) -L$(PREFIX)/lib $(EXTRA_LDFLAGS)

ifdef ENABLE_PDB
ifdef HAVE_CLANG
CFLAGS := $(CFLAGS) -gcodeview
CXXFLAGS := $(CXXFLAGS) -gcodeview
endif
endif

ifeq ($(shell gcc --version >/dev/null 2>&1 || echo No GCC),)
BUILDCC ?= gcc
BUILDCXX ?= g++
ifeq ($(shell gcc-ar --version >/dev/null 2>&1 || echo Prehistoric GCC),)
BUILDAR ?= gcc-ar
BUILDNM ?= gcc-nm
BUILDRANLIB ?= gcc-ranlib
endif
else ifeq ($(shell clang --version >/dev/null 2>&1 || No LLVM/Clang),)
BUILDCC ?= clang
BUILDCXX ?= clang++
ifeq ($(shell llvm-ar --version >/dev/null 2>&1 || echo Prehistoric LLVM),)
BUILDAR ?= llvm-ar
BUILDNM ?= llvm-nm
BUILDRANLIB ?= llvm-ranlib
BUILDSTRIP ?= llvm-strip
endif
endif

BUILDCC ?= cc
BUILDCXX ?= c++
BUILDLD ?= $(BUILDCC)
BUILDAR ?= ar
BUILDNM ?= nm
BUILDRANLIB ?= ranlib
BUILDSTRIP ?= strip

BUILDCPPFLAGS ?=
BUILDCFLAGS ?= -O2
BUILDCXXFLAGS ?= $(BUILDCFLAGS)
BUILDLDFLAGS ?= $(BUILDCFLAGS)

# Do not export variables above! Use HOSTVARS or BUILDVARS.

ifdef HAVE_WIN32
define UCRT_HEADER_CHECK :=
#include <crtdefs.h> \n
#if defined(_UCRT) || (__MSVCRT_VERSION__ >= 0x1400) || (__MSVCRT_VERSION__ >= 0xE00 && __MSVCRT_VERSION__ < 0x1000) \n
# undef _UCRT \n
# define _UCRT \n
#endif \n
endef
ifneq ($(call cppcheck, _UCRT, $(UCRT_HEADER_CHECK)),)
HAVE_UCRT = 1
endif
endif

# Do the FPU detection, after we have figured out our compilers and flags.
ifneq ($(findstring $(ARCH),aarch64 i386 ppc ppc64 ppc64le sparc sparc64 x86_64),)
# This should be consistent with include/vlc_cpu.h
HAVE_FPU = 1
else ifneq ($(findstring $(ARCH),arm),)
ifneq ($(call cppcheck, __VFP_FP__)),)
ifeq ($(call cppcheck, __SOFTFP__),)
HAVE_FPU = 1
endif
endif
else ifneq ($(filter riscv%, $(ARCH)),)
ifneq ($(call cppcheck, __riscv_flen),)
HAVE_FPU = 1
endif
else ifneq ($(call cppcheck, __mips_hard_float),)
HAVE_FPU = 1
endif

ifneq ($(wildcard $(VLC_TOOLS)/share/aclocal/*),)
VLC_ACLOCAL_PATH := $(PREFIX)/share/aclocal:$(abspath $(VLC_TOOLS)/share/aclocal):${ACLOCAL_PATH}
else
VLC_ACLOCAL_PATH := $(PREFIX)/share/aclocal:${ACLOCAL_PATH}
endif
export ACLOCAL_PATH=${VLC_ACLOCAL_PATH}

#########
# Tools #
#########

ifdef HAVE_CROSS_COMPILE
# Use pkg-config cross-tool if it actually works
ifeq ($(shell unset PKG_CONFIG_LIBDIR; $(HOST)-pkg-config --version 1>/dev/null 2>/dev/null || echo FAIL),)
PKG_CONFIG ?= $(HOST)-pkg-config
else
# Use the regular pkg-config and set some PKG_CONFIG_LIBDIR ourselves
PKG_CONFIG_LIBDIR ?= /usr/$(HOST)/lib/pkgconfig:/usr/lib/$(HOST)/pkgconfig:/usr/share/pkgconfig
export PKG_CONFIG_LIBDIR
need_pkg = $(shell PKG_CONFIG_PATH=$(SYSTEM_PKG_CONFIG_PATH) PKG_CONFIG_LIBDIR=$(PKG_CONFIG_LIBDIR) $(PKG_CONFIG) $(1) || echo 1)
endif
endif # HAVE_CROSS_COMPILE

need_pkg ?= $(shell PKG_CONFIG_PATH=$(SYSTEM_PKG_CONFIG_PATH) $(PKG_CONFIG) $(1) || echo 1)
PKG_CONFIG ?= pkg-config

SYSTEM_PKG_CONFIG_PATH := $(PKG_CONFIG_PATH)
PKG_CONFIG_PATH := $(PREFIX)/lib/pkgconfig:$(PREFIX)/share/pkgconfig:$(PKG_CONFIG_PATH)
export PKG_CONFIG_PATH

# Get the version of a system tool $1 and pass it through the $2 command(s)
FULL_VERSION_REGEX := 's/[^0-9]*\([0-9]\([0-9a-zA-Z\.\-]*\)\)\(.*\)/\1/p'
system_tool_version = $(shell PATH="${SYSTEM_PATH}" $(1) 2>/dev/null | head -1 | sed -ne ${FULL_VERSION_REGEX} | $(2))
# Get the major.minor version of a system tool
system_tool_majmin = $(call system_tool_version, $(1), cut -d '.' -f -2)
# Print the smallest version value of the given system tool (no spaces in the checked version)
system_tool_min_version = $(shell printf "$(2) $(call system_tool_version, $(1), grep . && echo || echo 0.0.0)" | tr " " "\n" | sort -V | head -n1)
# Check if native tool $1 is at least version $2
system_tool_matches_min = $(shell test "$(call system_tool_min_version,$(1),$(2))" = "$(2)" || echo FAIL)

ifndef GIT
ifeq ($(shell git --version >/dev/null 2>&1 || echo FAIL),)
GIT = git
endif
endif
GIT ?= $(error git not found)

ifeq ($(shell curl --version >/dev/null 2>&1 || echo FAIL),)
download = curl -f -L --retry 3 --output "$@" -- "$(1)"
else ifeq ($(shell wget --version >/dev/null 2>&1 || echo FAIL),)
download = (rm -f $@.tmp && \
	wget --passive -c -p -O $@.tmp "$(1)" && \
	touch $@.tmp && \
	mv $@.tmp $@ )
else ifeq ($(command -v fetch >/dev/null 2>&1 || echo FAIL),)
download = (rm -f $@.tmp && \
	fetch -p -o $@.tmp "$(1)" && \
	touch $@.tmp && \
	mv $@.tmp $@)
else
download = $(error Neither curl nor wget found)
endif

download_pkg = $(call download,$(CONTRIB_VIDEOLAN)/$(2)/$(lastword $(subst /, ,$(@)))) || \
	( $(call download,$(1)) && echo "Please upload this package $(lastword $(subst /, ,$(@))) to our FTP" )

ifeq ($(shell command -v xz >/dev/null 2>&1 || echo FAIL),)
XZ = xz
else
XZ ?= $(error XZ (LZMA) compressor not found)
endif

ifeq ($(shell sha512sum --version >/dev/null 2>&1 || echo FAIL),)
SHA512SUM = sha512sum --check
else ifeq ($(shell shasum --version >/dev/null 2>&1 || echo FAIL),)
SHA512SUM = shasum -a 512 --check
else ifeq ($(shell openssl version >/dev/null 2>&1 || echo FAIL),)
SHA512SUM = openssl dgst -sha512
else
SHA512SUM = $(error SHA-512 checksumming not found)
endif

#
# Common helpers
#
HOSTCONF := --prefix="$(PREFIX)"
HOSTCONF += --datarootdir="$(PREFIX)/share"
HOSTCONF += --includedir="$(PREFIX)/include"
HOSTCONF += --libdir="$(PREFIX)/lib"
HOSTCONF += --build="$(BUILD)" --host="$(HOST)" --target="$(HOST)"
HOSTCONF += --program-prefix=""
# libtool stuff:
HOSTCONF += --enable-static --disable-shared --disable-dependency-tracking
ifdef HAVE_WIN32
HOSTCONF += --without-pic
PIC :=
else
HOSTCONF += --with-pic
PIC := -fPIC
endif

HOSTTOOLS := \
	CC="$(CC)" CXX="$(CXX)" OBJC="$(OBJC)" LD="$(LD)" \
	AR="$(AR)" CCAS="$(CCAS)" RANLIB="$(RANLIB)" STRIP="$(STRIP)" \
	PKG_CONFIG="$(PKG_CONFIG)"

ifdef HAVE_BITCODE_ENABLED
CFLAGS := $(CFLAGS) -fembed-bitcode
CXXFLAGS := $(CXXFLAGS) -fembed-bitcode
endif

# Add these flags after CMake consumed the CFLAGS/CXXFLAGS
# CMake handles the optimization level with CMAKE_BUILD_TYPE
HOSTVARS_CMAKE := \
	CPPFLAGS="$(CPPFLAGS)" \
	CFLAGS="$(CFLAGS)" \
	CXXFLAGS="$(CXXFLAGS)" \
	LDFLAGS="$(LDFLAGS)"

# Add these flags after Meson consumed the CFLAGS/CXXFLAGS
# as when setting those for Meson, it would apply to tests
# and cause the check if symbols have underscore prefix to
# incorrectly report they have not, even if they have.
ifndef WITH_OPTIMIZATION
CFLAGS := $(CFLAGS) -g -O0
CXXFLAGS := $(CXXFLAGS) -g -O0
else
CFLAGS := $(CFLAGS) -g -O2
CXXFLAGS := $(CXXFLAGS) -g -O2
endif

HOSTVARS := $(HOSTTOOLS) \
	CPPFLAGS="$(CPPFLAGS)" \
	CFLAGS="$(CFLAGS)" \
	CXXFLAGS="$(CXXFLAGS)" \
	LDFLAGS="$(LDFLAGS)"
HOSTVARS_PIC := $(HOSTTOOLS) \
	CPPFLAGS="$(CPPFLAGS) $(PIC)" \
	CFLAGS="$(CFLAGS) $(PIC)" \
	CXXFLAGS="$(CXXFLAGS) $(PIC)" \
	LDFLAGS="$(LDFLAGS)"

BUILDCOMMONCONF := --disable-dependency-tracking
BUILDCOMMONCONF += --prefix="$(BUILDPREFIX)"
BUILDCOMMONCONF += --bindir="$(BUILDBINDIR)"
BUILDCOMMONCONF += --datarootdir="$(BUILDPREFIX)/share"
BUILDCOMMONCONF += --includedir="$(BUILDPREFIX)/include"
BUILDCOMMONCONF += --libdir="$(BUILDPREFIX)"
BUILDCOMMONCONF += --build="$(BUILD)" --host="$(BUILD)"
# For platform-independent tools (--target should be meaningless):
BUILDPROGCONF := $(BUILDCOMMONCONF) \
	--target="$(BUILD)" --program-prefix=""
# For platform-dependent tools:
BUILDTOOLCONF := $(BUILDCOMMONCONF) \
	--target="$(HOST)" --program-prefix="$(HOST)-"

BUILDTOOLS := \
	CC="$(BUILDCC)" CXX="$(BUILDCXX)" LD="$(BUILDLD)" \
	AR="$(BUILDAR)" NM="$(BUILDNM)" RANLIB="$(BUILDRANLIB)" \
	STRIP="$(BUILDSTRIP)" PATH="$(BUILDBINDIR):$(PATH)"

BUILDVARS := $(BUILDTOOLS) \
	CPPFLAGS="$(BUILDCPPFLAGS)" \
	CFLAGS="$(BUILDCFLAGS)" \
	CXXFLAGS="$(BUILDCXXFLAGS)" \
	LDFLAGS="$(BUILDLDFLAGS)"

download_git = \
	rm -Rf -- "$(@:.tar.xz=)" && \
	$(GIT) init --bare "$(@:.tar.xz=)" && \
	(cd "$(@:.tar.xz=)" && \
	$(GIT) remote add origin "$(1)" && \
	$(GIT) fetch origin "$(2)") && \
	(cd "$(@:.tar.xz=)" && \
	$(GIT) archive --prefix="$(notdir $(@:.tar.xz=))/" \
		--format=tar "$(3)") > "$(@:.xz=)" && \
	echo "$(3) $(@)" > "$(@:.tar.xz=.githash)" && \
	rm -Rf -- "$(@:.tar.xz=)" && \
	$(XZ) --stdout "$(@:.xz=)" > "$@.tmp" && \
	rm -f "$(@:.xz=)" && \
	mv -f -- "$@.tmp" "$@"
check_githash = \
	h=`sed -e "s,^\([0-9a-fA-F]\{40\}\) .*/$(notdir $<),\1,g" \
		< "$(<:.tar.xz=.githash)"` && \
	test "$$h" = "$1"

ifeq ($(V),1)
TAR_VERBOSE := v
endif

checksum = \
	$(foreach f,$(filter $(TARBALLS)/%,$^), \
		grep -- " $(f:$(TARBALLS)/%=%)$$" \
			"$(SRC)/$(patsubst $(3)%,%,$@)/$(2)SUMS" &&) \
	(cd $(TARBALLS) && $(1) /dev/stdin) < \
		"$(SRC)/$(patsubst $(3)%,%,$@)/$(2)SUMS"
CHECK_SHA512 = $(call checksum,$(SHA512SUM),SHA512,.sum-)
UNPACK = $(RM) -R $@ \
	$(foreach f,$(filter %.tar.gz %.tgz,$^), && tar $(TAR_VERBOSE)xzfo $(f)) \
	$(foreach f,$(filter %.tar.bz2,$^), && tar $(TAR_VERBOSE)xjfo $(f)) \
	$(foreach f,$(filter %.tar.xz,$^), && tar $(TAR_VERBOSE)xJfo $(f)) \
	$(foreach f,$(filter %.zip,$^), && unzip $(f) $(UNZIP_PARAMS))
UNPACK_DIR = $(patsubst %.tar,%,$(basename $(notdir $<)))
APPLY = (cd $(UNPACK_DIR) && patch -fp1) <
pkg_static = (cd $(UNPACK_DIR) && $(SRC_BUILT)/pkg-static.sh $(1))
MOVE = mv $(UNPACK_DIR) $@ && touch $@

AUTOMAKE_DATA_DIRS:=$(foreach n,$(foreach n,$(subst :, ,$(shell echo $$PATH)),$(abspath $(n)/../share)),$(wildcard $(n)/autoconf*/build-aux))
update_autoconfig = \
	for dir in $(AUTOMAKE_DATA_DIRS); do \
		if test -f "$${dir}/config.sub" -a -f "$${dir}/config.guess"; then \
			install -p "$${dir}/config.guess" "$(UNPACK_DIR)/$(1)/"; \
			install -p "$${dir}/config.sub"   "$(UNPACK_DIR)/$(1)/"; \
			break; \
		fi; \
	done

ifneq ($(wildcard $(abspath $(VLC_TOOLS))/share/autoconf-vlc/build-aux),)
VLC_CONFIG_GUESS := autom4te_buildauxdir=$(abspath $(VLC_TOOLS))/share/autoconf-vlc/build-aux
endif

AUTORECONF = GTKDOCIZE=true autoreconf
RECONF = mkdir -p -- $(PREFIX)/share/aclocal && \
	cd $< && $(VLC_CONFIG_GUESS) $(AUTORECONF) -fiv

BUILD_DIR = $</vlc_build
BUILD_SRC := ..
# build directory relative to UNPACK_DIR
BUILD_DIRUNPACK := vlc_build

MAKEBUILDDIR = mkdir -p $(BUILD_DIR) && rm -f $(BUILD_DIR)/config.status && test ! -f $</config.status || $(MAKE) -C $< distclean
MAKEBUILD = $(MAKE) -C $(BUILD_DIR)
MAKECONFDIR = cd $(BUILD_DIR) && $(HOSTVARS) $(BUILD_SRC)
MAKECONFIGURE = $(MAKECONFDIR)/configure $(HOSTCONF)

# Work around for https://lists.nongnu.org/archive/html/bug-gnulib/2020-05/msg00237.html
# When using a single command, make might take a shortcut and fork/exec
# itself instead of relying on a shell, but a bug in gnulib ends up
# trying to execute a cmake folder when one is found in the PATH
CMAKEBUILD = env cmake --build $(BUILD_DIR)
CMAKEINSTALL = env cmake --install $(BUILD_DIR)
CMAKECLEAN = rm -f $(BUILD_DIR)/CMakeCache.txt
CMAKECONFIG = cmake -S $< -B $(BUILD_DIR) \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		-DBUILD_SHARED_LIBS:BOOL=OFF \
		-DCMAKE_INSTALL_LIBDIR:STRING=lib \
		-DBUILD_TESTING:BOOL=OFF \
		-G $(CMAKE_GENERATOR)
ifeq ($(V),1)
CMAKECONFIG += -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
endif

CMAKE = $(CMAKECONFIG) \
		-DCMAKE_TOOLCHAIN_FILE=$(abspath toolchain.cmake) \
		-DCMAKE_INSTALL_PREFIX:STRING=$(PREFIX)
CMAKE_NATIVE = $(CMAKECONFIG) \
		-DCMAKE_INSTALL_PREFIX:STRING=$(BUILDPREFIX)
ifndef WITH_OPTIMIZATION
CMAKE += -DCMAKE_BUILD_TYPE=Debug
else
CMAKE += -DCMAKE_BUILD_TYPE=RelWithDebInfo
endif
ifdef HAVE_WIN32
CMAKE += -DCMAKE_DEBUG_POSTFIX:STRING=
endif
ifdef MSYS_BUILD
CMAKE = PKG_CONFIG_LIBDIR="$(PKG_CONFIG_PATH)" $(CMAKE)
CMAKE += -DCMAKE_LINK_LIBRARY_SUFFIX:STRING=.a
endif

MESONFLAGS = $(BUILD_DIR) $< --default-library static --prefix "$(PREFIX)" \
	--backend ninja -Dlibdir=lib -Dcmake_prefix_path="$(PREFIX)"
ifndef WITH_OPTIMIZATION
MESONFLAGS += --buildtype debug
else
MESONFLAGS += --buildtype debugoptimized
endif
ifdef HAVE_BITCODE_ENABLED
MESONFLAGS += -Db_bitcode=true
endif
MESONFLAGS += -Dc_args="$(CFLAGS)" -Dc_link_args="$(LDFLAGS)" -Dcpp_args="$(CXXFLAGS)" -Dcpp_link_args="$(LDFLAGS)"
ifdef HAVE_DARWIN_OS
MESONFLAGS += -Dobjc_args="$(CFLAGS)" -Dobjc_link_args="$(LDFLAGS)" -Dobjcpp_args="$(CXXFLAGS)" -Dobjcpp_link_args="$(LDFLAGS)"
endif

ifdef HAVE_CROSS_COMPILE
# When cross-compiling meson uses the env vars like
# CC, CXX, etc. and CFLAGS, CXXFLAGS, etc. for the
# build machine compiler and not like most other
# buildsystems for the host compilation. Therefore
# we clear the enviornment variables using the env
# command, except PATH, which is needed.
# The values of the mentioned relevant env variables
# are passed for the host compilation using the
# generated crossfile, so everything should work as
# expected.
MESONFLAGS += --cross-file $(abspath crossfile.meson)
MESON = env -i PATH="$(PATH)" \
	meson setup -Dpkg_config_path="$(PKG_CONFIG_PATH)" \
	$(MESONFLAGS)

else
MESON = $(HOSTTOOLS) meson setup $(MESONFLAGS)
endif
MESONCLEAN = rm -rf $(BUILD_DIR)/meson-private
MESONBUILD = meson compile -C $(BUILD_DIR) $(MESON_BUILD) && meson install -C $(BUILD_DIR)

# shared Qt config
ifeq ($(call system_tool_majmin, qmake6 -query QT_VERSION 2>/dev/null),$(QTBASE_VERSION_MAJOR))

ifdef HAVE_CROSS_COMPILE
QT_LIBEXECS := $(shell qmake6 -query QT_HOST_LIBEXECS)
QT_BINS := $(shell qmake6 -query QT_HOST_BINS)
else
QT_LIBEXECS := $(shell qmake6 -query QT_INSTALL_LIBEXECS):$(shell qmake6 -query QT_HOST_LIBEXECS)
QT_BINS := $(shell qmake6 -query QT_INSTALL_BINS):$(shell qmake6 -query QT_HOST_BINS)
endif

ifeq ($(call system_tool_majmin, PATH="${QT_LIBEXECS}" moc --version),$(QTBASE_VERSION_MAJOR))
ifeq ($(call system_tool_majmin, PATH="${QT_BINS}" qsb --version),$(QTBASE_VERSION_MAJOR))
ifeq ($(call system_tool_majmin, PATH="${QT_LIBEXECS}" qmlcachegen --version),$(QTBASE_VERSION_MAJOR))
QT_USES_SYSTEM_TOOLS = 1
endif
endif
endif
endif

ifdef HAVE_CROSS_COMPILE
ifdef QT_USES_SYSTEM_TOOLS
 # using system Qt native tools
 QT_HOST_PREFIX := $(shell PATH="${SYSTEM_PATH}" qmake6 -query QT_HOST_PREFIX)
 QT_HOST_LIBS := $(shell PATH="${SYSTEM_PATH}" qmake6 -query QT_HOST_LIBS)
else
 # using locally compiled Qt native tools
 QT_HOST_PREFIX := $(BUILDPREFIX)
 QT_HOST_LIBS := $(QT_HOST_PREFIX)/lib
endif
QT_HOST_PATH := -DQT_HOST_PATH=$(QT_HOST_PREFIX) -DQT_HOST_PATH_CMAKE_DIR=$(QT_HOST_LIBS)/cmake
endif
QT_CMAKE_CONFIG := -DCMAKE_TOOLCHAIN_FILE=$(PREFIX)/lib/cmake/Qt6/qt.toolchain.cmake $(QT_HOST_PATH)
ifdef QT_USES_SYSTEM_TOOLS
# We checked the versions match, assume we know what we're going
QT_CMAKE_CONFIG += -DQT_NO_PACKAGE_VERSION_CHECK=TRUE
endif

ifdef GPL
REQUIRE_GPL =
else
REQUIRE_GPL = @echo "Package \"$<\" requires the GPL license." >&2; exit 1
endif
ifdef GNUV3
REQUIRE_GNUV3 =
else
REQUIRE_GNUV3 = \
	@echo "Package \"$<\" requires the version 3 of GNU licenses." >&2; \
	exit 1
endif

PYTHON_VENV = $(BUILDPREFIX)/python-venv
PYTHON_ACTIVATE = . $(PYTHON_VENV)/bin/activate
PYTHON_INSTALL = $(HOSTVARS) $(PYTHON_VENV)/bin/pip3 install ./$<

.python-venv:
	python3 -m venv $(PYTHON_VENV)
	touch $@

#
# Rust specific rules
#
include $(SRC)/main-rust.mak

#
# Per-package build rules
#
PKGS_FOUND :=
include $(SRC)/*/rules.mak

ifeq ($(PKGS_DISABLE), all)
PKGS :=
endif
#
# Targets
#
ifneq ($(filter $(PKGS_DISABLE),$(PKGS_ENABLE)),)
$(error Same package(s) disabled and enabled at the same time)
endif
# Apply automatic selection (= remove distro packages):
PKGS_AUTOMATIC := $(filter-out $(PKGS_FOUND),$(PKGS))
# Apply manual selection (from bootstrap):
PKGS_MANUAL := $(sort $(PKGS_ENABLE) $(filter-out $(PKGS_DISABLE),$(PKGS_AUTOMATIC)))
# Resolve dependencies:
PKGS_DEPS := $(filter-out $(PKGS_FOUND) $(PKGS_MANUAL),$(sort $(foreach p,$(PKGS_MANUAL),$(DEPS_$(p)))))
PKGS := $(sort $(PKGS_MANUAL) $(PKGS_DEPS))

fetch: $(PKGS:%=.sum-%)
fetch-all: $(PKGS_ALL:%=.sum-%)
install: $(PKGS:%=.%)
tools: $(PKGS_TOOLS:%=.dep-%)

mostlyclean:
	-$(RM) $(foreach p,$(PKGS_ALL),.$(p) .sum-$(p) .dep-$(p))
	-$(RM) -R "$(PYTHON_VENV)"
	-$(RM) .python-venv
	-$(RM) toolchain.cmake
	-$(RM) crossfile.meson
	-$(RM) -R "$(PREFIX)"
	-$(RM) "$(BUILDBINDIR)/$(HOST)-*"
	-$(RM) -R */

clean: mostlyclean
	-$(RM) $(TARBALLS)/*.*

distclean: clean
	$(RM) config.mak
	unlink Makefile

PREBUILT_URL=https://download.videolan.org/pub/videolan/contrib/$(HOST)/vlc-contrib-$(HOST)-latest.tar.bz2

vlc-contrib-$(HOST)-latest.tar.bz2:
	$(call download,$(PREBUILT_URL))

prebuilt: vlc-contrib-$(HOST)-latest.tar.bz2
	$(RM) -r $(PREFIX)
	$(UNPACK)
	mv $(HOST) $(PREFIX)
	cd $(PREFIX) && $(abspath $(SRC))/change_prefix.sh

package: install
	rm -Rf tmp/
	mkdir -p tmp/
	cp -R $(PREFIX) tmp/
	# remove useless files
	cd tmp/$(notdir $(PREFIX)); \
		cd share; rm -Rf man doc gtk-doc info lua projectM; cd ..; \
		rm -Rf man sbin etc lib/lua lib/sidplay
	cd tmp/$(notdir $(PREFIX)) && $(abspath $(SRC))/change_prefix.sh $(PREFIX)
ifneq ($(notdir $(PREFIX)),$(HOST))
	(cd tmp && mv $(notdir $(PREFIX)) $(HOST))
endif
	(cd tmp && tar c $(HOST)/) | bzip2 -c > ../vlc-contrib-$(HOST)-$(DATE).tar.bz2

list:
	@echo All packages:
	@echo '  $(PKGS_ALL)' | tr " " "\n" | sort | tr "\n" " " |fmt
	@echo All native tools:
	@echo '  $(PKGS_TOOLS)' | tr " " "\n" | sort | tr "\n" " " |fmt
	@echo Distribution-provided packages:
	@echo '  $(PKGS_FOUND)' | tr " " "\n" | sort | tr "\n" " " |fmt
	@echo Automatically selected packages:
	@echo '  $(PKGS_AUTOMATIC)' | tr " " "\n" | sort | tr "\n" " " |fmt
	@echo Manually deselected packages:
	@echo '  $(PKGS_DISABLE)' | tr " " "\n" | sort | tr "\n" " " |fmt
	@echo Manually selected packages:
	@echo '  $(PKGS_ENABLE)' | tr " " "\n" | sort | tr "\n" " " |fmt
	@echo Depended-on packages:
	@echo '  $(PKGS_DEPS)' | tr " " "\n" | sort | tr "\n" " " |fmt
	@echo To-be-built packages:
	@echo '  $(PKGS)' | tr " " "\n" | sort | tr "\n" " " |fmt

help:
	@cat $(SRC)/help.txt

.PHONY: all fetch fetch-all install mostlyclean clean distclean package list help prebuilt tools

CMAKE_HOST_ARCH=$(ARCH)
ifeq ($(ARCH),i386)
CMAKE_HOST_ARCH=i686
else ifeq ($(ARCH),arm)
CMAKE_HOST_ARCH=armv7-a
endif

CMAKE_SYSTEM_NAME =
ifdef HAVE_CROSS_COMPILE
CMAKE_SYSTEM_NAME = $(error CMAKE_SYSTEM_NAME required for cross-compilation)
endif
ifdef HAVE_ANDROID
CMAKE_SYSTEM_NAME = Android
else
CMAKE_SYSTEM_NAME = Linux
endif
ifdef HAVE_WIN32
CMAKE_SYSTEM_NAME = Windows
ifdef HAVE_VISUALSTUDIO
ifdef HAVE_WINSTORE
CMAKE_SYSTEM_NAME = WindowsStore
endif
endif
endif
ifdef HAVE_DARWIN_OS
CMAKE_SYSTEM_NAME = Darwin
endif
ifdef HAVE_EMSCRIPTEN
CMAKE_SYSTEM_NAME = Emscripten
EMCMAKE_PATH := $(shell command -v emcmake)
EMSDK_PATH := $(dir $(EMCMAKE_PATH))
endif

ifdef HAVE_ANDROID
CFLAGS += -DANDROID_NATIVE_API_LEVEL=$(ANDROID_API)
endif

# CMake toolchain
CMAKE_TOOLCHAIN_ENV := $(HOSTTOOLS) HOST_ARCH="$(CMAKE_HOST_ARCH)" SYSTEM_NAME="$(CMAKE_SYSTEM_NAME)"
ifdef HAVE_WIN32
ifdef HAVE_CROSS_COMPILE
	CMAKE_TOOLCHAIN_ENV += RC_COMPILER="$(WINDRES)"
endif
endif
ifdef HAVE_DARWIN_OS
ifdef HAVE_IOS
	CMAKE_TOOLCHAIN_ENV += OSX_SYSROOT="$(IOS_SDK)"
else
	CMAKE_TOOLCHAIN_ENV += OSX_SYSROOT="$(MACOSX_SDK)"
endif
endif
ifdef HAVE_CROSS_COMPILE
	CMAKE_TOOLCHAIN_ENV += TOOLCHAIN_PREFIX="$(HOST)-"
	CMAKE_TOOLCHAIN_ENV += PATH_MODE_LIBRARY="ONLY"
	CMAKE_TOOLCHAIN_ENV += PATH_MODE_INCLUDE="ONLY"
	CMAKE_TOOLCHAIN_ENV += PATH_MODE_PACKAGE="ONLY"
endif
ifdef HAVE_ANDROID
	CMAKE_TOOLCHAIN_ENV += ANDROID_NDK=$(ANDROID_NDK)
	CMAKE_TOOLCHAIN_ENV += ANDROID_ABI=$(ANDROID_ABI)
	CMAKE_TOOLCHAIN_ENV += ANDROID_API=$(ANDROID_API)
	CMAKE_TOOLCHAIN_ENV += EXTRA_INCLUDE=$(ANDROID_NDK)/build/cmake/android.toolchain.cmake
endif
ifdef MSYS_BUILD
	CMAKE_TOOLCHAIN_ENV += FIND_ROOT_PATH="$(shell cygpath -m $(PREFIX))"
else
	CMAKE_TOOLCHAIN_ENV += FIND_ROOT_PATH="$(PREFIX)"
endif
ifdef HAVE_EMSCRIPTEN
	CMAKE_TOOLCHAIN_ENV += EXTRA_INCLUDE="$(EMSDK_PATH)cmake/Modules/Platform/Emscripten.cmake"
endif

toolchain.cmake: $(SRC)/gen-cmake-toolchain.py
	$(CMAKE_TOOLCHAIN_ENV) $(SRC)/gen-cmake-toolchain.py $@
	cat $@

MESON_SYSTEM_NAME =
ifdef HAVE_WIN32
	MESON_SYSTEM_NAME = windows
else
ifdef HAVE_DARWIN_OS
	MESON_SYSTEM_NAME = darwin
else
ifdef HAVE_ANDROID
	MESON_SYSTEM_NAME = android
else
ifdef HAVE_LINUX
	# android has also system = linux and defines HAVE_LINUX
	MESON_SYSTEM_NAME = linux
else
ifdef HAVE_EMSCRIPTEN
	MESON_SYSTEM_NAME = emscripten
else
	$(error "No meson system name known for this target")
endif
endif
endif
endif
endif

crossfile.meson: $(SRC)/gen-meson-machinefile.py
	$(HOSTTOOLS) \
	CMAKE="$(shell command -v cmake)" \
	WINDRES="$(WINDRES)" \
	PKG_CONFIG="$(PKG_CONFIG)" \
	HOST_SYSTEM="$(MESON_SYSTEM_NAME)" \
	HOST_ARCH="$(subst i386,x86,$(ARCH))" \
	HOST="$(HOST)" \
	$(SRC)/gen-meson-machinefile.py $@
	cat $@

# Default pattern rules
.sum-%: $(SRC)/%/SHA512SUMS
	$(CHECK_SHA512)
	touch $@

.sum-%:
	$(error Download and check target not defined for $*)

# Dummy dependency on found packages
$(patsubst %,.dep-%,$(PKGS_FOUND)): .dep-%:
	touch $@

# Real dependency on missing packages
$(patsubst %,.dep-%,$(filter-out $(PKGS_FOUND),$(PKGS_ALL))): .dep-%: .%
	touch -r $< $@

.SECONDEXPANSION:

# Dependency propagation (convert 'DEPS_foo = bar' to '.foo: .bar')
$(foreach p,$(PKGS_ALL),.$(p)): .%: $$(foreach d,$$(DEPS_$$*),.dep-$$(d))

.DELETE_ON_ERROR:
