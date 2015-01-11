
CC := gcc
.SUFFIXES=.c .o

#===============================================================================
# Define compilation target
#===============================================================================
  TARGET := synth
#===============================================================================

#===============================================================================
# Set OS flag
#===============================================================================
  OS := $(shell uname)
  ifeq ($(OS), MINGW32_NT-6.1)
    OS := Win
  endif
#===============================================================================

#===============================================================================
# Define CFLAGS (compiler flags)
#===============================================================================
# Add all warnings and default include path
  CFLAGS := -Wall -I"./include" -I"./src/include"
# Add architecture flag
  ARCH := $(shell uname -m)
  ifeq ($(ARCH), x86_64)
    CFLAGS := $(CFLAGS) -m64
  else
    CFLAGS := $(CFLAGS) -m32
  endif
# Add debug flags
  ifneq ($(RELEASE), yes)
    CFLAGS := $(CFLAGS) -O0 -g
  endif
#===============================================================================

#===============================================================================
# Define LFLAGS (linker flags)
#===============================================================================
  LFLAGS := -lpthread
  SDLLFLAGS := -lm -lSDL2main -lSDL2
  ifeq ($(OS), Win)
    SDLLFLAGS := -lmingw32 $(SDLLFLAGS)
  endif
#===============================================================================

#===============================================================================
# Define where source files can be found and where objects and binary are output
#===============================================================================
  VPATH := src:tst
  OBJDIR := obj
  BINDIR := bin
#===============================================================================

#===============================================================================
# Define every object required by compilation
#===============================================================================
  OBJS := $(OBJDIR)/synth.o \
          $(OBJDIR)/synth_audio.o \
          $(OBJDIR)/synth_audiolist.o \
          $(OBJDIR)/synth_buffer.o \
          $(OBJDIR)/synth_cache.o \
          $(OBJDIR)/synth_lexer.o \
          $(OBJDIR)/synth_note.o \
          $(OBJDIR)/synth_parser.o \
          $(OBJDIR)/synth_prng.o \
          $(OBJDIR)/synth_sdl2_backend.o \
          $(OBJDIR)/synth_thread.o \
          $(OBJDIR)/synth_track.o \
          $(OBJDIR)/synth_volume.o
#===============================================================================

#===============================================================================
# Define default compilation rule
#===============================================================================
all: MKDIRS $(BINDIR)/$(TARGET).a tests
#===============================================================================

lib: MKDIRS $(BINDIR)/$(TARGET).a

$(BINDIR)/$(TARGET).a: $(OBJS)
	rm -f $(BINDIR)/$(TARGET).a
	ar -cvq $(BINDIR)/$(TARGET).a $(OBJS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

tests:  \
        $(BINDIR)/parse_string \
        $(BINDIR)/play_audio \
        $(BINDIR)/play_hc_mario \
        $(BINDIR)/play_hc_track \
        $(BINDIR)/test_loop \
        $(BINDIR)/tokenize_mml

$(BINDIR)/play_audio: MKDIRS $(OBJDIR)/play_audio.o $(BINDIR)/$(TARGET).a
	$(CC) $(CFLAGS) -o $(BINDIR)/play_audio $(OBJDIR)/play_audio.o $(BINDIR)/$(TARGET).a $(LFLAGS) $(SDLLFLAGS)

$(BINDIR)/tokenize_mml: MKDIRS $(OBJDIR)/tokenize_mml.o $(BINDIR)/$(TARGET).a
	$(CC) $(CFLAGS) -o $(BINDIR)/tokenize_mml $(OBJDIR)/tokenize_mml.o $(BINDIR)/$(TARGET).a $(LFLAGS) $(SDLLFLAGS)

MKDIRS: | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

clean:
	rm -f $(OBJS) $(BINDIR)/$(TARGET).a $(BINDIR)/*
	rm -rf $(OBJDIR) $(BINDIR)

