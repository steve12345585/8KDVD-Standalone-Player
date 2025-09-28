# SoXR

SOXR_VERSION := 0.1.3
SOXR_URL := $(GITHUB)/chirlu/soxr/archive/refs/tags/$(SOXR_VERSION).tar.gz

PKGS += soxr
ifeq ($(call need_pkg,"soxr >= 0.1"),)
PKGS_FOUND += soxr
endif
DEPS_soxr = ffmpeg $(DEPS_ffmpeg)

$(TARBALLS)/soxr-$(SOXR_VERSION)-Source.tar.xz:
	$(call download_pkg,$(SOXR_URL),soxr)

.sum-soxr: soxr-$(SOXR_VERSION)-Source.tar.xz

soxr: soxr-$(SOXR_VERSION)-Source.tar.xz .sum-soxr
	$(UNPACK)
	$(APPLY) $(SRC)/soxr/0001-always-generate-.pc.patch
	$(APPLY) $(SRC)/soxr/0002-expose-Libs.private-in-.pc.patch
	$(APPLY) $(SRC)/soxr/0003-add-aarch64-support.patch
	$(APPLY) $(SRC)/soxr/0004-arm-fix-SIGILL-when-doing-divisions-on-some-old-arch.patch
	$(APPLY) $(SRC)/soxr/find_ff_pkgconfig.patch
	$(APPLY) $(SRC)/soxr/soxr-check-function.patch
	$(APPLY) $(SRC)/soxr/aarch64.patch
	$(APPLY) $(SRC)/soxr/0001-Allocate-an-extra-real-number.patch
	$(call pkg_static,"src/soxr.pc.in")
	$(MOVE)

SOXR_CONF := \
		-DBUILD_TESTS=OFF \
		-DWITH_LSR_BINDINGS=OFF \
		-DWITH_OPENMP=OFF \
		-DWITH_AVFFT=ON

.soxr: soxr toolchain.cmake
	$(CMAKECLEAN)
	$(HOSTVARS_CMAKE) $(CMAKE) $(SOXR_CONF)
	+$(CMAKEBUILD)
	$(CMAKEINSTALL)
	touch $@
