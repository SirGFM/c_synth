
ifndef ($(CC))
  CC = gcc
endif
.SUFFIXES=.c .o

#==============================================================================
# Define compilation target
#==============================================================================
  TARGET := libCSynth
  LIBNAME := lCSynth
  MAJOR_VERSION := 1
  MINOR_VERSION := 0
  REV_VERSION   := 0
# If the DEBUG flag was set, generate another binary (so it doesn't collide
# with the release one)
  ifeq ($(DEBUG), yes)
    TARGET := $(TARGET)_dbg
  endif
#==============================================================================

#===============================================================================
# Define every object required by compilation
#===============================================================================
  OBJS = $(OBJDIR)/synth.o           \
         $(OBJDIR)/synth_audio.o     \
         $(OBJDIR)/synth_lexer.o     \
         $(OBJDIR)/synth_note.o      \
         $(OBJDIR)/synth_parser.o    \
         $(OBJDIR)/synth_track.o     \
         $(OBJDIR)/synth_volume.o
         #$(OBJDIR)/synth_audiolist.o \
         #$(OBJDIR)/synth_buffer.o    \
         #$(OBJDIR)/synth_cache.o     \
         #$(OBJDIR)/synth_prng.o      \
         #$(OBJDIR)/synth_thread.o    \
 
 #ifeq ($(USE_SDL), yes)
 #  OBJS += $(OBJS) $(OBJDIR)/synth_sdl2_backend.o
 #else
 #  OBJS += $(OBJS) $(OBJDIR)/synth_lpulse_backend.o
 #endif
#===============================================================================

#===============================================================================
# Set OS flag
#===============================================================================
  OS := $(shell uname)
  ifeq ($(OS), MINGW32_NT-6.1)
    OS := Win
  endif
  ifeq ($(CC), emcc)
    OS := emscript
  endif
#===============================================================================

#===============================================================================
# Define CFLAGS (compiler flags)
#===============================================================================
# Add all warnings and default include path
  CFLAGS := -Wall -I"./include" -I"./src/include"
# Add architecture flag
  ARCH := $(shell uname -m)
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -I"$(EMSCRIPTEN)/system/include/" -m32
  else
    ifeq ($(ARCH), x86_64)
      CFLAGS := $(CFLAGS) -m64
    else
      CFLAGS := $(CFLAGS) -m32
    endif
  endif
# Add debug flags
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -O2
  else
    ifneq ($(RELEASE), yes)
      CFLAGS := $(CFLAGS) -g -O0
    else
      CFLAGS := $(CFLAGS) -O3
    endif
    #ifeq ($(USE_SDL), yes)
    #  CFLAGS := $(CFLAGS) -DUSE_SDL
    #endif
  endif
# Set flags required by OS
  ifeq ($(OS), Win)
    CFLAGS := $(CFLAGS) -I"/d/windows/mingw/include"
  else
    CFLAGS := $(CFLAGS) -fPIC
  endif
# Set the current compiler
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -DEMCC
  endif
#===============================================================================

#===============================================================================
# Define LFLAGS (linker flags)
#===============================================================================
  LFLAGS := 
  #LFLAGS := -lpthread
  #ifeq ($(USE_SDL), yes)
  #  SDLLFLAGS := -lm -lSDL2
  #else
  #  SDLLFLAGS := -lm -lpulse-simple
  #endif
  
  ifeq ($(OS), Win)
    LFLAGS := $(LFLAGS) -lmingw32
    ifeq ($(USE_SDL), yes)
      SDLLFLAGS := -lmingw32 -lSDL2main $(SDLLFLAGS)
    else
      SDLLFLAGS := -lmingw32 $(SDLLFLAGS)
    endif
  endif
#===============================================================================

#===============================================================================
# Define where source files can be found and where objects and binary are output
#===============================================================================
  VPATH := src:tst
  OBJDIR := obj/$(OS)
  BINDIR := bin/$(OS)
  TESTDIR := tst
  ifeq ($(OS), Win)
     ifeq ($(ARCH), x64)
       LIBPATH := /d/windows/mingw/lib
     else
       LIBPATH := /d/windows/mingw/mingw32/lib
     endif
     HEADERPATH := /d/windows/mingw/include
  else
    LIBPATH := /usr/lib
    HEADERPATH := /usr/include
  endif
#===============================================================================

#==============================================================================
# Automatically look up for tests and compile them
#==============================================================================
 TEST_SRC := $(wildcard $(TESTDIR)/tst_*.c)
 TEST_OBJS := $(TEST_SRC:$(TESTDIR)/%.c=$(OBJDIR)/%.o)
 TEST_BIN := $(addprefix $(BINDIR)/, $(TEST_SRC:$(TESTDIR)/%.c=%$(BIN_EXT)))
#==============================================================================

#==============================================================================
# Make sure the test's object files aren't automatically deleted
#==============================================================================
.SECONDARY: $(TEST_OBJS)
#==============================================================================

#==============================================================================
# Make the objects list constant (and the icon, if any)
#==============================================================================
  OBJS := $(OBJS)
#==============================================================================

#==============================================================================
# Set shared library's extension
#==============================================================================
  ifeq ($(OS), Win)
    SO := dll
    MJV := $(SO)
    MNV := $(SO)
  else
    SO := so
    MJV := $(SO).$(MAJOR_VERSION)
    MNV := $(SO).$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)
  endif
#==============================================================================

#===============================================================================
# Define default compilation rule
#===============================================================================
all: static shared tests
#===============================================================================

#==============================================================================
# Rule for building the static lib
#==============================================================================
static: MKDIRS $(BINDIR)/$(TARGET).a
#==============================================================================

#==============================================================================
# Rule for building the shared libs
#==============================================================================
shared: MKDIRS $(BINDIR)/$(TARGET).$(MNV)
#==============================================================================

#==============================================================================
# Rule for building tests
#==============================================================================
tests: MKDIRS shared $(TEST_BIN)
#==============================================================================

#==============================================================================
# Rule for actually building the static library
#==============================================================================
$(BINDIR)/$(TARGET).a: $(OBJS)
	rm -f $(BINDIR)/$(TARGET).a
	ar -cvq $(BINDIR)/$(TARGET).a $(OBJS)
#==============================================================================

#==============================================================================
# Rule for actually building the shared library
#==============================================================================
ifeq ($(OS), Win)
  $(BINDIR)/$(TARGET).$(MNV): $(OBJS)
	rm -f $(BINDIR)/$(TARGET).$(MNV)
	gcc -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-all-symbols \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LFLAGS)
else
  $(BINDIR)/$(TARGET).$(MNV): $(OBJS)
	rm -f $(BINDIR)/$(TARGET).$(MNV) $(BINDIR)/$(TARGET).$(SO)
	gcc -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LFLAGS)
	cd $(BINDIR); ln -f -s $(TARGET).$(MNV) $(TARGET).$(MJV)
	cd $(BINDIR); ln -f -s $(TARGET).$(MJV) $(TARGET).$(SO)
endif
#==============================================================================

#==============================================================================
# Rule for compiling test binaries that uses SDL2 as its backend (those are
# prefixed by 'tst_' and suffixed by 'SDL2')
#==============================================================================
$(BINDIR)/tst_%SDL2$(BIN_EXT): $(OBJDIR)/tst_%SDL2.o
	$(CC) $(CFLAGS) -o $@ $< -L$(BINDIR) $(LFLAGS) -$(LIBNAME) -lSDL2
#==============================================================================

#==============================================================================
# Rule for compiling a test binary (it's prefixed by 'tst_')
#==============================================================================
$(BINDIR)/tst_%$(BIN_EXT): $(OBJDIR)/tst_%.o
	$(CC) $(CFLAGS) -o $@ $< -L$(BINDIR) $(LFLAGS) -$(LIBNAME)
#==============================================================================

#==============================================================================
# Rule for compiling any .c in its object
#==============================================================================
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
#==============================================================================

#==============================================================================
# Rule for creating every directory
#==============================================================================
MKDIRS: | $(OBJDIR)
#==============================================================================

#==============================================================================
# Rule for actually creating every directory
#==============================================================================
$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
#==============================================================================

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINDIR)/$(TARGET).a $(BINDIR)/*
	rm -rf $(OBJDIR) $(BINDIR)

