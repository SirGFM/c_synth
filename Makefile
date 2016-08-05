
#==============================================================================
# Import the configurations
#------------------------------------------------------------------------------
# ARCH=x86_64 -> -m64
# DEBUG=yes -> -g -O0 -DDEBUG
# STRICT=yes -> -DSTRICT
# ...
#==============================================================================
  include conf/Makefile.conf
#==============================================================================

#===============================================================================
# Define every object required by compilation
#===============================================================================
  OBJS := lexer/synth_lexer.o \
          lexer/synth_fileLexer.o \
          memory/synth_memory.o \
          error/synth_errorDict.o \
          synth_type.o
#===============================================================================

#===============================================================================
# Extra objects used only on debug or strict mode
#===============================================================================
# I'll fix this eventually!!! (NO IFs SHOULD BE PLACED ON THE MAIN MAKEFILE)
ifneq (, $(filter yes, $(DEBUG) $(STRICT)))
  EXTRA_OBJS := error/synth_error.o
endif
#===============================================================================

#===============================================================================
# Define every app (each must have its specific build rule)
#===============================================================================
  APPS = synth_tokenizer
#===============================================================================

#===============================================================================
# List of directories that must be generated
#===============================================================================
  DIRS := error lexer memory
# List all directories used when compiling with dynamic memory
  DIRS := $(DIRS) dyn $(DIRS:%=dyn/%)
# Generated a list with all the required directories
  DIRLIST := $(BASE_BINDIR) $(BASE_OBJDIR)
  DIRLIST := $(DIRLIST) $(DIRS:%=$(BASE_OBJDIR)/$(RELEASE_TARGET)/%)
  DIRLIST := $(DIRLIST) $(DIRS:%=$(BASE_OBJDIR)/$(DEBUG_TARGET)/%)
  DIRLIST := $(DIRLIST) $(BASE_BINDIR)/$(DEBUG_TARGET)
  DIRLIST := $(DIRLIST) $(BASE_BINDIR)/$(RELEASE_TARGET)
#===============================================================================

#==============================================================================
# Clear the suffixes' default rule, since there's an explicit one
#==============================================================================
.SUFFIXES:
#==============================================================================

#==============================================================================
# Define all targets that doesn't match its generated file
#==============================================================================
.PHONY: all shared static clean help mkdirs apps install_linux \
        install_win install_macosx
#==============================================================================

#===============================================================================
# Define base path where source files can be found
#===============================================================================
  VPATH := src:app
#===============================================================================

#==============================================================================
# Make both objects and apps list constants. Also prepend the output folder
#==============================================================================
  OBJS     := $(OBJS) $(EXTRA_OBJS)
  LIB_OBJS := $(OBJS:%=$(OBJDIR)/%)
  APPS     := $(APPS:%=$(BINDIR)/%)
#==============================================================================

#===============================================================================
# Define default compilation rule
#===============================================================================
all: help static shared apps
#===============================================================================

#==============================================================================
# Display a short helper
#==============================================================================
help:
	@ echo "Active build flags:"
	@ echo "  OS      : $(OS)"
	@ echo "  ARCH    : $(ARCH)"
	@ echo "  CC      : $(CC)"
	@ echo "  DEBUG   : $(DEBUG)"
	@ echo "  VERSION : $(VERSION)"
	@ echo "  CFLAGS  : $(CFLAGS)"
	@ echo "  LDFLAGS : $(LDFLAGS)"
	@ echo ""
	@ echo -n "NOTE: If the compilation fails with error \"No such file or "
	@ echo      "directory\", try running:"
	@ echo "    '$$ make OS=$(OS) mkdirs'"
	@ echo ""
#==============================================================================

#==============================================================================
# Rule for building the static lib
#==============================================================================
static: $(BINDIR)/$(LIB_NAME).a
#==============================================================================

#==============================================================================
# Rule for building the shared libs
#==============================================================================
shared: $(BINDIR)/$(TARGET_LIB)
#==============================================================================

#==============================================================================
# Rule for building all apps
#==============================================================================
apps: $(APPS)
#==============================================================================

#==============================================================================
# Rule for installing the lib
#==============================================================================
install: $(INSTALL_RULE)

# TODO
#install_linux:
#install_win:
#install_macosx:

#==============================================================================

#==============================================================================
# Rule for actually building the static library
#==============================================================================
$(BINDIR)/$(LIB_NAME).a: $(LIB_OBJS)
	@ echo -n "[  AR  ] $@... "
	@ rm -f $@
	@ ar -cvq $@ $(LIB_OBJS) > /dev/null
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for actually building the shared library
#==============================================================================

# Windows DLL
$(BINDIR)/$(LIB_NAME).dll: $(LIB_OBJS)
	@ echo -n "[  DLL ] $@... "
	@ rm -f $@
	@ gcc -shared -Wl,-soname,$(LIB_NAME).dll -Wl,-export-all-symbols \
	    $(CFLAGS) -o $@ $(LIB_OBJS) $(LDFLAGS)
	@ echo "DONE"

# Main linux shared lib symlink (i.e., libCSynth.so)
$(BINDIR)/$(LIB_NAME).so: $(BINDIR)/$(LIB_NAME).so.$(MAJOR_VERSION)
	@ echo -n "[  SO  ] $@... "
	@ rm -f $@
	@ cd $(BINDIR); ln -f -s $(LIB_NAME).so.$(MAJOR_VERSION) $(LIB_NAME).so
	@ echo "DONE"

# Major linux shared lib symlink (e.g., libCSynth.so.2)
$(BINDIR)/$(LIB_NAME).so.$(MAJOR_VERSION): $(BINDIR)/$(LIB_NAME).so.$(VERSION)
	@ echo -n "[  SO  ] $@... "
	@ rm -f $@
	@ cd $(BINDIR); ln -f -s $(LIB_NAME).so.$(VERSION) $(LIB_NAME).so.$(MAJOR_VERSION)
	@ echo "DONE"

# Linux shared lib (e.g., libCSynth.so.2.0.0)
$(BINDIR)/$(LIB_NAME).so.$(VERSION): $(LIB_OBJS)
	@ echo -n "[  SO  ] $@... "
	@ rm -f $@
	@ gcc -shared -Wl,-soname,$(LIB_NAME).so.$(MAJOR_VERSION) -Wl,-export-dynamic \
	    $(CFLAGS) -o $@ $(LIB_OBJS) $(LDFLAGS) \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm err.out false)
	@ echo "DONE"

# Mac OS X shared lib (e.g., libCSynth.dylib)
$(BINDIR)/$(LIB_NAME).dylib: $(LIB_OBJS)
	@ echo -n "[ DYLIB] $@... "
	@ rm -f $@
	@ gcc -dynamiclib $(CFLAGS) -o $@ $(LIB_OBJS) $(LDFLAGS) \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm -f err.out; false)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for compiling any .c into its object
#==============================================================================
$(OBJDIR)/%.o: %.c
	@ echo -n "[  CC  ] $@ < $<... "
	@ $(CC) $(CFLAGS) -o $@ -c $< > /dev/null 2> err.out \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm -f err.out; false)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for compiling any .c into its object (with malloc enabled)
#==============================================================================
$(OBJDIR)/dyn/%.o: %.c
	@ echo -n "[CC APP] $@ < $<... "
	@ $(CC) $(CFLAGS) -DENABLE_MALLOC -o $@ -c $< > /dev/null 2> err.out \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm -f err.out; false)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for creating every directory
#==============================================================================
mkdirs:
	@ echo -n "[MKDIRS] ... "
	@ mkdir -p $(DIRLIST)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Removes all built objects
#==============================================================================
clean:
	@ echo -n "[ CLEAN ] ... "
	@ rm -rf $(DIRLIST)
	@ echo "DONE"
#==============================================================================

#==============================================================================
# Rule for each specific app
#==============================================================================
SYNTH_TOKENIZER_OBJ := $(OBJS) \
                       lexer/synth_lexerDict.o \
                       memory/synth_dynamicMemory.o \
                       synth_tokenizer.o
SYNTH_TOKENIZER_OBJ := $(SYNTH_TOKENIZER_OBJ:%=$(OBJDIR)/dyn/%)

$(BINDIR)/synth_tokenizer: $(SYNTH_TOKENIZER_OBJ)
	@ echo -n "[  APP ] $@... "
	@ $(CC) $(CFLAGS) -o $@ $(SYNTH_TOKENIZER_OBJ) > /dev/null 2> err.out \
	    && (rm -f err.out ; true) \
	    || (echo "[FAILED]"; cat err.out >&2 ; rm -f err.out; false)
	@ echo "DONE"

#==============================================================================

