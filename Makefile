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

BUILD_DIR     := build
SRC_DIR       := src

INC_DIRS      := config include
LIB_DIR       := vendor
SDL_DIR       := sdl
HEADLESS_DIR  := headless

CFLAGS        := -O2 -std=c2x
CPPFLAGS      := $(foreach d, $(INC_DIRS), -I$(d)) $(foreach d, $(LIB_DIR), -isystem $(d))
SDL_CFLAGS    := $(shell pkg-config --cflags sdl3)
SDL_LDLIBS    := $(shell pkg-config --libs sdl3)
VERIFY_FLAGS  := -Wall -Wextra -Wpedantic -Werror

CORE_SRCS     := $(shell find $(SRC_DIR) -name "*.c")
SDL_SRCS      := $(shell find $(SDL_DIR) -name "*.c")
HEADLESS_SRC  := $(HEADLESS_DIR)/main.c

C_HDRS        := $(shell find include/gbemu/ -name "*.h")
RUST_SRCS     := $(shell find $(RUST_DIR)/src -name "*.rs")

CORE_OBJS     := $(CORE_SRCS:%.c=$(BUILD_DIR)/%.o)
SDL_OBJS      := $(SDL_SRCS:%.c=$(BUILD_DIR)/%.o)
HEADLESS_OBJ  := $(HEADLESS_SRC:%.c=$(BUILD_DIR)/%.o)

ALL_OBJS      := $(CORE_OBJS) $(SDL_OBJS) $(HEADLESS_OBJ)
DEPS          := $(ALL_OBJS:.o=.d)

.PHONY: all gbemu gbemu_headless clean format test verify

all: gbemu gbemu_headless $(PYTHON)

gbemu: $(BUILD_DIR)/gbemu
gbemu_headless: $(BUILD_DIR)/gbemu_headless

$(BUILD_DIR)/gbemu: $(RUST_LIB) $(CORE_OBJS) $(SDL_OBJS)
	$(CC) $(CORE_OBJS) $(SDL_OBJS) $(RUST_LIB) -o $@ $(SDL_LDLIBS)

$(BUILD_DIR)/gbemu_headless: $(RUST_LIB) $(CORE_OBJS) $(HEADLESS_OBJ)
	$(CC) $(CORE_OBJS) $(HEADLESS_OBJ) $(RUST_LIB) -o $@

$(CORE_OBJS) $(HEADLESS_OBJ): $(RUST_HDR)
$(SDL_OBJS): $(RUST_HDR)
$(RUST_HDR): $(RUST_LIB)

$(BUILD_DIR)/$(SRC_DIR)/sdl/%.o: $(SRC_DIR)/sdl/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(RUST_LIB): $(RUST_MANIFEST) $(RUST_SRCS)
	$(CARGO) build --manifest-path $(RUST_MANIFEST) --release
	$(CBINDS) $(RUST_DIR) --crate $(RUST_CRATE) --output $(RUST_HDR)

clean:
	rm -rf $(BUILD_DIR) $(RUST_HDR) $(VENV)
	$(CARGO) clean --manifest-path $(RUST_MANIFEST)

format:
	$(FORMAT) -i $(CORE_SRCS) $(SDL_SRCS) $(HEADLESS_SRC) $(C_HDRS)

$(PYTHON):
	python3 -m venv $(VENV)
	$(VENV)/bin/python3 -m ensurepip --default-pip
	$(PIP) install --upgrade pip
	$(PYTHON) -m pip install -r $(REQS)

test: $(PYTHON) $(BUILD_DIR)/gbemu_headless
	$(PYTHON) -m pytest

verify: clean $(RUST_HDR)
	$(FORMAT) --dry-run -Werror $(CORE_SRCS) $(SDL_SRCS) $(HEADLESS_SRC) $(C_HDRS)
	$(CARGO) fmt --check --manifest-path $(RUST_MANIFEST)
	$(CARGO) clippy --manifest-path $(RUST_MANIFEST) -- -D warnings # -D clippy::pedantic
# $(TIDY) $(CORE_SRCS) -header-filter='.*' --checks='*' --warnings-as-errors='*' -- $(CFLAGS) $(BASE_CPPFLAGS)
# $(CPPCHECK) --enable=all --inconclusive --error-exitcode=1 $(SRC_DIR) include/
	$(MAKE) gbemu_headless CFLAGS="$(CFLAGS) $(VERIFY_FLAGS)"
	$(MAKE) test

-include $(DEPS)
