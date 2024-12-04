# Project: atty
# Makefile created by Steve Chang
# Date modified: 2024.11.30

OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))
ASMS = $(addprefix $(ASMDIR)/,$(SRCS:.c=.s))
DEPS = $(addprefix $(OBJDIR)/,$(SRCS:.c=.c.d))

# C_INCDIRS = $(foreach dir,$(MODULE_INCLUDES),$(PROJDIR)/$(dir))

CFLAGS = \
	$(OSFLAG) \
	$(addprefix -I,$(COMMON_INCLUDE)) \
	$(addprefix -I,$(EXTERN_INCLUDE)) \
	$(foreach include, . $(INCLUDE), -I$(SRCDIR)/$(include)) \
	-g -O2 -Wall
# -v
# -g -Wall


# DEFINES +=-DXXXX
DEFINES +=

# # Extra flags to give to compilers when they are supposed to invoke the linker,
# # ‘ld’, such as -L. Libraries (-lfoo) should be added to the LDLIBS variable
# # instead.
# LDFLAGS = \
# 	-static \
# 	-static-libgcc

# # Library flags or names given to compilers when they are supposed to invoke
# # the linker, ‘ld’. LOADLIBES is a deprecated (but still supported) alternative to
# # LDLIBS. Non-library linker flags, such as -L, should go in the LDFLAGS variable.
# LDLIBS = \
# 	-lpthread

#
ARFLAGS = \
	rcs \
	# rcsv

###
.PHONY: all
all: $(LIBDIR)/$(LIBNAME)

$(LIBDIR)/$(LIBNAME): $(ASMS) $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

###
# all: $(BINDIR)/$(BINNAME)

# $(BINDIR)/$(BINNAME): $(OBJS)
# 	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

###
.PHONY: objall
objall: $(OBJS)

$(OBJS): | $(OBJDIR)

$(OBJDIR)/%.o : $(ASMDIR)/%.s
	$(CC) $(DEFINES) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $@

###
.PHONY: asmall
asmall: $(ASMS)

$(ASMS): | $(ASMDIR)

$(ASMDIR)/%.s : %.c
	$(CC) $(DEFINES) $(CFLAGS) -c $< -S -o $@

$(ASMDIR):
	mkdir $@

###
.PHONY: depall
depall: | $(OBJDIR)
	$(CC) $(DEFINES) $(CFLAGS) -M $(SRCS) > $(OBJDIR)/depend.d
	sed -i 's|\(.*\.o:\)|$(OBJDIR)/\1|g' $(OBJDIR)/depend.d

###
.PHONY: clean
clean:
	rm -rf $(OBJDIR)
	rm -rf $(ASMDIR)

.PHONY: objclean
objclean:
	rm -f $(OBJDIR)/*.o

.PHONY: asmclean
asmclean:
	rm -f $(ASMDIR)/*.s
.PHONY: depclean
depclean:
	rm -f $(OBJDIR)/*.d

ifeq ($(MAKECMDGOALS),all)
ifneq ($(wildcard $(OBJDIR)/depend.d),)
include $(OBJDIR)/depend.d
endif
endif
