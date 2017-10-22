NAME := TestExecutable
LIBS := SDL2 GLEW
FRAMEWORKS := OpenGL
CORELIB := AndoEngine

#commands
CXX := clang++
CPP := clang
RM := rm -rf
MKDIR := mkdir -p

CXXFLAGS := -std=c++14 -Wall -c -MMD -MP -I./include

ifdef LIBS #external libraries
LDLIBS += $(foreach LIB, $(LIBS), -l$(LIB))
endif

ifdef FRAMEWORKS
LDLIBS += -framework $(FRAMEWORKS)
endif

#core library
CXXFLAGS += -I../$(CORELIB)/include
LDFLAGS += -L../$(CORELIB)/
LDLIBS += -l$(CORELIB)

#directories
DIRSRC := src
DIROBJ := obj
DIRBIN := bin

SRC := $(patsubst ./%, %, $(shell find . -name "*.cpp"))
OBJ := $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.o, $(SRC))
DEP := $(patsubst $(DIRSRC)/%.cpp, $(DIROBJ)/%.d, $(SRC))
DIRS := $(sort $(DIROBJ) $(DIRBIN) $(patsubst %/, %, $(dir $(OBJ))))

#targets
TARGET := $(DIRBIN)/$(NAME).out
TARGETTYPE := executable

all: directories corelib $(TARGET)
	$(info > Done)

.PHONY: directories
directories:
	@$(MKDIR) $(DIRS)

.PHONY: clean
clean:
	$(info > Cleaning...)
	@$(RM) $(TARGET) $(DIROBJ) $(DIRBIN)

-include $(DEP) # include all dep files

$(DIROBJ)/%.o: $(DIRSRC)/%.cpp
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(info > Building $(TARGETTYPE) $(THIS)...)
	@$(RM) $@
	@$(CXX) $(LDFLAGS) $(LDLIBS) $^ -o $@

.PHONY: corelib
corelib:
	@$(MAKE) -C ../$(CORELIB)
