CC            ?= gcc
CARGO         ?= cargo
CBINDS        ?= cbindgen
FORMAT        ?= clang-format

VENV          := .venv
PYTHON        := $(VENV)/bin/python3
PIP           := $(VENV)/bin/pip3
REQS          := requirements.txt

RUST_DIR      := rust
RUST_LIB      := $(RUST_DIR)/target/release/librust.a
RUST_HDR      := include/rust.h
RUST_MANIFEST := $(RUST_DIR)/Cargo.toml
RUST_CRATE    := rust

BIN           := gbemu
BUILD_DIR     := build
SRC_DIR       := src

INC_DIRS      := config include vendor
CPPFLAGS      := $(foreach d, $(INC_DIRS), -I$(d)) $(shell pkg-config --cflags sdl3)
CFLAGS        := -Wall -Wextra -Wpedantic -O2 -std=c2x
LDLIBS        := $(shell pkg-config --libs sdl3)

C_SRCS        := $(shell find $(SRC_DIR) -name "*.c")
RUST_SRCS     := $(shell find $(RUST_DIR)/src -name "*.rs")
OBJS          := $(C_SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS          := $(OBJS:.o=.d)

.PHONY: all clean format test

all: $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): $(RUST_LIB) $(OBJS)
	$(CC) $(OBJS) $(RUST_LIB) -o $@ $(LDLIBS)

$(OBJS): $(RUST_HDR)
$(RUST_HDR): $(RUST_LIB)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(RUST_LIB): $(RUST_MANIFEST) $(RUST_SRCS)
	$(CARGO) build --manifest-path $(RUST_MANIFEST) --release
	$(CBINDS) $(RUST_DIR) --crate $(RUST_CRATE) --output $(RUST_HDR)

clean:
	rm -rf $(BUILD_DIR) $(RUST_HDR)
	$(CARGO) clean --manifest-path $(RUST_MANIFEST)

format:
	$(FORMAT) -i $(C_SRCS) $(shell find $(SRC_DIR) -name "*.h")

$(PYTHON):
	python3 -m venv $(VENV)
	$(PIP) install --upgrade pip

test: $(PYTHON) $(BUILD_DIR)/$(BIN)
	$(PYTHON) -m pip install -r $(REQS)
	$(PYTHON) -m pytest -v

-include $(DEPS)
