# directories
DIR_CONTENT := content/shaders
DIR_BUILD := content/shaders/compiled

# tools
RM := rm -rf
MKDIR := mkdir -p

SPVC := glslc
SPVC_FLAGS := -O -Werror #dependencies not included here because they require per-target values

# source files
VERT_SOURCE := $(patsubst ./%, %, $(shell find ./$(DIR_CONTENT) -name "*.vert"))
FRAG_SOURCE := $(patsubst ./%, %, $(shell find ./$(DIR_CONTENT) -name "*.frag"))

# output files
BUILD_SPV := $(patsubst $(DIR_CONTENT)/%.vert, $(DIR_BUILD)/%.vert.spv, $(VERT_SOURCE)) $(patsubst $(DIR_CONTENT)/%.frag, $(DIR_BUILD)/%.frag.spv, $(FRAG_SOURCE))
BUILD_DEP := $(patsubst $(DIR_CONTENT)/%.vert, $(DIR_BUILD)/%.vert.d, $(VERT_SOURCE)) $(patsubst $(DIR_CONTENT)/%.frag, $(DIR_BUILD)/%.frag.d, $(FRAG_SOURCE))

all: $(BUILD_SPV)
	$(info > Done)

.PHONY: directories
directories:
	@-$(MKDIR) $(sort $(patsubst %/, %, $(dir $(BUILD_SPV))))

.PHONY: clean
clean:
	$(info > Cleaning compiled shader files...)
	@$(RM) $(TARGET) $(DIR_BUILD)/*

# vert shader files
$(DIR_BUILD)/%.vert.spv: $(DIR_CONTENT)/%.vert | directories
	$(info > Compiling vertex shader $(basename $<)...)
	@$(SPVC) $(SPVC_FLAGS) -MD -MF $@.d $< -o $@

# frag shader files
$(DIR_BUILD)/%.frag.spv: $(DIR_CONTENT)/%.frag | directories
	$(info > Compiling fragment shader $(basename $<)...)
	@$(SPVC) $(SPVC_FLAGS) -MD -MF $@.d $< -o $@

# dependency files
-include $(BUILD_DEP)
