# substeps
MAKE_COMPILE_SHADERS := $(MAKE) -j2 -f ../make/std_compile_shaders.mk
MAKE_COMPILE := $(MAKE) -j4 -f compile.mk

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
