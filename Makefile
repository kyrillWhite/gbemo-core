# Build configuration
ARCH ?= x64
# Shipped releases are the static library only; the DLL is opt-in because a
# C++ class across a DLL boundary binds the consumer to this exact compiler and
# runtime version.
SHARED ?= 0
VERSION ?= $(shell git describe --tags --always --dirty 2>/dev/null || echo dev)

# "make x86 release": the selector words set variables and build nothing
# themselves, so the real goal is built once rather than once per selector and
# once more for the defaults.
SELECTORS := x86 x64 shared
GOALS := $(filter-out $(SELECTORS),$(MAKECMDGOALS))
ifneq ($(filter x86,$(MAKECMDGOALS)),)
	ARCH := x86
endif
ifneq ($(filter x64,$(MAKECMDGOALS)),)
	ARCH := x64
endif
ifneq ($(filter shared,$(MAKECMDGOALS)),)
	SHARED := 1
endif

MKDIR := mkdir -p
RM := rm -rf
CP := cp -f
AR ?= ar
ZIP ?= zip
DLL := .dll

ARCH_FLAGS :=
ifeq ($(ARCH),x86)
	ARCH_FLAGS := -m32
endif

CXX ?= g++

MINGW32_BIN ?= /c/msys64/mingw32/bin
ifeq ($(ARCH),x86)
	ifneq ($(shell which i686-w64-mingw32-g++ 2>/dev/null),)
		CXX := i686-w64-mingw32-g++
	else ifneq ($(wildcard $(MINGW32_BIN)/g++.exe),)
		CXX := $(MINGW32_BIN)/g++.exe
		# cc1plus sits outside bin/ and resolves its runtime DLLs through PATH.
		# With mingw64/bin ahead of us it loads the 64-bit ones and dies without
		# printing anything, so put the 32-bit environment first.
		export PATH := $(MINGW32_BIN):$(PATH)
	else
		$(error x86 build requires i686-w64-mingw32-g++ on PATH, or an MSYS2 \
			MINGW32 install at MINGW32_BIN (currently $(MINGW32_BIN)))
	endif

	# MSYS2 MINGW32 ships the cross-named g++ but not a cross-named ar.
	ifneq ($(shell which i686-w64-mingw32-ar 2>/dev/null),)
		AR := i686-w64-mingw32-ar
	else ifneq ($(wildcard $(MINGW32_BIN)/ar.exe),)
		AR := $(MINGW32_BIN)/ar.exe
	endif
endif

CXXFLAGS_BASE := -std=c++20 -Iinclude -Isrc $(ARCH_FLAGS)
LDFLAGS := $(ARCH_FLAGS)

# Sources live in per-subsystem directories under src/; headers are included by
# their path relative to src/ (e.g. "cpu/alu.h"), public ones from include/.
MY_SRC := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJ_DIR_DEBUG = obj/debug/$(ARCH)
OBJ_DIR_RELEASE = obj/release/$(ARCH)
BIN_DIR_DEBUG = bin/debug/$(ARCH)
BIN_DIR_RELEASE = bin/release/$(ARCH)
MY_OBJ_DEBUG = $(patsubst src/%.cpp,$(OBJ_DIR_DEBUG)/%.o,$(MY_SRC))
MY_OBJ_RELEASE = $(patsubst src/%.cpp,$(OBJ_DIR_RELEASE)/%.o,$(MY_SRC))

LIB_DEBUG = $(BIN_DIR_DEBUG)/libgbemo.a
LIB_RELEASE = $(BIN_DIR_RELEASE)/libgbemo.a
DLL_DEBUG = $(BIN_DIR_DEBUG)/gbemo$(DLL)
DLL_RELEASE = $(BIN_DIR_RELEASE)/gbemo$(DLL)

SHARED_DEBUG :=
SHARED_RELEASE :=
ifeq ($(SHARED),1)
	SHARED_DEBUG := $(DLL_DEBUG)
	SHARED_RELEASE := $(DLL_RELEASE)
endif

# What a release ships: the public headers plus the optimized static library.
PUBLIC_HEADERS := $(wildcard include/*.h)
DIST_NAME := gbemo-$(VERSION)-$(ARCH)
DIST_ROOT := dist
DIST_DIR := $(DIST_ROOT)/$(DIST_NAME)

.PHONY: all debug release clean dist x86 x64 shared

all: debug

debug: CXXFLAGS := $(CXXFLAGS_BASE) -D_DEBUG -g
debug: $(LIB_DEBUG) $(SHARED_DEBUG)

release: CXXFLAGS := $(CXXFLAGS_BASE) -O3
release: $(LIB_RELEASE) $(SHARED_RELEASE)

# Static library
$(LIB_DEBUG): $(MY_OBJ_DEBUG) | $(BIN_DIR_DEBUG)
	$(AR) rcs $@ $(MY_OBJ_DEBUG)

$(LIB_RELEASE): $(MY_OBJ_RELEASE) | $(BIN_DIR_RELEASE)
	$(AR) rcs $@ $(MY_OBJ_RELEASE)

# Shared library (MinGW auto-exports every symbol, plus an import library)
$(DLL_DEBUG): $(MY_OBJ_DEBUG) | $(BIN_DIR_DEBUG)
	$(CXX) -shared $(LDFLAGS) $(MY_OBJ_DEBUG) -o $@ \
		-Wl,--out-implib,$(BIN_DIR_DEBUG)/libgbemo.dll.a

$(DLL_RELEASE): $(MY_OBJ_RELEASE) | $(BIN_DIR_RELEASE)
	$(CXX) -shared $(LDFLAGS) $(MY_OBJ_RELEASE) -o $@ \
		-Wl,--out-implib,$(BIN_DIR_RELEASE)/libgbemo.dll.a

# The object tree mirrors src/, so each recipe creates its own subdirectory.
$(OBJ_DIR_DEBUG)/%.o: src/%.cpp
	@$(MKDIR) $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR_RELEASE)/%.o: src/%.cpp
	@$(MKDIR) $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR_DEBUG):
	@$(MKDIR) $(BIN_DIR_DEBUG)

$(BIN_DIR_RELEASE):
	@$(MKDIR) $(BIN_DIR_RELEASE)

clean:
	@$(RM) obj
	@$(RM) bin
	@$(RM) $(DIST_ROOT)

# Release package: headers + optimized static library for one architecture.
# Override VERSION to name it, e.g. "make VERSION=1.0.0 x64 dist".
dist: release
	@$(RM) $(DIST_DIR)
	@$(MKDIR) $(DIST_DIR)/include $(DIST_DIR)/lib
	@$(CP) $(PUBLIC_HEADERS) $(DIST_DIR)/include/
	@$(CP) $(LIB_RELEASE) $(DIST_DIR)/lib/
	@$(CP) README.md $(DIST_DIR)/
ifeq ($(SHARED),1)
	@$(MKDIR) $(DIST_DIR)/bin
	@$(CP) $(DLL_RELEASE) $(DIST_DIR)/bin/
	@$(CP) $(BIN_DIR_RELEASE)/libgbemo.dll.a $(DIST_DIR)/lib/
endif
	@cd $(DIST_ROOT) && $(RM) $(DIST_NAME).zip && $(ZIP) -qr $(DIST_NAME).zip $(DIST_NAME)
	@echo "packaged $(DIST_ROOT)/$(DIST_NAME).zip"

ifeq ($(GOALS),)
# "make x86" on its own still means "build the default target for x86".
$(SELECTORS): all
else
$(SELECTORS):
	@:
endif
