# substeps
MAKE_COMPILE_SHADERS := $(MAKE) -k -j 2 -f ../make/compile_shaders.mk
MAKE_COMPILE := $(MAKE) -k -j 4 -f compile.mk

all: compile

compile: compile_shaders
	$(info >>> Starting Compilation...)
	@$(MAKE_COMPILE)

compile_shaders:
	$(info >>> Starting Shader Compilation...)
	@$(MAKE_COMPILE_SHADERS)

clean:
	@$(MAKE_COMPILE_SHADERS) clean
	@$(MAKE_COMPILE) clean
