NAME := AndoEngine

#commands
CXX := clang++
CPP := clang
RM := rm -rf
MKDIR := mkdir -p

CXXFLAGS := -std=c++14 -Wall -c -MMD -MP -I./include
ARFLAGS := cq

#directories
DIRSRC := src
DIROBJ := obj

SRC := $(patsubst ./%, %, $(shell find . -name "*.cpp"))
OBJ := $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.o, $(SRC))
DEP := $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.d, $(SRC))
DIRS += $(sort $(patsubst %/, %, $(dir $(OBJ))))

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
	@$(RM) $(TARGET) $(DIROBJ)

$(DIROBJ)/%.o: $(DIRSRC)/%.cpp
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

-include $(DEP) # include all dep files

$(TARGET): $(OBJ)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $^
