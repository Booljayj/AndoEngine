# main
NAME := AndoEngine
TARGET = lib$(NAME).a
TARGETTYPE := library

# directories
DIR_SOURCE := source
DIR_GENERATED := generated
DIR_BUILD := build

# commands
CXX := clang++
CPP := clang
AR := libtool
RM := rm -rf
MKDIR := mkdir -p

CXXFLAGS := -std=c++14 -Wall -c -MMD -MP -I./$(DIR_SOURCE) -I./$(DIR_GENERATED) -I/usr/local/include/
ARFLAGS := -static -o

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
$(TARGET): $(BUILD_OBJ)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $(BUILD_OBJ)

# normal source files
$(DIR_BUILD)/%.o: $(DIR_SOURCE)/%.cpp | directories
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

# generated source files
$(DIR_BUILD)/%.o: $(DIR_GENERATED)/%.cpp | directories
	$(info > Compiling $(basename $<)...)
	@$(CXX) $(CXXFLAGS) $< -o $@

# dependency files
-include $(BUILD_DEP)
