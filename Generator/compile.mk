# main
NAME := Generator
TARGETTYPE := executable
LIBS := clangTooling clangToolingCore clangFrontend clangSerialization clangDriver clangRewriteFrontend clangRewrite clangParse\
	clangSema clangAnalysis clangAST clangASTMatchers clangEdit clangLex clangBasic clangStaticAnalyzerFrontend
DIR_RESOURCES := /usr/local/opt/llvm

CXXFLAGS := $(shell $(DIR_RESOURCES)/bin/llvm-config --cxxflags)
LDFLAGS := $(shell $(DIR_RESOURCES)/bin/llvm-config --ldflags)
LDLIBS := $(shell $(DIR_RESOURCES)/bin/llvm-config --libs --system-libs)

include ../make/std_compile.mk
