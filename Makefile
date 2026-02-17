CC            ?= gcc
CARGO         ?= cargo
CBINDS        ?= cbindgen
FORMAT        ?= clang-format
TIDY          ?= clang-tidy
CPPCHECK      ?= cppcheck

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

INC_DIRS      := config include
LIB_DIR       := vendor

CFLAGS        := -O3 -march=native -std=c2x
CPPFLAGS      := $(foreach d, $(INC_DIRS), -I$(d)) $(foreach d, $(LIB_DIR), -isystem $(d)) $(shell pkg-config --cflags sdl3)
LDLIBS        := $(shell pkg-config --libs sdl3)
VERIFY_FLAGS  := -Wall -Wextra -Wpedantic -Werror

C_SRCS        := $(shell find $(SRC_DIR) -name "*.c")
C_HDRS        := $(shell find include/gbemu/ -name "*.h")
RUST_SRCS     := $(shell find $(RUST_DIR)/src -name "*.rs")
OBJS          := $(C_SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS          := $(OBJS:.o=.d)

.PHONY: all clean format test verify

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
	rm -rf $(BUILD_DIR) $(RUST_HDR) $(VENV)
	$(CARGO) clean --manifest-path $(RUST_MANIFEST)

format:
	$(FORMAT) -i $(C_SRCS) $(C_HDRS)

$(PYTHON):
	python3 -m venv $(VENV)
	$(VENV)/bin/python3 -m ensurepip --default-pip
	$(PIP) install --upgrade pip
	$(PYTHON) -m pip install -r $(REQS)

test: $(PYTHON) $(BUILD_DIR)/$(BIN)
	$(PYTHON) -m pytest -v -n auto

verify: clean $(RUST_HDR)
	$(FORMAT) --dry-run -Werror $(C_SRCS) $(C_HDRS)
	$(CARGO) fmt --check --manifest-path $(RUST_MANIFEST)
	$(CARGO) clippy --manifest-path $(RUST_MANIFEST) -- -D warnings # -D clippy::pedantic
# $(TIDY) $(C_SRCS) -header-filter='.*' --checks='*' --warnings-as-errors='*' -- $(CFLAGS) $(CPPFLAGS)
# $(CPPCHECK) --enable=all --inconclusive --error-exitcode=1 $(SRC_DIR) include/
	$(MAKE) $(BUILD_DIR)/$(BIN) CFLAGS="$(CFLAGS) $(VERIFY_FLAGS)" LDLIBS="$(LDLIBS) $(VERIFY_FLAGS)"
	$(MAKE) test

-include $(DEPS)
