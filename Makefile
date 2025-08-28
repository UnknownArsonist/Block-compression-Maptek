########################################################################
####################### Makefile for Windows ##########################
########################################################################

# Compiler settings
CC = x86_64-w64-mingw32-g++        # 64-bit Windows compiler
CXXFLAGS = -std=c++11 -Wall -Iinclude -static
LDFLAGS = 

# Makefile settings
APPNAME = myapp
EXT = .cpp
SRCDIR = ./src
OBJDIR = ./obj
EXE = .exe

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)

# Windows-compatible remove command
RM = del /Q /F

########################################################################
####################### Targets beginning here #########################
########################################################################

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
	-$(RM) "$(OBJDIR)\*.o"
	-$(RM) "$(APPNAME)$(EXE)"

# Clean only object files
cleandep:
	-$(RM) "$(OBJDIR)\*.o"