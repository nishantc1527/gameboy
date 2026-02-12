CC            ?= gcc
FORMAT        ?= clang-format

VENV          := .venv
PYTHON        := $(VENV)/bin/python3
PIP           := $(VENV)/bin/pip3
REQS          := requirements.txt

BIN           := gbemu
BUILD_DIR     := build
SRC_DIR       := src

INC_DIRS      := config include vendor
CPPFLAGS      := $(foreach d, $(INC_DIRS), -I$(d)) $(shell pkg-config --cflags sdl3)
CFLAGS        := -Wall -Wextra -Wpedantic -O2 -std=c2x
LDLIBS        := $(shell pkg-config --libs sdl3)

C_SRCS        := $(shell find $(SRC_DIR) -name "*.c")
OBJS          := $(C_SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS          := $(OBJS:.o=.d)

.PHONY: all clean format test

all: $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(RUST_HDR)

format:
	$(FORMAT) -i $(C_SRCS) $(shell find $(SRC_DIR) -name "*.h")

$(PYTHON):
	python3 -m venv $(VENV)
	$(PIP) install --upgrade pip

test: $(PYTHON) $(BUILD_DIR)/$(BIN)
	$(PYTHON) -m pip install -r $(REQS)
	$(PYTHON) -m pytest -v

-include $(DEPS)
