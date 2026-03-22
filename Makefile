CC            ?= gcc
PYTHON3       ?= python3
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

INC_DIRS      := include
LIB_DIR       := vendor
SDL_DIR       := sdl
HEADLESS_DIR  := headless

VERSION       := 1.0.0

CFLAGS        := -O2 -std=c2x
CPPFLAGS      := $(foreach d, $(INC_DIRS), -I$(d)) $(foreach d, $(LIB_DIR), -isystem $(d))
SDL_CFLAGS    := $(shell pkg-config --cflags sdl3)
SDL_LDLIBS    := $(shell pkg-config --libs sdl3)
LDFLAGS       :=
VERIFY_FLAGS  := -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion \
                 -Wno-unused-parameter -Wno-sign-conversion

ifdef DEBUG
  CFLAGS += -O0 -g -DDEBUG
endif
ifdef ASAN
  CFLAGS  += -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
endif

CORE_SRCS     := $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c)
VENDOR_SRCS   := $(wildcard $(LIB_DIR)/*.c)
SDL_SRCS      := $(wildcard $(SDL_DIR)/*.c)
HEADLESS_SRC  := $(HEADLESS_DIR)/main.c

C_HDRS        := $(wildcard include/gbemu/*.h)
RUST_SRCS     := $(wildcard $(RUST_DIR)/src/*.rs $(RUST_DIR)/src/*/*.rs)

CORE_OBJS     := $(CORE_SRCS:%.c=$(BUILD_DIR)/%.o)
VENDOR_OBJS   := $(VENDOR_SRCS:%.c=$(BUILD_DIR)/%.o)
SDL_OBJS      := $(SDL_SRCS:%.c=$(BUILD_DIR)/%.o)
HEADLESS_OBJ  := $(HEADLESS_SRC:%.c=$(BUILD_DIR)/%.o)

ALL_OBJS      := $(CORE_OBJS) $(VENDOR_OBJS) $(SDL_OBJS) $(HEADLESS_OBJ)
DEPS          := $(ALL_OBJS:.o=.d)

.PHONY: all gbemu gbemu_headless clean format test verify compile_commands

all: gbemu gbemu_headless $(PYTHON)

gbemu: $(BUILD_DIR)/gbemu
gbemu_headless: $(BUILD_DIR)/gbemu_headless

$(BUILD_DIR)/gbemu: $(RUST_LIB) $(CORE_OBJS) $(VENDOR_OBJS) $(SDL_OBJS)
	$(CC) $(CORE_OBJS) $(VENDOR_OBJS) $(SDL_OBJS) $(RUST_LIB) $(LDFLAGS) -o $@ $(SDL_LDLIBS)

$(BUILD_DIR)/gbemu_headless: $(RUST_LIB) $(CORE_OBJS) $(VENDOR_OBJS) $(HEADLESS_OBJ)
	$(CC) $(CORE_OBJS) $(VENDOR_OBJS) $(HEADLESS_OBJ) $(RUST_LIB) $(LDFLAGS) -o $@

$(CORE_OBJS) $(VENDOR_OBJS) $(HEADLESS_OBJ): $(RUST_HDR)
$(SDL_OBJS): $(RUST_HDR)
$(RUST_HDR): $(RUST_LIB)

$(BUILD_DIR)/$(SRC_DIR)/sdl/%.o: $(SRC_DIR)/sdl/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -O2 -std=c2x $(CPPFLAGS) -MMD -MP -c $< -o $@

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
	$(PYTHON3) -m venv $(VENV)
	$(PIP) install --upgrade pip
	$(PYTHON) -m pip install -r $(REQS)

test: $(PYTHON) $(BUILD_DIR)/gbemu_headless
	$(PYTHON) -m pytest

verify: clean $(RUST_HDR) $(PYTHON)
	$(FORMAT) --dry-run -Werror $(CORE_SRCS) $(SDL_SRCS) $(HEADLESS_SRC) $(C_HDRS)
	$(CARGO) fmt --check --manifest-path $(RUST_MANIFEST)
	$(CARGO) clippy --manifest-path $(RUST_MANIFEST) -- -D warnings # -D clippy::pedantic
# $(TIDY) $(CORE_SRCS) -header-filter='.*' --checks='*' --warnings-as-errors='*' -- $(CFLAGS) $(BASE_CPPFLAGS)
# $(CPPCHECK) --enable=all --inconclusive --error-exitcode=1 $(SRC_DIR) include/
	$(MAKE) all CFLAGS="$(CFLAGS) $(VERIFY_FLAGS)"
	$(PYTHON) tests/check_coverage.py
	$(MAKE) test ASAN=1

compile_commands: clean
	bear -- $(MAKE) all

-include $(DEPS)
