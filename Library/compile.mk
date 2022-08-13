# main
NAME := AndoEngine
TARGETTYPE := library

INCLUDE_PATHS := /usr/local/include/freetype2/
PRECOMPILE := Engine/StandardTypes.h

DEFINE := VULKAN_DEBUG

include ../make/std_compile.mk
