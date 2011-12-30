MAKEFILE_DIR	:= $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
PREFIX		:= $(MAKEFILE_DIR)local
PREFIX32	:= $(MAKEFILE_DIR)local32

ARCHITECTURE	:= $(shell uname -m)
PLATFORM	:= $(shell uname)

ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLAT_WINNT	:= Yes
endif

ifeq ($(PLATFORM), Linux)
	PLAT_LINUX	:= Yes
endif

ifeq ($(PLATFORM), Darwin)
	PLAT_DARWIN	:= Yes
endif

SAMPLESRC	:= 'int main(){return 0;}'

ifdef PLAT_WINNT
EXECFLAGS	:= -c -o /dev/null - > /dev/null 2>&1
else
EXECFLAGS	:= -o /dev/null - > /dev/null 2>&1
endif

HAVE_MULTILIB := $(shell \
	(echo $(SAMPLESRC) | $(CC) -m32 -xc $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CXX) -m32 -xc++ $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CC) -m64 -xc $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CXX) -m64 -xc++ $(EXECFLAGS)); \
	echo $$?)
ifeq ($(HAVE_MULTILIB), 0)
	MULTILIB	:= Yes
endif

$(ARCHIVE):
	@if test -f '$@'; then exit 0; else            			\
		trap 'rm -rf '$(notdir $(@)).lock' '$(@)' '$(@)'.tmp'	\
			1 2 13 15;					\
		if mkdir '$(notdir $(@)).lock' 2>/dev/null; then	\
			echo 'downloading '$(notdir $(@))'';		\
			mkdir -p '$(dir $(@))';				\
			curl -k -s -L '$(URL)' -o '$(@)'.tmp;		\
			result=$$?;					\
			if [ $$result = "0" ]; then			\
				mv '$(@)'.tmp '$(@)';			\
			else						\
				rm -f '$(@)'.tmp;			\
			fi;						\
			rm -rf '$(notdir $(@)).lock';			\
			exit $$result;					\
		else               					\
			echo ''$(notdir $(@))' is downloading';		\
			while test -d '$(notdir $(@)).lock';		\
				do sleep 1;				\
			done;						\
			test -f '$(@)'; exit $$?;			\
		fi;							\
	fi



#.NOTPARALLEL:
.PHONY: all libgmp libmpfr

all: libgmp libmpfr
#	@echo "done"

%/libgmp.a: $(MAKEFILE_DIR)Makefile.mpir
	@rm -f '$(@)'
	$(MAKE) -C '$(MAKEFILE_DIR)' -f '$(notdir $(<))' '$(@)'

ifeq ($(MULTILIB), Yes)
libgmp: $(PREFIX)/lib/libgmp.a $(PREFIX32)/lib/libgmp.a
else
libgmp: $(PREFIX)/lib/libgmp.a
endif


%/libmpfr.a: $(MAKEFILE_DIR)Makefile.mpfr %/libgmp.a
	@rm -f '$(@)'
	$(MAKE) -C '$(MAKEFILE_DIR)' -f '$(notdir $(<))' '$(@)'

ifeq ($(MULTILIB), Yes)
libmpfr: $(PREFIX)/lib/libmpfr.a $(PREFIX32)/lib/libmpfr.a
else
libmpfr: $(PREFIX)/lib/libmpfr.a
endif
