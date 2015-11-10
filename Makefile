
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
         $(OBJDIR)/synth_prng.o      \
         $(OBJDIR)/synth_track.o     \
         $(OBJDIR)/synth_volume.o
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
  SDL_LFLAGS := -lSDL2
  
  ifeq ($(OS), Win)
    LFLAGS := $(LFLAGS) -lmingw32
    SDL_LFLAGS := -lSDL2main -lSDL2
  else
    LFLAGS := $(LFLAGS) -lm
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

#==============================================================================
# Get the number of cores for fun stuff
#==============================================================================
  ifeq ($(OS), Win)
   CORES := 1
  else
   CORES := $$(($(shell nproc) * 2))
  endif
#==============================================================================

#===============================================================================
# Define default compilation rule
#===============================================================================
all: static shared tests
#===============================================================================

#==============================================================================
# Define the release rule, to compile everything on RELEASE mode (it's done in
# quite an ugly way.... =/)
#==============================================================================
release: MKDIRS
	# Remove all old binaries
	make clean
	# Compile everything in release mode
	make RELEASE=yes fast
	# Remove all debug info from the binaries
	strip $(BINDIR)/$(TARGET).a
	strip $(BINDIR)/$(TARGET).$(MNV)
	# Delete all .o to recompile as debug
	rm -f $(OBJS)
	# Recompile the lib with debug info
	make DEBUG=yes fast
	date
#==============================================================================

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
# Rule for installing the library
#==============================================================================
ifeq ($(OS), Win)
  install: release
	# Create destiny directories
	mkdir -p /c/c_synth/lib/
	mkdir -p /c/c_synth/include/c_synth
	# Copy every shared lib (normal, optmized and debug)
	cp -f $(BINDIR)/$(TARGET)*.$(MNV) /c/c_synth/lib
	# -P = don't follow sym-link
	cp -fP $(BINDIR)/$(TARGET)*.$(MJV) /c/c_synth/lib
	cp -fP $(BINDIR)/$(TARGET)*.$(SO) /c/c_synth/lib
	# Copy the headers
	cp -rf ./include/c_synth/* /c/c_synth/include/c_synth
else
  install: release
	# Create destiny directories
	mkdir -p $(LIBPATH)/c_synth
	mkdir -p $(HEADERPATH)/c_synth
	# Copy every shared lib (normal, optmized and debug)
	cp -f $(BINDIR)/$(TARGET)*.$(MNV) $(LIBPATH)/c_synth
	# -P = don't follow sym-link
	cp -fP $(BINDIR)/$(TARGET)*.$(MJV) $(LIBPATH)/c_synth
	cp -fP $(BINDIR)/$(TARGET)*.$(SO) $(LIBPATH)/c_synth
	# Copy the static lib
	cp -f $(BINDIR)/$(TARGET)*.a $(LIBPATH)/c_synth
	# Copy the headers
	cp -rf ./include/c_synth/* $(HEADERPATH)/c_synth
	# Make the lib be automatically found
	echo "$(LIBPATH)/c_synth" > /etc/ld.so.conf.d/c_synth.conf
	# Update the paths
	ldconfig
endif
#==============================================================================

#==============================================================================
# Rule for uninstalling the library
#==============================================================================
ifeq ($(OS), Win)
  uninstall:
	# Remove the libraries
	rm -f /c/c_synth/lib/$(TARGET)_dbg.$(MNV)
	rm -f /c/c_synth/lib/$(TARGET)_dbg.$(MJV)
	rm -f /c/c_synth/lib/$(TARGET)_dbg.$(SO)
	rm -f /c/c_synth/lib/$(TARGET).$(MNV)
	rm -f /c/c_synth/lib/$(TARGET).$(MJV)
	rm -f /c/c_synth/lib/$(TARGET).$(SO)
	# Remove the headers
	rm -rf /c/c_synth/include/*
	# Remove its directories
	rmdir /c/c_synth/lib/
	rmdir /c/c_synth/include/
	rmdir /c/c_synth/
else
  uninstall:
	# Remove the libraries
	rm -f $(LIBPATH)/c_synth/$(TARGET)_dbg.$(MNV)
	rm -f $(LIBPATH)/c_synth/$(TARGET)_dbg.$(MJV)
	rm -f $(LIBPATH)/c_synth/$(TARGET)_dbg.$(SO)
	rm -f $(LIBPATH)/c_synth/$(TARGET)_dbg.a
	rm -f $(LIBPATH)/c_synth/$(TARGET).$(MNV)
	rm -f $(LIBPATH)/c_synth/$(TARGET).$(MJV)
	rm -f $(LIBPATH)/c_synth/$(TARGET).$(SO)
	rm -f $(LIBPATH)/c_synth/$(TARGET).a
	# Remove the headers
	rm -rf $(HEADERPATH)/c_synth/*
	# Remove its directories
	rmdir $(LIBPATH)/c_synth
	rmdir $(HEADERPATH)/c_synth
	# Remove the lib from the default path
	rm /etc/ld.so.conf.d/c_synth.conf
	# Update the paths
	ldconfig
endif
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
	$(CC) $(CFLAGS) -o $@ $< -L$(BINDIR) $(LFLAGS) -$(LIBNAME) $(SDL_LFLAGS)
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
# Build everything as fast as possible (and using as many cores/threads as
# possible)
#==============================================================================
fast:
	make -j $(CORES) static shared
#==============================================================================

#==============================================================================
# Build everything as fast as possible (and using as many cores/threads as
# possible)
#==============================================================================
fast_all:
	make -j $(CORES) static shared && make -j $(CORES)
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

