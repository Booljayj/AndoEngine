# substeps
MAKE_COMPILE := $(MAKE) -k -j 4 -f compile.mk

all: compile

compile:
	$(info >>> Starting Compilation...)
	@$(MAKE_COMPILE)

clean:
	@$(MAKE_COMPILE) clean
