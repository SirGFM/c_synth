
CC = gcc
.SUFFIXES=.c .o

#===============================================================================
# Define compilation target
#===============================================================================
  TARGET = synth
#===============================================================================

#===============================================================================
# Define CFLAGS (compiler flags)
#===============================================================================
# Add all warnings and default include path
  CFLAGS = -Wall -I"./include"
# Add architecture flah
  ARCH=$(shell uname -m)
  ifeq ($(ARCH), x86_64)
    CFLAGS += -m64
  else
    CFLAGS += -m32
  endif
# Add debug flags
  ifneq ($(RELEASE), yes)
    CFLAGS += -O0 -g
  endif
#===============================================================================

#===============================================================================
# Define LFLAGS (linker flags)
#===============================================================================
  LFLAGS = 
  SDLLFLAGS = -lm -lSDL2main -lSDL2
#===============================================================================

#===============================================================================
# Define where source files can be found and where objects and binary are output
#===============================================================================
  VPATH = src:tst
  OBJDIR = obj
  BINDIR = bin
#===============================================================================

#===============================================================================
# Define every object required by compilation
#===============================================================================
  OBJS = $(OBJDIR)/synth_sdl2_backend.o
#===============================================================================

#===============================================================================
# Define default compilation rule
#===============================================================================
all: MKDIRS $(BINDIR)/$(TARGET).a tests
#===============================================================================

$(BINDIR)/$(TARGET).a: $(OBJS)
	rm -f $(BINDIR)/$(TARGET).a
	ar -cvq $(BINDIR)/$(TARGET).a $(OBJS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

tests: $(BINDIR)/play_hardcoded_buffer

$(BINDIR)/play_hardcoded_buffer: $(OBJDIR)/play_hardcoded_buffer.o $(BINDIR)/$(TARGET).a
	$(CC) $(CFLAGS) -o $(BINDIR)/play_hardcoded_buffer $(OBJDIR)/play_hardcoded_buffer.o $(BINDIR)/$(TARGET).a $(LFLAGS) $(SDLLFLAGS)

MKDIRS: | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

clean:
	rm -f $(OBJS) $(BINDIR)/$(TARGET).a
	rm -rf $(OBJDIR) $(BINDIR)

