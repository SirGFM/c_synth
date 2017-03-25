#===============================================================================
# Override-able variables:
#
# CC -- Compiler
# AR -- Archiver
# OS -- Target operating system (in {Linux, Win, emscript})
# ARCH -- Target architecture (in {i686, x86_64})
# CFLAGS -- Base compiler flags (to which more flags are appended)
# PREFIX -- Base directory where the lib will be installed
# LIBPATH -- Directory where the shared library will be installed
# HEADERPATH -- Directory where the headers will be installed
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
# Cross-compiling instructions:
#
#===============================================================================

#===============================================================================
# Define every object required by compilation
#===============================================================================
  OBJS = $(OBJDIR)/synth.o          \
         $(OBJDIR)/synth_audio.o    \
         $(OBJDIR)/synth_lexer.o    \
         $(OBJDIR)/synth_note.o     \
         $(OBJDIR)/synth_parser.o   \
         $(OBJDIR)/synth_prng.o     \
         $(OBJDIR)/synth_renderer.o \
         $(OBJDIR)/synth_track.o    \
         $(OBJDIR)/synth_volume.o
#===============================================================================

#==============================================================================
# Select which compiler to use (either gcc or emcc)
#==============================================================================
  ifneq (, $(findstring $(MAKECMDGOALS), emscript))
    CC := emcc
  endif
  CC ?= gcc
  AR ?= ar
  STRIP ?= strip
# Set DEBUG as the default mode
  ifneq ($(RELEASE), yes)
    RELEASE := no
    DEBUG := yes
  endif
#==============================================================================

#==============================================================================
# Clear the suffixes' default rule, since there's an explicit one
#==============================================================================
.SUFFIXES:
#==============================================================================

#==============================================================================
# Define all targets that doesn't match its generated file
#==============================================================================
.PHONY: emscript fast fast_all install install_shared install_shared_win \
        install_shared_x install_static install_static_win install_static_x \
        uninstall uninstall_win uninstall_x clean emscript_clean distclean
#==============================================================================

#==============================================================================
# Define compilation target
#==============================================================================
  TARGET := libCSynth
  LIBNAME := lCSynth
  MAJOR_VERSION := 1
  MINOR_VERSION := 0
  REV_VERSION   := 2
# If the DEBUG flag was set, generate another binary (so it doesn't collide
# with the release one)
  ifeq ($(DEBUG), yes)
    TARGET := $(TARGET)_dbg
  endif
#==============================================================================

#===============================================================================
# Set OS flag
#===============================================================================
  UNAME := $(shell uname)
  OS ?= $(UNAME)
  ifneq (, $(findstring $(UNAME), Windows_NT))
    OS := Win
    UNAME := Win
  endif
  ifneq (, $(findstring $(UNAME), MINGW))
    OS := Win
    UNAME := Win
  endif
  ifneq (, $(findstring $(UNAME), MSYS))
    OS := Win
    UNAME := Win
  endif
  ifeq ($(CC), emcc)
    OS := emscript
  endif
#===============================================================================

#===============================================================================
# Define CFLAGS (compiler flags)
#===============================================================================
# Add all warnings and default include path
  CFLAGS := $(CFLAGS) -Wall -I"./include" -I"./src/include"
# Add architecture flag
  ARCH ?= $(shell uname -m)
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
  ifeq ($(UNAME), Win)
    CFLAGS := $(CFLAGS) -I"/d/windows/mingw/include"
  endif
  ifneq ($(OS), Win)
    CFLAGS := $(CFLAGS) -fPIC
  endif
# Set the current compiler
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -DEMCC
  endif
#===============================================================================

#===============================================================================
# Define LDFLAGS (linker flags)
#===============================================================================
  SDL_LDFLAGS := -lSDL2

  ifeq ($(OS), Win)
    LDFLAGS := $(LDFLAGS) -lmingw32
    SDL_LDFLAGS := -lSDL2main -lSDL2
  else
    LDFLAGS := $(LDFLAGS) -lm
  endif
#===============================================================================

#===============================================================================
# Define where source files can be found and where objects and binary are output
#===============================================================================
  VPATH := src:tst
  ifeq ($(RELEASE), yes)
    OBJDIR := obj/release/$(OS)
    BINDIR := bin/release/$(OS)
  else
    OBJDIR := obj/debug/$(OS)
    BINDIR := bin/debug/$(OS)
  endif
  TESTDIR := tst

  PREFIX ?= /usr
  LIBPATH ?= $(PREFIX)/lib/c_synth
  HEADERPATH ?= $(PREFIX)/include/c_synth
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
    SO ?= dll
    MJV ?= $(SO)
    MNV ?= $(SO)
  else
    SO ?= so
    MJV ?= $(SO).$(MAJOR_VERSION)
    MNV ?= $(SO).$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)
  endif
#==============================================================================

#==============================================================================
# Ensure debug build isn't stripped
#==============================================================================
  ifneq ($(RELEASE), yes)
    STRIP := touch
  endif
#==============================================================================

#==============================================================================
# Get the number of cores for fun stuff
#==============================================================================
  ifeq ($(UNAME), Win)
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
# Rule for building a object file for emscript
#==============================================================================
emscript: $(BINDIR)/$(TARGET).bc
#===============================================================================

#==============================================================================
# Rule for cleaning emscript build... It's required to modify the CC
#==============================================================================
emscript_clean: clean
#===============================================================================

#==============================================================================
# Build a emscript (LLVM) binary, to be used when compiling for HTML5
#==============================================================================
$(BINDIR)/$(TARGET).bc: MKDIRS $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)
#==============================================================================

#==============================================================================
# Rule for building the static lib
#==============================================================================
static: MKDIRS $(BINDIR)/$(TARGET).a
#==============================================================================

#==============================================================================
# Rule for building the shared libs
#==============================================================================
shared: MKDIRS $(BINDIR)/$(TARGET).$(SO)
#==============================================================================

#==============================================================================
# Rule for building tests
#==============================================================================
tests: MKDIRS shared $(TEST_BIN)
#==============================================================================

#==============================================================================
# Rule for installing the library
#==============================================================================
ifeq ($(UNAME), Win)
  install: install_shared_win install_static_win
  install_shared: install_shared_win
  install_static: install_static_win
else
  install: install_shared_x install_static_x
  install_shared: install_shared_x
  install_static: install_static_x
endif

install_shared_win: shared
	# Create destiny directories
	mkdir -p /c/c_synth/lib/
	mkdir -p /c/c_synth/include/c_synth
	# Copy the headers
	cp -rf ./include/c_synth/* /c/c_synth/include/c_synth
	# Copy the lib
	cp -rf $(BINDIR)/$(TARGET).dll /c/c_synth/lib/

install_static_win: static
	# Create destiny directories
	mkdir -p /c/c_synth/lib/
	mkdir -p /c/c_synth/include/c_synth
	# Copy the headers
	cp -rf ./include/c_synth/* /c/c_synth/include/c_synth
	# Copy the lib
	cp -rf $(BINDIR)/$(TARGET).a /c/c_synth/lib/

install_shared_x: shared
	# Create destiny directories
	mkdir -p $(LIBPATH)
	mkdir -p $(HEADERPATH)
	# Copy the headers
	cp -rf ./include/c_synth/* $(HEADERPATH)
	# Copy every shared lib (normal, optmized and debug)
	cp -f $(BINDIR)/$(TARGET)*.$(MNV) $(LIBPATH)
	# -P = don't follow sym-link
	cp -fP $(BINDIR)/$(TARGET)*.$(MJV) $(LIBPATH)
	cp -fP $(BINDIR)/$(TARGET)*.$(SO) $(LIBPATH)
	# Make the lib be automatically found
	echo "$(LIBPATH)" > /etc/ld.so.conf.d/c_synth.conf
	ldconfig

install_static_x: static
	# Create destiny directories
	mkdir -p $(LIBPATH)
	mkdir -p $(HEADERPATH)
	# Copy the headers
	cp -rf ./include/c_synth/* $(HEADERPATH)
	# Copy the static lib
	cp -f $(BINDIR)/$(TARGET)*.a $(LIBPATH)
#==============================================================================

#==============================================================================
# Rule for uninstalling the library
#==============================================================================
ifeq ($(UNAME), Win)
  uninstall: uninstall_win
else
  uninstall: uninstall_x
endif

uninstall_win:
	# Remove the libraries (account for different versions)
	rm -f /c/c_synth/lib/$(TARGET)_dbg.*
	rm -f /c/c_synth/lib/$(TARGET).*
	# Remove the headers
	rm -rf /c/c_synth/include/*
	# Remove its directories
	rmdir /c/c_synth/lib/
	rmdir /c/c_synth/include/
	rmdir /c/c_synth/

uninstall_x:
	# Remove the libraries (account for different versions)
	rm -f $(LIBPATH)/$(TARGET)_dbg.*
	rm -f $(LIBPATH)/$(TARGET).*
	# Remove the headers
	rm -rf $(HEADERPATH)/*
	# Remove its directories
	rmdir $(LIBPATH)
	rmdir $(HEADERPATH)
	# Remove the lib from the default path
	rm /etc/ld.so.conf.d/c_synth.conf
	# Update the paths
	ldconfig
#==============================================================================

#==============================================================================
# Rule for actually building the static library
#==============================================================================
$(BINDIR)/$(TARGET).a: $(OBJS)
	rm -f $(BINDIR)/$(TARGET).a
	$(AR) -cvq $(BINDIR)/$(TARGET).a $(OBJS)
#==============================================================================

#==============================================================================
# Rule for actually building the shared library
#==============================================================================
# Windows DLL
$(BINDIR)/$(TARGET).dll: $(OBJS)
	rm -f $@
	$(CC) -shared -Wl,-soname,$(TARGET).dll -Wl,-export-all-symbols $(CFLAGS) \
	    -o $@ $(OBJS) $(LDFLAGS)
	$(STRIP) $@

# Linux
$(BINDIR)/$(TARGET).so: $(BINDIR)/$(TARGET).$(MJV)
	rm -f $(BINDIR)/$(TARGET).$(SO)
	cd $(BINDIR); ln -f -s $(TARGET).$(MJV) $(TARGET).$(SO)

ifneq ($(SO), $(MJV))
$(BINDIR)/$(TARGET).$(MJV): $(BINDIR)/$(TARGET).$(MNV)
	rm -f $(BINDIR)/$(TARGET).$(MJV)
	cd $(BINDIR); ln -f -s $(TARGET).$(MNV) $(TARGET).$(MJV)
endif

ifneq ($(SO), $(MNV))
$(BINDIR)/$(TARGET).$(MNV): $(OBJS)
	$(CC) -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LDFLAGS)
	$(STRIP) $@
endif

# Mac OS X
$(BINDIR)/$(TARGET).dylib: $(OBJS)
	$(CC) -dynamiclib $(CFLAGS) -o $(BINDIR)/$(TARGET).dylib $(OBJS)
	$(STRIP) $@
#==============================================================================

#==============================================================================
# Rule for compiling test binaries that uses SDL2 as its backend (those are
# prefixed by 'tst_' and suffixed by 'SDL2')
#==============================================================================
$(BINDIR)/tst_%SDL2$(BIN_EXT): $(OBJDIR)/tst_%SDL2.o
	$(CC) $(CFLAGS) -o $@ $< -L$(BINDIR) $(LDFLAGS) -$(LIBNAME)_dbg $(SDL_LDFLAGS)
#==============================================================================

#==============================================================================
# Rule for compiling a test binary (it's prefixed by 'tst_')
#==============================================================================
$(BINDIR)/tst_%$(BIN_EXT): $(OBJDIR)/tst_%.o
	$(CC) $(CFLAGS) -o $@ $< -L$(BINDIR) $(LDFLAGS) -$(LIBNAME)_dbg
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

#==============================================================================
# Removes all built objects (use emscript_clean to clear the emscript stuff)
#==============================================================================
clean:
	rm -f $(OBJS) $(BINDIR)/$(TARGET).a $(BINDIR)/*
	rm -rf $(OBJDIR) $(BINDIR)
#==============================================================================

#==============================================================================
# Remove all built objects and target directories
#==============================================================================
distclean: clean
	make emscript_clean DEBUG=yes
	make emscript_clean RELEASE=yes
	make clean DEBUG=yes
	make clean RELEASE=yes
	rmdir obj/ bin/
#==============================================================================

