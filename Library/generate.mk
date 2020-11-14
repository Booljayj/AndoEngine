# directories
DIR_SOURCE := source
DIR_GENERATED := generated
DIR_PROGRAMS := programs

# scripts
ENUM_SCRIPT := $(DIR_PROGRAMS)/GenerateEnum.py
EMBED_SCRIPT := $(DIR_PROGRAMS)/Embed.py

# commands
RM := rm -rf
MKDIR := mkdir -p
ENUM := python3 $(ENUM_SCRIPT)
EMBED := python3 $(EMBED_SCRIPT)

#generated files
ENUM_SOURCE := $(patsubst ./%, %, $(shell find ./$(DIR_SOURCE) -name "*.enum"))
ENUM_GENERATED_OUTPUT := $(patsubst $(DIR_SOURCE)/%.enum, $(DIR_GENERATED)/%.enum.cpp, $(ENUM_SOURCE))

ALL_GENERATED_OUTPUT := $(ENUM_GENERATED_OUTPUT)

# targets
all: $(ALL_GENERATED_OUTPUT)
	$(info > Done)

.PHONY: directories
directories:
	@-$(MKDIR) $(sort $(patsubst %/, %, $(dir $(ALL_GENERATED_OUTPUT))))

.PHONY: clean
clean:
	$(info > Cleaning generation files...)
	@$(RM) $(DIR_GENERATED)/*

# generate enum source files
$(DIR_GENERATED)/%.enum.cpp: $(DIR_SOURCE)/%.enum $(ENUM_SCRIPT) | directories
	$(info > Generating enum $(basename $<)...)
	@$(ENUM) $< $(dir $@)
