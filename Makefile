# Project: atty
# Makefile created by Steve Chang
# Date modified: 2024.11.30

PROJDIR = $(CURDIR)
SRCDIR 	= $(PROJDIR)/src
LIBDIR	= $(PROJDIR)/lib
BINDIR 	= $(PROJDIR)/bin
APPNAME = atty

SUBDIR = \
	src \
	src/serial \

COMMON_INCLUDE = \
	$(CURDIR)/src/include \

ifeq ($(OS),Windows_NT)
    OSFLAG += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        OSFLAG += -D AMD64
		BINNAME = $(APPNAME).exe

		EXTERN_LIBDIR = \
			"C:/MinGW/lib" \

		EXTERN_INCLUDE = \
			"C:/MinGW/include" \

    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            OSFLAG += -D AMD64
			BINNAME = $(APPNAME).exe

			EXTERN_LIBDIR = \
				"C:/MinGW/lib" \

			EXTERN_INCLUDE = \
				"C:/MinGW/include" \

        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            OSFLAG += -D IA32
			BINNAME = $(APPNAME).exe

			EXTERN_LIBDIR = \
				"C:/MinGW/lib" \

			EXTERN_INCLUDE = \
				"C:/MinGW/include" \

        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OSFLAG += -D LINUX
		BINNAME = $(APPNAME)

		EXTERN_LIBDIR = \
			"/usr/lib" \

		EXTERN_INCLUDE = \
			"/usr/include" \

    endif
    ifeq ($(UNAME_S),Darwin)
        OSFLAG += -D OSX
		BINNAME = $(APPNAME)

		EXTERN_LIBDIR = \
			"/usr/lib" \

		EXTERN_INCLUDE = \
			"/usr/include" \

    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        OSFLAG += -D AMD64
		BINNAME = $(APPNAME)

		EXTERN_LIBDIR = \
			"/usr/lib" \

		EXTERN_INCLUDE = \
			"/usr/include" \

    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        OSFLAG += -D IA32
		BINNAME = $(APPNAME)

		EXTERN_LIBDIR = \
			"/usr/lib" \

		EXTERN_INCLUDE = \
			"/usr/include" \

    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        OSFLAG += -D ARM
		BINNAME = $(APPNAME)

		EXTERN_LIBDIR = \
			"/usr/lib" \

		EXTERN_INCLUDE = \
			"/usr/include" \

    endif
endif

CC = gcc
AR = ar
LD = ld

MAKE_RULES := $(PROJDIR)/make/make_rules.mk

# Extra flags to give to compilers when they are supposed to invoke the linker,
# ‘ld’, such as -L. Libraries (-lfoo) should be added to the LDLIBS variable
# instead.
LDFLAGS = \
	$(addprefix -L,$(EXTERN_LIBDIR)) \
	$(addprefix -L,$(LIBDIR)) \
	# -v \
	# -static \
	# -static-libgcc \

# Library flags or names given to compilers when they are supposed to invoke
# the linker, ‘ld’. LOADLIBES is a deprecated (but still supported) alternative to
# LDLIBS. Non-library linker flags, such as -L, should go in the LDFLAGS variable.
LIBS = \
	main \
	serial \

LDLIBS = $(foreach lib,$(LIBS),-l$(lib)) -lpthread	# <-- Do not change this order.

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

.PHONY: all
all:
	@echo $(OSFLAG)
	mkdir -p $(BINDIR)
	mkdir -p $(LIBDIR)
	for dir in $(SUBDIR); do \
		cd $$dir; \
		# make -j $@; \
		make $@; \
		cd $(CURDIR); \
	done
	$(CC) $(LDFLAGS) $(LDLIBS) -o $(BINDIR)/$(BINNAME)

.PHONY: clean
clean:
	rm -f $(BINDIR)/$(BINNAME)
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
	$(CURDIR)/Astyle/bin/astyle.exe --options=./_astylerc -R ./*.c,*.h --exclude=AStyle --formatted