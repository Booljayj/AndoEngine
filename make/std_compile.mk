# required variables
ifndef NAME
$(error Must define a NAME variable before including "std_compile.mk")
endif
ifndef TARGETTYPE
$(error Must define a TARGETTYPE variable before including "std_compile.mk")
endif

ifeq ($(TARGETTYPE),library)
IS_LIBRARY := 1
TARGET = lib$(NAME).a
else ifeq ($(TARGETTYPE),executable)
IS_EXECUTABLE := 1
TARGET := $(NAME).out
else
$(error TARGETTYPE must be "library" or "executable")
endif

# directories
DIR_SOURCE := source
DIR_GENERATED := generated
DIR_BUILD := build

# tools
ifdef DIR_RESOURCES
CXX := $(DIR_RESOURCES)/bin/clang++
CPP := $(DIR_RESOURCES)/bin/clang
else
CXX := clang++
CPP := clang
endif

AR := libtool
RM := rm -rf
MKDIR := mkdir -p

# flags (appended to existing defined flags)
CXXFLAGS := $(CXXFLAGS) -std=gnu++17 -g -Wall -c -MMD -MP -I./$(DIR_SOURCE) -I./$(DIR_GENERATED)
LDFLAGS := $(LDFLAGS)
ARFLAGS := -static -o

# compilation defines
ifdef DEFINE
CXXFLAGS += $(foreach DEF, $(DEFINE), -D$(DEFINE))
endif

# external libraries
ifdef LIBS
LDLIBS += $(foreach LIB, $(LIBS), -l$(LIB))
endif

# external frameworks (OSX)
ifdef FRAMEWORKS
LDLIBS += -framework $(FRAMEWORKS)
endif

# source files
CPP_SOURCE := $(patsubst ./%, %, $(shell find ./$(DIR_SOURCE) -name "*.cpp"))
CPP_SOURCE_GENERATED := $(patsubst ./%, %, $(shell find ./$(DIR_GENERATED) -name "*.cpp"))

# output files
BUILD_OBJ := $(patsubst $(DIR_SOURCE)/%.cpp, $(DIR_BUILD)/%.o, $(CPP_SOURCE)) $(patsubst $(DIR_GENERATED)/%.cpp, $(DIR_BUILD)/%.o, $(CPP_SOURCE_GENERATED))
BUILD_DEP := $(patsubst $(DIR_SOURCE)/%.cpp, $(DIR_BUILD)/%.d, $(CPP_SOURCE)) $(patsubst $(DIR_GENERATED)/%.cpp, $(DIR_BUILD)/%.d, $(CPP_SOURCE_GENERATED))

# targets
all: $(TARGET)
	$(info > Done)

.PHONY: directories
directories:
	@-$(MKDIR) $(sort $(patsubst %/, %, $(dir $(BUILD_OBJ))))

.PHONY: clean
clean:
	$(info > Cleaning compilation files...)
	@$(RM) $(TARGET) $(DIR_BUILD)/*

# main target
ifdef IS_LIBRARY
$(TARGET): $(BUILD_OBJ)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $(BUILD_OBJ)
else ifdef IS_EXECUTABLE
$(TARGET): $(BUILD_OBJ)
	$(info > Building $(TARGETTYPE) $(NAME)...)
	@$(RM) $@
	@$(CXX) $(LDFLAGS) $(LDLIBS) $(BUILD_OBJ) -o $@
endif

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
