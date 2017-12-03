# substeps
MAKE_GENERATE := $(MAKE) -k -j 2 -f generate.mk
MAKE_COMPILE := $(MAKE) -k -j 4 -f compile.mk

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
