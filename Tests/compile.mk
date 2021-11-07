# core library
CORELIB := AndoEngine
CORELIB_PATH := ../Library

# standard compilation
NAME := Tests
TARGETTYPE := executable

DEPENDENCIES := $(CORELIB_PATH)/lib$(CORELIB).a
INCLUDE_PATHS := $(CORELIB_PATH)/source $(CORELIB_PATH)/generated
LIBS := AndoEngine SDL2 GLEW vulkan MoltenVK
LIB_PATHS := ../Library
FRAMEWORKS := OpenGL

DEFINE := VULKAN_DEBUG

include ../make/std_compile.mk
