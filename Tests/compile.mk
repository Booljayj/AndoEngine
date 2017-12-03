# main
NAME := Tests
TARGET = $(NAME).out
TARGETTYPE := executable
LIBS := SDL2 GLEW
FRAMEWORKS := OpenGL

# directories
DIR_SOURCE := source
DIR_GENERATED := generated
DIR_BUILD := build

# commands
CXX := clang++
CPP := clang
RM := rm -rf
MKDIR := mkdir -p

CXXFLAGS := -std=c++14 -Wall -c -MMD -MP -I./$(DIR_SOURCE) -I./$(DIR_GENERATED) -I/usr/local/include/

ifdef LIBS # external libraries
LDLIBS += $(foreach LIB, $(LIBS), -l$(LIB))
endif

ifdef FRAMEWORKS # external frameworks (OSX)
LDLIBS += -framework $(FRAMEWORKS)
endif

# core library
CORELIB := AndoEngine
CORELIB_PATH := ../Library
CORELIB_FILE := $(CORELIB_PATH)/lib$(CORELIB).a

CXXFLAGS += -I$(CORELIB_PATH)/source -I$(CORELIB_PATH)/generated
LDFLAGS += -L$(CORELIB_PATH)
LDLIBS += -l$(CORELIB)

# source files
CPP_SOURCE := $(patsubst ./%, %, $(shell find ./$(DIR_SOURCE) -name "*.cpp"))
CPP_SOURCE_GENERATED := $(patsubst ./%, %, $(shell find ./$(DIR_GENERATED) -name "*.cpp"))

# output files
BUILD_OBJ := $(patsubst $(DIR_SOURCE)/%.cpp, $(DIR_BUILD)/%.o, $(CPP_SOURCE)) $(patsubst $(DIR_GENERATED)/%.cpp, $(DIR_BUILD)/%.o, $(CPP_SOURCE_GENERATED))
BUILD_DEP := $(patsubst $(DIR_SOURCE)/%.cpp, $(DIR_BUILD)/%.d, $(CPP_SOURCE)) $(patsubst $(DIR_GENERATED)/%.cpp, $(DIR_BUILD)/%.d, $(CPP_SOURCE_GENERATED))

#targets
all: $(TARGET)
	$(info > Done)

.PHONY: directories
directories:
	@$(MKDIR) $(sort $(patsubst %/, %, $(dir $(BUILD_OBJ))))

.PHONY: clean
clean:
	$(info > Cleaning compilation files...)
	@$(RM) $(TARGET) $(DIR_BUILD)/*

# main target
$(TARGET): $(BUILD_OBJ) $(CORELIB_FILE)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(CXX) $(LDFLAGS) $(LDLIBS) $(BUILD_OBJ) -o $@

# core library
$(CORELIB_FILE):
	@$(MAKE) -C $(CORELIB_PATH)

# normal source files
$(DIR_BUILD)/%.o: $(DIR_SOURCE)/%.cpp $(CORELIB_FILE) | directories
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

# generated source files
$(DIR_BUILD)/%.o: $(DIR_GENERATED)/%.cpp $(CORELIB_FILE) | directories
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

# dependency files
-include $(BUILD_DEP)
