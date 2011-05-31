PROJECT_NAME	:= libk
VERSION		:= $(shell ./version)
UNAMEEXISTS	:= $(shell uname > /dev/null 2>&1; echo $$?)
PWDEXISTS	:= $(shell pwd > /dev/null 2>&1; echo $$?)
GCCEXISTS	:= $(shell gcc --version > /dev/null 2>&1; echo $$?)
CLANGEXISTS	:= $(shell clang --version > /dev/null 2>&1; echo $$?)
#ICCEXISTS	:= $(shell icc --version > /dev/null 2>&1; echo $$?)
GITEXISTS	:= $(shell git --version > /dev/null 2>&1; echo $$?)
TAREXISTS	:= $(shell tar --version > /dev/null 2>&1; echo $$?)
BZIP2EXISTS	:= $(shell bzip2 --help > /dev/null 2>&1; echo $$?)

ifeq ($(VERSION),)
$(error can't determine version string)
endif
ifneq ($(PWDEXISTS), 0)
$(error command 'pwd' not found)
endif
ifneq ($(UNAMEEXISTS), 0)
$(error command 'uname' not found)
endif
ifneq ($(GCCEXISTS), 0)
ifneq ($(CLANGEXISTS), 0)
ifneq ($(ICCEXISTS), 0)
$(error neither 'gcc', 'icc' nor 'clang' found)
endif
endif
endif

PLATFORM	:= $(shell uname)
PWD		:= $(shell pwd)

GCC_MAJOR	:= 0
GCC_MINOR	:= 0
ifeq ($(CONF), debug)
	DEBUG		:= Yes
endif
ifeq ($(CONF), release)
	RELEASE		:= Yes
endif
ifeq ($(CLANGEXISTS), 0)
	HAVE_CLANG	:= Yes
endif
ifeq ($(GCCEXISTS), 0)
	HAVE_GCC	:= Yes
ifneq ($(PLATFORM), MINGW32_NT-6.1)
# TODO: write shellscript in order to get detailed compiler version information
# and advanced testing possibilities (i.e. greater/less than, not just equality)
	GCC_MAJOR	:= $(shell gcc --version 2>&1 | head -n 1 | \
		cut -d' ' -f3 | cut -d'.' -f1)
	GCC_MINOR	:= $(shell gcc --version 2>&1 | head -n 1 | \
		cut -d' ' -f3 | cut -d'.' -f1)
endif
endif
ifeq ($(ICCEXISTS), 0)
	HAVE_ICC	:= Yes
endif
ifndef VERBOSE
	VERB		:= -s
endif

ifeq ($(PLATFORM), Linux)
	PLAT_LINUX	:= Yes
	PLATFORM	:= LINUX
	SO_EXT		:= so
else ifeq ($(PLATFORM), OpenBSD)
	PLAT_OPENBSD	:= Yes
	PLATFORM	:= OPENBSD
	SO_EXT		:= so
else ifeq ($(PLATFORM), FreeBSD)
	PLAT_FREEBSD	:= Yes
	PLATFORM	:= FREEBSD
	SO_EXT		:= so
else ifeq ($(PLATFORM), Darwin)
	PLAT_DARWIN	:= Yes
	PLATFORM	:= DARWIN
	SO_EXT		:= dylib
else ifeq ($(PLATFORM), MINGW32_NT-6.1)
	PLAT_WINNT	:= Yes
	PLATFORM	:= WINNT
	SO_EXT		:= dll
else
$(error unsupported platform: $(PLATFORM))
endif

################################################################################

LIBRARIES	+= -lpthread

INCLUDES	+= -I./src
INCLUDES	+= -I./include

SRC		+= ./src/version.c
.PHONY: ./src/version.c

SRC		+= ./src/blockcipher/blockcipher.c
SRC		+= ./src/blockcipher/modes/noop.c
SRC		+= ./src/blockcipher/modes/ecb.c
SRC		+= ./src/blockcipher/modes/cbc.c
SRC		+= ./src/blockcipher/modes/cfb.c
SRC		+= ./src/blockcipher/modes/ofb.c
SRC		+= ./src/blockcipher/modes/ctr.c
SRC		+= ./src/blockcipher/modes/tests.c
SRC		+= ./src/blockcipher/ciphers/threefish/threefish256.c
SRC		+= ./src/blockcipher/ciphers/threefish/threefish512.c
SRC		+= ./src/blockcipher/ciphers/threefish/threefish1024.c
SRC		+= ./src/blockcipher/ciphers/aes/aes.c

SRC		+= ./src/streamcipher/streamcipher.c
SRC		+= ./src/streamcipher/arc4.c

SRC		+= ./src/prng/prng.c
SRC		+= ./src/prng/platform.c
SRC		+= ./src/prng/mt19937.c

SRC		+= ./src/hash/hash.c
SRC		+= ./src/hash/skein/skein_common.c
SRC		+= ./src/hash/skein/skein256.c
SRC		+= ./src/hash/skein/skein512.c
SRC		+= ./src/hash/skein/skein1024.c
SRC		+= ./src/hash/sha1/sha1.c

SRC		+= ./src/kderive/simple.c

SRC		+= ./src/utils/pres.c
SRC		+= ./src/utils/tfile.c
SRC		+= ./src/utils/mem.c
SRC		+= ./src/utils/err.c
SRC		+= ./src/utils/workbench.c
SRC		+= ./src/utils/unittest.c
SRC		+= ./src/utils/mempool.c
SRC		+= ./src/utils/ioutils.c

ifdef PLAT_WINNT
SRC		+= ./src/utils/ntwrap.c
endif

SRCBIN		+= ./src/ktool/main.c

################################################################################


# preprocessor definitions
ifdef RELEASE
DEFINES		+= -DNDEBUG
endif
DEFINES		+= -D_GNU_SOURCE=1
ifndef PLAT_DARWIN
DEFINES		+= -D_POSIX_C_SOURCE=200809L
endif
ifdef PLAT_WINNT
DEFINES		+= -D_CRT_RAND_S=1
endif
DEFINES		+= -D_BSD_SOURCE=1
DEFINES		+= -D_FILE_OFFSET_BITS=64
DEFINES		+= -D_LARGEFILE64_SOURCE=1
DEFINES		+= -D_LARGEFILE_SOURCE=1
DEFINES		+= -D_REENTRANT=1
DEFINES		+= -D__$(PLATFORM)__=1
DEFINES		+= -DVERSION='"$(VERSION)"'
DEFINES		+= -D__LIBRARY_BUILD=1

# toolchain configuration

OUTDIR		:= ./build
BUILDDIR	:= $(OUTDIR)/$(TOOLCHAIN)_$(CONF)

# common flags
CFLAGS		:= -Wall

ifeq ($(TOOLCHAIN), gcc)
ifeq ($(GCC_MAJOR), 4)
CFLAGS		+= -fvisibility=hidden
endif
else
CFLAGS		+= -fvisibility=hidden
endif

ifdef PLAT_DARWIN
MACARCHS	:= -arch x86_64
CFLAGS		+= -mmacosx-version-min=10.5
endif
ifdef M32
ifdef PLAT_DARWIN
MACARCHS	:= -arch i386
endif
CFLAGS		+= -m32
endif

ifdef DEBUG
CFLAGS		+= -O0 -g
endif

ifdef RELEASE
CFLAGS		+= -O3
ifdef PLAT_WINNT
CFLAGS		+= -flto -fwhole-program
else

ifeq ($(GCC_MAJOR), 4)
ifeq ($(GCC_MAJOR), 5)
CFLAGS		+= -flto -fuse-linker-plugin
endif
endif

endif
endif #RELEASE
CXXFLAGS	:= $(CFLAGS)

# language dependent flags
ifneq ($(TOOLCHAIN), clang)
CFLAGS		+= -std=c99
endif
ifdef RELEASE
CXXFLAGS	+= -fvisibility-inlines-hidden
endif

LDFLAGS		:= $(CFLAGS)
ifdef PLAT_LINUX
#LDFLAGS		+= -static-libgcc
endif
ifdef PLAT_DARWIN
ARFLAGS		:= -static -o
else
ARFLAGS		:= cru
STRIPFLAGS	:= -s
endif


# determine intermediate object filenames
C_SRC		:= $(filter %.c, $(SRC))
CXX_SRC		:= $(filter %.cpp, $(SRC))
ASM_SRC		:= $(filter %.asm, $(SRC))

DEPS		:= $(patsubst %.c, $(BUILDDIR)/.obj/%_C.dep, $(C_SRC))
DEPS		+= $(patsubst %.cpp, $(BUILDDIR)/.obj/%_CXX.dep, $(CXX_SRC))
DEPS		+= $(patsubst %.c, $(BUILDDIR)/.obj/%_C_PIC.dep, $(C_SRC))
DEPS		+= $(patsubst %.cpp, $(BUILDDIR)/.obj/%_CXX_PIC.dep, $(CXX_SRC))

OBJECTS_ASM	:= $(patsubst %.asm, $(BUILDDIR)/.obj/%_ASM.o, $(ASM_SRC))

OBJECTS		:= $(patsubst %.c, $(BUILDDIR)/.obj/%_C.o, $(C_SRC))
OBJECTS		+= $(patsubst %.cpp, $(BUILDDIR)/.obj/%_CXX.o, $(CXX_SRC))

OBJECTS_PIC	:= $(patsubst %.c, $(BUILDDIR)/.obj/%_C_PIC.o, $(C_SRC))
OBJECTS_PIC	+= $(patsubst %.cpp, $(BUILDDIR)/.obj/%_CXX_PIC.o, $(CXX_SRC))

OBJECTS_BIN	:= $(patsubst %.c, $(BUILDDIR)/.obj/%_C.o, $(SRCBIN))

# tools
STRIP		:=  $(CROSS)strip
ifeq ($(TOOLCHAIN), gcc)
	CC		:= $(CROSS)gcc
	CXX		:= $(CROSS)g++
	ifeq ($(CXX_SRC),)
		LD	:= $(CROSS)gcc
	else
		LD	:= $(CROSS)g++
	endif
endif
ifeq ($(TOOLCHAIN), clang)
	CC		:= clang
	CXX		:= clang++
	ifeq ($(CXX_SRC),)
		LD	:= clang
	else
		LD	:= clang++
	endif
endif
ifeq ($(TOOLCHAIN), icc)
	CC		:= icc -ipo -no-prec-div -static-intel -wd,1338
	CXX		:= icc -ipo -no-prec-div -static-intel -wd,1338
	LD		:= icc -ipo -no-prec-div -static-intel \
				-wd,1338,11021,11000,11001,11006
endif

ifdef PLAT_DARWIN
	AR		:= libtool
else
	AR		:= $(CROSS)ar
endif


print_ar	:= echo $(eflags) "AR   "
print_tar	:= echo $(eflags) "TAR  "
print_ld	:= echo $(eflags) "LD   "
print_as	:= echo $(eflags) "ASM  "
print_cc	:= echo $(eflags) "CC   "
print_cxx	:= echo $(eflags) "CXX  "
print_strip	:= echo $(eflags) "STRIP"


# targets
all: release

help:
	@echo "following make targets are available:"
	@echo "  help        - print this"
	@echo "  release     - build release version of $(PROJECT_NAME) (*)"
	@echo "  debug       - build debug version of $(PROJECT_NAME)"
	@echo "  clean       - recursively delete the output directory" \
		"'$(OUTDIR)'"
	@echo ""
	@echo "(*) denotes the default target if none or 'all' is specified"
debug:
	@$(MAKE) CONF=debug $(VERB) -C . all-recursive
release:
	@$(MAKE) CONF=release $(VERB) -C . all-recursive
Release: release
Debug: debug

clean:
	@echo "deleting '$(OUTDIR)'"
	@-rm -rf $(OUTDIR)

all-recursive:
ifdef HAVE_GCC
	$(MAKE) $(VERB) -C . TOOLCHAIN=gcc final-all-recursive
endif
ifdef HAVE_CLANG
	$(MAKE) $(VERB) -C . TOOLCHAIN=clang final-all-recursive
endif
ifdef HAVE_ICC
	$(MAKE) $(VERB) -C . TOOLCHAIN=icc final-all-recursive
endif

ifdef PLAT_WINNT
final-all-recursive:					\
	$(BUILDDIR)/$(PROJECT_NAME).$(SO_EXT)		\
	$(BUILDDIR)/$(PROJECT_NAME).a			\
	$(BUILDDIR)/sktool				\
	$(BUILDDIR)/ktool
else
final-all-recursive:					\
	$(BUILDDIR)/$(PROJECT_NAME).$(SO_EXT)		\
	$(BUILDDIR)/$(PROJECT_NAME).a			\
	$(BUILDDIR)/$(PROJECT_NAME)_pic.a		\
	$(BUILDDIR)/sktool				\
	$(BUILDDIR)/ktool
endif

ifdef PLAT_WINNT
$(BUILDDIR)/$(PROJECT_NAME).$(SO_EXT): $(OBJECTS) $(OBJECTS_ASM)
else
$(BUILDDIR)/$(PROJECT_NAME).$(SO_EXT): $(OBJECTS_PIC) $(OBJECTS_ASM)
endif
	$(print_ld) $(subst $(PWD)/,./,$(abspath $(@)))
	@-mkdir -p $(dir $(@))
ifdef PLAT_DARWIN
	$(LD) -fPIC -dynamiclib						\
		-install_name "@rpath/$(notdir $(@))"			\
		-undefined dynamic_lookup $(MACARCHS) $(LDFLAGS)	\
		$(LPATH) $(FRAMEWORKS) -o $(@) $(^) $(LIBRARIES)
else ifdef PLAT_WINNT
	$(LD) -shared -Wl,--out-implib,$(subst .dll,.lib,$(@))		\
		$(LDFLAGS) $(LPATH) -o $(@) $(^) $(LIBRARIES)
else
	$(LD) -fPIC -shared -Wl,-soname,$(notdir $(@)),-z,combreloc	\
		$(LDFLAGS) $(LPATH) -o $(@) $(^) $(LIBRARIES)
endif

$(BUILDDIR)/ktool:	\
	$(OBJECTS_BIN) $(BUILDDIR)/$(PROJECT_NAME).$(SO_EXT)
	$(print_ld) $(subst $(PWD)/,./,$(abspath $(@)))
	@-mkdir -p $(dir $(@))
ifdef PLAT_DARWIN
	$(LD) -Wl,-rpath,"@loader_path/" $(MACARCHS) $(LDFLAGS) \
	$(LPATH) $(FRAMEWORKS) -o $(@) $(^) $(LIBRARIES)
else
	@export LD_RUN_PATH='$${ORIGIN}' && $(LD) $(MACARCHS) $(LDFLAGS) \
	$(LPATH) $(FRAMEWORKS) -o $(@) $(^) $(LIBRARIES)
endif

$(BUILDDIR)/sktool:	\
	$(OBJECTS_BIN) $(BUILDDIR)/$(PROJECT_NAME).a
	$(print_ld) $(subst $(PWD)/,./,$(abspath $(@)))
	@-mkdir -p $(dir $(@))
ifdef PLAT_DARWIN
	$(LD) -Wl,-rpath,"@loader_path/" $(MACARCHS) $(LDFLAGS) \
	$(LPATH) $(FRAMEWORKS) -o $(@) $(^) $(LIBRARIES)
else
	@export LD_RUN_PATH='$${ORIGIN}' && $(LD) $(MACARCHS) $(LDFLAGS) \
	$(LPATH) $(FRAMEWORKS) -o $(@) $(^) $(LIBRARIES)
endif


$(BUILDDIR)/.obj/$(PROJECT_NAME).ro: $(OBJECTS) $(OBJECTS_ASM)
	@$(print_ld) $(subst $(PWD)/,./,$(abspath $(@)))
	@-mkdir -p $(dir $(@))
	$(LD) -nostdlib -Wl,-r $(MACARCHS) $(LDFLAGS) \
	$(LPATH) $(FRAMEWORKS) -o $(@) $(^)

$(BUILDDIR)/.obj/$(PROJECT_NAME)_pic.ro: $(OBJECTS_PIC) $(OBJECTS_ASM)
	@$(print_ld) $(subst $(PWD)/,./,$(abspath $(@)))
	@-mkdir -p $(dir $(@))
	$(LD) -fPIC -nostdlib -Wl,-r $(MACARCHS) $(LDFLAGS) \
	$(LPATH) $(FRAMEWORKS) -o $(@) $(^)

$(BUILDDIR)/$(PROJECT_NAME).a: $(BUILDDIR)/.obj/$(PROJECT_NAME).ro
	@$(print_ar) $(subst $(PWD)/,./,$(abspath $(@)))
	@-mkdir -p $(dir $(@))
	@$(AR) $(ARFLAGS) $(@) $(^)

$(BUILDDIR)/$(PROJECT_NAME)_pic.a: $(BUILDDIR)/.obj/$(PROJECT_NAME)_pic.ro
	@$(print_ar) $(subst $(PWD)/,./,$(abspath $(@)))
	@-mkdir -p $(dir $(@))
	@$(AR) $(ARFLAGS) $(@) $(^)

$(BUILDDIR)/.obj/%_ASM.o: %.asm
	$(print_as) $(subst $(PWD)/,./,$(abspath $(<)))
	-mkdir -p $(dir $(@))
ifdef PLAT_WINNT
	yasm -f win64 -m amd64 -DWIN_ABI -DINTEL_SHA1_SINGLEBLOCK -o $(@) $(<)
else ifdef PLAT_DARWIN
	yasm -f macho64 -m amd64 -DINTEL_SHA1_SINGLEBLOCK \
	-DLEADING_UNDERSCORE -o $(@) $(<)
else
	yasm -f elf64 -m amd64 -DINTEL_SHA1_SINGLEBLOCK -o $(@) $(<)
endif

$(BUILDDIR)/.obj/%_C.o: %.c
	$(print_cc) $(subst $(PWD)/,./,$(abspath $(<)))
	-mkdir -p $(dir $(@))
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -E -M -MT \
		"$(@) $(@:.o=.dep)" -o $(@:.o=.dep) $(<)
	$(CC) $(CFLAGS) $(MACARCHS) $(DEFINES) $(INCLUDES) -c -o $(@) $(<)

$(BUILDDIR)/.obj/%_C_PIC.o: %.c
	$(print_cc) $(subst $(PWD)/,./,$(abspath $(<)))
	-mkdir -p $(dir $(@))
	$(CC) $(CFLAGS) $(DEFINES) -DPIC $(INCLUDES) -E -M -MT \
		"$(@) $(@:.o=.dep)" -o $(@:.o=.dep) $(<)
	$(CC) -fPIC $(CFLAGS) -DPIC $(MACARCHS) $(DEFINES) \
		$(INCLUDES) -c -o $(@) $(<)

$(BUILDDIR)/.obj/%_CXX.o: %.cpp
	$(print_cxx) $(subst $(PWD)/,./,$(abspath $(<)))
	-mkdir -p $(dir $(@))
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -E -M -MT \
		"$(@) $(@:.o=.dep)" -o $(@:.o=.dep) $(<)
	$(CXX) $(CXXFLAGS) $(MACARCHS) $(DEFINES) $(INCLUDES) -c -o $(@) $(<)

$(BUILDDIR)/.obj/%_CXX_PIC.o: %.cpp
	$(print_cxx) $(subst $(PWD)/,./,$(abspath $(<)))
	-mkdir -p $(dir $(@))
	$(CXX) $(CXXFLAGS) $(DEFINES) -DPIC $(INCLUDES) -E -M -MT \
		"$(@) $(@:.o=.dep)" -o $(@:.o=.dep) $(<)
	$(CXX) -fPIC $(CXXFLAGS) $(MACARCHS) $(DEFINES) -DPIC \
		$(INCLUDES) -c -o $(@) $(<)

-include $(DEPS)
