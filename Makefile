########################################################################
####################### Makefile for Linux ############################
########################################################################

# Compiler settings
CC = g++                        # Linux compiler
CXXFLAGS = -std=c++11 -Wall -Iinclude -pthread
LDFLAGS = -pthread

# Makefile settings
APPNAME = myapp
EXT = .cpp
SRCDIR = ./src
OBJDIR = ./obj
EXE = 

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)

# Linux remove command
RM = rm -f

########################################################################
####################### Targets beginning here #########################
########################################################################

all: $(APPNAME)$(EXE)

# Create obj directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

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
	$(RM) $(OBJDIR)/*.o
	$(RM) $(APPNAME)$(EXE)

# Clean only object files
cleandep:
	$(RM) $(OBJDIR)/*.o
