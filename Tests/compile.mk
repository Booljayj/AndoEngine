# main
NAME := Tests
TARGET = $(NAME).out
TARGETTYPE := executable

LIBS := SDL2 GLEW vulkan MoltenVK
FRAMEWORKS := OpenGL

# core library
CORELIB := AndoEngine
CORELIB_PATH := ../Library

CXXFLAGS += -I$(CORELIB_PATH)/source -I$(CORELIB_PATH)/generated
LDFLAGS += -L$(CORELIB_PATH)
LDLIBS += -l$(CORELIB)

include ../make/std_compile.mk
