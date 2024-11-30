# Project: atty
# Makefile created by Steve Chang
# Date modified: 2024.11.30
# MAKEFLAGS += --trace
GCC_PATH := $(shell which gcc)
$(info [make] Using GCC from: $(GCC_PATH))
# $(info [make] MSYSTEM: $(MSYSTEM))
# $(info [make] GCC: $(shell pacman -Qe | grep gcc))
# $(info [make] MSYS2: $(shell pacman -Qe | grep msys))
# $(info [make] GCC Version: $(shell pacman -Qe | grep gcc | awk '{print $$2}'))
# $(info [make] MSYS2 Version: $(shell pacman -Qe | grep msys | awk '{print $$2}'))

PROJDIR = $(CURDIR)
SRCDIR 	= $(PROJDIR)/src
LIBDIR	= $(PROJDIR)/lib
BINDIR 	= $(PROJDIR)/bin
DLLDIR  = $(PROJDIR)/dll

SUBDIR = \
	src \
	src/serial \

COMMON_INCLUDE = \
	$(CURDIR)/include \

_BINNAME = atty

ifeq ($(OS),Windows_NT)
    OSFLAG += -DWIN32
    BINNAME = $(_BINNAME).exe
	DLL_FILE_EXT += dll
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        OSFLAG += -DAMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            OSFLAG += -DAMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            OSFLAG += -DIA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OSFLAG += -DLINUX
		BINNAME = $(_BINNAME)
		DLL_FILE_EXT += so
    endif
    ifeq ($(UNAME_S),Darwin)
        OSFLAG += -DOSX
		BINNAME = $(_BINNAME)
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        OSFLAG += -DAMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        OSFLAG += -DIA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        OSFLAG += -DARM
    endif
    UNAME_M := $(shell uname -m)
    ifneq ($(filter aarch64%,$(UNAME_M)),)
        OSFLAG += -DARM
    endif
endif

CC = gcc
# CC = g++
AR = ar
LD = ld

MAKE_RULES := $(PROJDIR)/make/make_rules.mk

# Extra flags to give to compilers when they are supposed to invoke the linker,
# `ld`, such as -L. Libraries (-lfoo) should be added to the LDLIBS variable
# instead.
LDFLAGS = \
	$(addprefix -L,$(EXTERN_LIBDIR)) \
	$(addprefix -L,$(LIBDIR)) \
	# -v \
	# -static \
	# -static-libgcc \

# Library flags or names given to compilers when they are supposed to invoke
# the linker, `ld`. LOADLIBES is a deprecated (but still supported) alternative to
# LDLIBS. Non-library linker flags, such as -L, should go in the LDFLAGS variable.
LIBS = \
	main \
	serial \

LDLIBS = $(foreach lib,$(LIBS),-l$(lib)) -lm # -lpthread	# <-- Do not change this order.

ifeq ($(CC),gcc)
C_FILE_EXT   = c
CPP_FILE_EXT = cpp
else
C_FILE_EXT   = cpp
CPP_FILE_EXT = cpp
endif

export BINNAME
export PROJDIR
export SRCDIR
export LIBDIR
export BINDIR
export COMMON_INCLUDE
export EXTERN_LIBDIR
export EXTERN_INCLUDE
export CC AR
export MAKE_RULES
export OSFLAG
export C_FILE_EXT

.PHONY: all
all:
	mkdir -p $(BINDIR)
	mkdir -p $(LIBDIR)
	#cp $(DLLDIR)/*.$(DLL_FILE_EXT) $(BINDIR)
	@echo $(OSFLAG)
	for dir in $(SUBDIR); do \
		cd $$dir; \
		# make -j $@; \
		make $@; \
		cd $(CURDIR); \
	done
	$(CC) $(LDFLAGS) $(LDLIBS) -o $(BINDIR)/$(BINNAME)

.PHONY: clean
clean:
	rm -f $(BINDIR)/*
	rm -f $(LIBDIR)/*
	for dir in $(SUBDIR); do \
		cd $$dir; \
		make -j $@; \
		cd $(CURDIR); \
	done

.PHONY: objall
objall:
	for dir in $(SUBDIR); do \
		cd $$dir; \
		make -j $@; \
		cd $(CURDIR); \
	done

.PHONY: objclean
objclean:
	for dir in $(SUBDIR); do \
		cd $$dir; \
		make -j $@; \
		cd $(CURDIR); \
	done

.PHONY: asmall
asmall:
	for dir in $(SUBDIR); do \
		cd $$dir; \
		make -j $@; \
		cd $(CURDIR); \
	done

.PHONY: asmclean
asmclean:
	for dir in $(SUBDIR); do \
		cd $$dir; \
		make -j $@; \
		cd $(CURDIR); \
	done

.PHONY: depall
depall:
	for dir in $(SUBDIR); do \
		cd $$dir; \
		make -j $@; \
		cd $(CURDIR); \
	done

.PHONY: depclean
depclean:
	for dir in $(SUBDIR); do \
		cd $$dir; \
		make -j $@; \
		cd $(CURDIR); \
	done

.PHONY: format
format:
	# $(CURDIR)/Astyle/bin/astyle.exe --options=$(CURDIR)/_astylerc -I -R ./*.$(C_FILE_EXT),*.$(CPP_FILE_EXT),*.h --exclude=AStyle --formatted
	$(CURDIR)/Astyle/bin/astyle.exe --options=$(CURDIR)/AStyle/file/linux.ini -I -R ./*.$(C_FILE_EXT),*.$(CPP_FILE_EXT),*.h --exclude=AStyle --formatted
	find $(CURDIR) -type f -exec dos2unix {} \;
	find . -name '*.orig' -exec rm -f {} +;

