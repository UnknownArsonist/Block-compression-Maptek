########################################################################
<<<<<<< HEAD
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -std=c++11 -Wall -Iinclude -pthread
LDFLAGS = 

# Makefile settings - Can be customized.
=======
####################### Makefile for Windows ##########################
########################################################################

# Compiler settings
CC = x86_64-w64-mingw32-g++        # 64-bit Windows compiler
CXXFLAGS = -std=c++11 -Wall -Iinclude -static
LDFLAGS = 

# Makefile settings
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
APPNAME = myapp
EXT = .cpp
SRCDIR = ./src
OBJDIR = ./obj
<<<<<<< HEAD
=======
EXE = .exe
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
<<<<<<< HEAD
DEP = $(OBJ:$(OBJDIR)/%.o=%.d)
# UNIX-based OS variables & settings
RM = rm
DELOBJ = $(OBJ)
# Windows OS variables & settings
DEL = del
EXE = .exe
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)
=======

# Windows-compatible remove command
RM = rm -f
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e

########################################################################
####################### Targets beginning here #########################
########################################################################

<<<<<<< HEAD
all: $(APPNAME)

# Builds the app
$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


# Includes all .h files
-include $(DEP)

# Building rule for .o files and its .c/.cpp in combination with all .h
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT)
	$(CC) $(CXXFLAGS) -o $@ -c $<

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) $(DELOBJ) $(DEP) $(APPNAME)

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) $(DEP)

#################### Cleaning rules for Windows OS #####################
# Cleans complete project
.PHONY: cleanw
cleanw:
	$(DEL) $(WDELOBJ) $(DEP) $(APPNAME)$(EXE)

# Cleans only all files with the extension .d
.PHONY: cleandepw
cleandepw:
	$(DEL) $(DEP)
=======
all: $(APPNAME)$(EXE)

# Create obj directory if it doesn't exist
$(OBJDIR):
	@if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"

# Build the application
$(APPNAME)$(EXE): $(OBJDIR) $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT)
	$(CC) $(CXXFLAGS) -c $< -o $@

################### Cleaning rules ###################
.PHONY: clean cleandep all

# Clean complete project
clean:
	$(RM) $(OBJDIR)\*.o
	$(RM) $(APPNAME)$(EXE)

# Clean only object files
cleandep:
	$(RM) $(OBJDIR)\*.o
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
