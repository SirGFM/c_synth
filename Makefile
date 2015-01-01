
CC = gcc
.SUFFIXES=.c .o

#===============================================================================
# Define compilation target
#===============================================================================
  TARGET = synth
#===============================================================================

#===============================================================================
# Set OS flag
#===============================================================================
  OS=$(shell uname)
  ifeq ($(OS), MINGW32_NT-6.1)
    OS=Win
  endif
#===============================================================================

#===============================================================================
# Define CFLAGS (compiler flags)
#===============================================================================
# Add all warnings and default include path
  CFLAGS = -Wall -I"./include" -I"./src/include"
# Add architecture flag
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
  ifeq ($(OS), Win)
    SDLLFLAGS := -lmingw32 $(SDLLFLAGS)
  endif
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
  OBJS = $(OBJDIR)/synth_lexer.o \
         $(OBJDIR)/synth_note.o \
         $(OBJDIR)/synth_sdl2_backend.o \
         $(OBJDIR)/synth_track.o \
         $(OBJDIR)/synth_volume.o
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

tests: $(BINDIR)/play_hardcoded_buffer $(BINDIR)/play_hc_track $(BINDIR)/play_hc_mario $(BINDIR)/parse_string

$(BINDIR)/parse_string: $(OBJDIR)/parse_string.o $(BINDIR)/$(TARGET).a
	$(CC) $(CFLAGS) -o $(BINDIR)/parse_string $(OBJDIR)/parse_string.o $(BINDIR)/$(TARGET).a $(LFLAGS) $(SDLLFLAGS)

$(BINDIR)/play_hardcoded_buffer: $(OBJDIR)/play_hardcoded_buffer.o $(BINDIR)/$(TARGET).a
	$(CC) $(CFLAGS) -o $(BINDIR)/play_hardcoded_buffer $(OBJDIR)/play_hardcoded_buffer.o $(BINDIR)/$(TARGET).a $(LFLAGS) $(SDLLFLAGS)

$(BINDIR)/play_hc_track: $(OBJDIR)/play_hc_track.o $(BINDIR)/$(TARGET).a
	$(CC) $(CFLAGS) -o $(BINDIR)/play_hc_track $(OBJDIR)/play_hc_track.o $(BINDIR)/$(TARGET).a $(LFLAGS) $(SDLLFLAGS)

$(BINDIR)/play_hc_mario: $(OBJDIR)/play_hc_mario.o $(BINDIR)/$(TARGET).a
	$(CC) $(CFLAGS) -o $(BINDIR)/play_hc_mario $(OBJDIR)/play_hc_mario.o $(BINDIR)/$(TARGET).a $(LFLAGS) $(SDLLFLAGS)

MKDIRS: | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

clean:
	rm -f $(OBJS) $(BINDIR)/$(TARGET).a $(BINDIR)/*
	rm -rf $(OBJDIR) $(BINDIR)

