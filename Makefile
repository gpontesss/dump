CC := gcc
CC_FLAGS := -Wall
BIN_DIR := bin

TARGETS := $(addprefix $(BIN_DIR)/,$(basename $(shell find * -type f -path '*.c')))

all: $(TARGETS)

$(BIN_DIR):
	@echo "Creating bin dir"
	@mkdir -p $(BIN_DIR)

$(BIN_DIR)/%: %.c | $(BIN_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CC_FLAGS) $+ -o $@

clean:
	@rm -f $(BIN_DIR)
