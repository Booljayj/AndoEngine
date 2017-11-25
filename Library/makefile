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
ENUM_BUILDER := gen/GenerateEnum.py
ENUM_BUILD := python3 gen/GenerateEnum.py
ENUM_INPUT := $(patsubst ./%, %, $(shell find ./$(DIRGEN) -name "*.enum"))

ENUM_SRC := $(patsubst $(DIRGEN)/%.enum, $(DIRSRC)/%.enum.cpp, $(ENUM_INPUT))
ENUM_INC := $(patsubst $(DIRGEN)/%.enum, $(DIRINC)/%.enum.h, $(ENUM_INPUT))
#ENUM_OBJ := $(patsubst $(DIRINC)/%, $(DIROBJ)/%.o, $(ENUM_INPUT))
#ENUM_DEP := $(patsubst $(DIRINC)/%, $(DIROBJ)/%.d, $(ENUM_INPUT))
#ENUM_DIRS = $(sort $(patsubst %/, %, $(dir $(ENUM_OBJ))))
.SECONDARY: $(ENUM_SRC) #prevent generated source from being cleaned up after build.

#targets
TARGET = lib$(NAME).a
TARGETTYPE := library

all: generate directories $(TARGET)
	$(info > Done)

.PHONY: generate
generate: $(ENUM_SRC)

.PHONY: directories
directories:
	@$(MKDIR) $(ENUM_DIRS) $(DIRS)

.PHONY: clean
clean:
	$(info > Cleaning...)
	@$(RM) $(TARGET) $(DIROBJ) $(ENUM_SRC) $(ENUM_INC)

#generic rule for normal source files
$(DIROBJ)/%.o: $(DIRSRC)/%.cpp
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

#rule for generating enum source files (header generation is incidental)
$(DIRSRC)/%.enum.cpp: $(DIRGEN)/%.enum $(ENUM_BUILDER)
	$(info > Generating $<...)
	@$(ENUM_BUILD) $< $(patsubst $(DIRGEN)/%, $(DIRINC)/%, $(dir $<)) $(patsubst $(DIRGEN)/%, $(DIRSRC)/%, $(dir $<))

#include all dep files
-include $(DEP)
-include $(ENUM_DEP)

$(TARGET): $(ENUM_OBJ) $(OBJ)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $(OBJ)
