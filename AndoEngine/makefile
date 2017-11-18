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

SRC = $(patsubst ./%, %, $(shell find ./$(DIRSRC) -name "*.cpp"))
GEN = $(patsubst ./%, %, $(shell find ./$(DIRGEN) -name "*.cpp")) #generated source files
OBJ = $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.o, $(SRC)) $(patsubst $(DIRGEN)/%.cpp, $(DIROBJ)/%.o, $(GEN))
DEP = $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.d, $(SRC)) $(patsubst $(DIRGEN)/%.cpp, $(DIROBJ)/%.d, $(GEN))
DIRS = $(sort $(patsubst %/, %, $(dir $(OBJ))))

#generated files
ENUM_BUILDER := bin/GenerateEnum.py
ENUM_BUILD := python3 bin/GenerateEnum.py
ENUM_SRC := $(patsubst ./%, %, $(shell find ./$(DIRINC) -name "*.enum"))
ENUM_OUT := $(foreach FILE, $(patsubst $(DIRINC)/%, %, $(ENUM_SRC)), $(DIRGEN)/$(FILE).cpp $(DIRGEN)/$(FILE).h)

#targets
TARGET = lib$(NAME).a
TARGETTYPE := library

all: directories $(TARGET)
	$(info > Done)

.PHONY: directories
directories:
	@$(MKDIR) $(DIRS)

.PHONY: clean
clean:
	$(info > Cleaning...)
	@$(RM) $(TARGET) $(DIROBJ) $(DIRGEN)

$(DIROBJ)/%.o: $(DIRSRC)/%.cpp
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

$(DIROBJ)/%.o: $(DIRGEN)/%.cpp
	$(info > Compiling $(basename $<) (generated)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

$(DIRGEN)/%.enum.cpp: $(DIRINC)/%.enum $(ENUM_BUILDER)
	$(info > Generating $<...)
	@$(ENUM_BUILD) $< $(dir $@)

$(DIRGEN)/%.enum.h: $(DIRINC)/%.enum $(ENUM_BUILDER)
	$(info > Generating $<...)
	@$(ENUM_BUILD) $< $(dir $@)

-include $(DEP) # include all dep files

$(TARGET): $(ENUM_OUT) $(OBJ)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $(OBJ)
