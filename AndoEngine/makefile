NAME := AndoEngine

#commands
CXX := clang++
CPP := clang
AR := libtool
RM := rm -rf
MKDIR := mkdir -p

CXXFLAGS := -std=c++14 -Wall -c -MMD -MP -I./include -I./gen
ARFLAGS := -static -o

#directories
DIRSRC := src
DIRINC := include
DIROBJ := obj
DIRGEN := gen

#normal source files
SRC = $(patsubst ./%, %, $(shell find ./$(DIRSRC) -name "*.cpp"))
OBJ = $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.o, $(SRC))
DEP = $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.d, $(SRC))
DIRS = $(sort $(patsubst %/, %, $(dir $(OBJ))))

#generated files
ENUM_BUILDER := bin/GenerateEnum.py
ENUM_BUILD := python3 bin/GenerateEnum.py
ENUM_INPUT := $(patsubst ./%, %, $(shell find ./$(DIRINC) -name "*.enum"))

ENUM_SRC := $(patsubst $(DIRINC)/%, $(DIRGEN)/%.cpp, $(ENUM_INPUT))
ENUM_OBJ := $(patsubst $(DIRINC)/%, $(DIROBJ)/%.o, $(ENUM_INPUT))
ENUM_DEP := $(patsubst $(DIRINC)/%, $(DIROBJ)/%.d, $(ENUM_INPUT))
ENUM_DIRS = $(sort $(patsubst %/, %, $(dir $(ENUM_OBJ))))
.SECONDARY: $(ENUM_SRC) #prevent generated source from being cleaned up after build.

#targets
TARGET = lib$(NAME).a
TARGETTYPE := library

all: directories $(TARGET)
	$(info > Done)

.PHONY: directories
directories:
	@$(MKDIR) $(ENUM_DIRS) $(DIRS)

.PHONY: clean
clean:
	$(info > Cleaning...)
	@$(RM) $(TARGET) $(DIROBJ) $(DIRGEN)

#generic rule for normal source files
$(DIROBJ)/%.o: $(DIRSRC)/%.cpp
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

#generic rule for generated source files
$(DIROBJ)/%.enum.o: $(DIRGEN)/%.enum.cpp
	$(info > Compiling $(basename $<) (generated)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

#rule for generating enum source files (header generation is incidental)
$(DIRGEN)/%.enum.cpp: $(DIRINC)/%.enum $(ENUM_BUILDER)
	$(info > Generating $<...)
	@$(ENUM_BUILD) $< $(dir $@)

#include all dep files
-include $(DEP)
-include $(ENUM_DEP)

$(TARGET): $(ENUM_OBJ) $(OBJ)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $(OBJ)
