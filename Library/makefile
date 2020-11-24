# substeps
MAKE_GENERATE := $(MAKE) -j2 -f generate.mk
MAKE_COMPILE := $(MAKE) -j4 -f compile.mk

all: compile

compile: generate
	$(info >>> Starting Compilation...)
	@$(MAKE_COMPILE)

generate:
	$(info >>> Starting Generation...)
	@$(MAKE_GENERATE)

clean:
	@$(MAKE_GENERATE) clean
	@$(MAKE_COMPILE) clean
