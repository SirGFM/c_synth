
#==============================================================================
# Import the configurations
#==============================================================================
  include conf/Makefile.conf
#==============================================================================

#===============================================================================
# Define every object required by compilation
#===============================================================================
  OBJS = lexer/synth_lexer.o \
         lexer/synth_fileLexer.o \
         memory/synth_memory.o
#===============================================================================

#===============================================================================
# Define every app (each must have its specific build rule)
#===============================================================================
  APPS = synth_tokenizer
#===============================================================================

#===============================================================================
# List of directories that must be generated
#===============================================================================
  DIRLIST := $(BINDIR) $(OBJDIR) $(OBJDIR)/lexer $(OBJDIR)/memory \
    $(OBJDIR)/app $(OBJDIR)/app/lexer $(OBJDIR)/app/memory
#===============================================================================

#==============================================================================
# Clear the suffixes' default rule, since there's an explicit one
#==============================================================================
.SUFFIXES:
#==============================================================================

#==============================================================================
# Define all targets that doesn't match its generated file
#==============================================================================
.PHONY: all shared static clean listflags mkdirs apps
#==============================================================================

#===============================================================================
# Define base path where source files can be found
#===============================================================================
  VPATH := src:app
#===============================================================================

#==============================================================================
# Make both objects and apps list constants. Also prepend the output folder
#==============================================================================
  OBJS := $(OBJS:%=$(OBJDIR)/%)
  APPS := $(APPS:%=$(BINDIR)/%)
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
all: listflags static shared apps
#===============================================================================

#==============================================================================
# Rule for building the static lib
#==============================================================================
listflags:
	@ echo "Active build flags:"
	@ echo "  OS     : $(OS)"
	@ echo "  ARCH   : $(ARCH)"
	@ echo "  CC     : $(CC)"
	@ echo "  DEBUG  : $(DEBUG)"
	@ echo "  VERSION: $(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)"
	@ echo "  CFLAGS : $(CFLAGS)"
#==============================================================================

#==============================================================================
# Rule for building the static lib
#==============================================================================
static: mkdirs $(BINDIR)/$(TARGET_NAME).a
#==============================================================================

#==============================================================================
# Rule for building the shared libs
#==============================================================================
shared: mkdirs $(BINDIR)/$(TARGET)
#==============================================================================

#==============================================================================
# Rule for building all apps
#==============================================================================
apps: mkdirs $(APPS)
#==============================================================================

#==============================================================================
# Rule for actually building the static library
#==============================================================================
$(BINDIR)/$(TARGET_NAME).a: $(OBJS)
	@ echo -n "Building the static lib '$@'... "
	@ rm -f $(BINDIR)/$(TARGET_NAME).a
	@ ar -cvq $(BINDIR)/$(TARGET_NAME).a $(OBJS) > /dev/null
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for actually building the shared library
#==============================================================================

# Windows DLL
# TODO Check if stderr redirection works on windows and fix here
$(BINDIR)/$(TARGET_NAME).dll: $(OBJS)
	@ echo -n "Building the DLL '$@'... "
	@ rm -f $@
	@ gcc -shared -Wl,-soname,$(TARGET_NAME).dll -Wl,-export-all-symbols \
	    $(CFLAGS) -o $@ $(OBJS) $(LFLAGS)
	@ echo "DONE"

# Main linux shared lib symlink (i.e., libCSynth.so)
$(BINDIR)/$(TARGET_NAME).so: $(BINDIR)/$(TARGET_MAJOR)
	@ echo -n "Building the shared lib '$@'... "
	@ rm -f $(BINDIR)/$(TARGET_NAME).so
	@ cd $(BINDIR); ln -f -s $(TARGET_MAJOR) $(TARGET_NAME).so
	@ echo "DONE"

# Major linux shared lib symlink (e.g., libCSynth.so.2)
$(BINDIR)/$(TARGET_MAJOR): $(BINDIR)/$(TARGET_MINOR)
	@ echo -n "Building the shared lib '$@'... "
	@ rm -f $(BINDIR)/$(TARGET_MAJOR)
	@ cd $(BINDIR); ln -f -s $(TARGET_MINOR) $(TARGET_MAJOR)
	@ echo "DONE"

# Linux shared lib (e.g., libCSynth.so.2.0.0)
$(BINDIR)/$(TARGET_MINOR): $(OBJS)
	@ echo -n "Building the shared lib '$@'... "
	@ rm -f $(BINDIR)/$(TARGET_MINOR)
	@ gcc -shared -Wl,-soname,$(TARGET_MAJOR) -Wl,-export-dynamic \
	    $(CFLAGS) -o $@ $(OBJS) $(LFLAGS) \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm err.out false)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for compiling any .c in its object
#==============================================================================
$(OBJDIR)/%.o: %.c
	@ echo -n "Compiling '$<' into '$@'... "
	@ $(CC) $(CFLAGS) -o $@ -c $< > /dev/null 2> err.out \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm err.out false)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for compiling any .c in its object (for apps)
#==============================================================================
$(OBJDIR)/app/%.o: %.c
	@ echo -n "Compiling '$<' into '$@'... "
	@ $(CC) $(CFLAGS) -DENABLE_MALLOC -o $@ -c $< > /dev/null 2> err.out \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm err.out false)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for creating every directory
#==============================================================================
mkdirs:
	@ echo -n "Checking/Creating necessary directories... "
	@ mkdir -p $(DIRLIST)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Removes all built objects (use emscript_clean to clear the emscript stuff)
#==============================================================================
clean:
	@ echo -n "Cleaning the project... "
	@ rm -f $(OBJS) $(BINDIR)/$(TARGET).a $(BINDIR)/*
	@ rm -rf $(DIRLIST)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for each specific app
#==============================================================================

SYNTH_TOKENIZER_OBJ := $(OBJDIR)/app/synth_tokenizer.o \
        $(OBJDIR)/app/lexer/synth_lexerDict.o \
        $(OBJDIR)/app/lexer/synth_lexer.o \
        $(OBJDIR)/app/lexer/synth_fileLexer.o \
        $(OBJDIR)/app/memory/synth_memory.o \
        $(OBJDIR)/app/memory/synth_dynamicMemory.o \

$(BINDIR)/synth_tokenizer: $(SYNTH_TOKENIZER_OBJ)
	@ echo -n "Building the app '$@'... "
	@ $(CC) $(CFLAGS) -o $@ $(SYNTH_TOKENIZER_OBJ)
	@ echo "DONE"

#==============================================================================

