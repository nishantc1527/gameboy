VERSION        := 1.0.0

CC             ?= gcc
CXX            ?= g++
PYTHON3        ?= python3
CARGO          ?= cargo
CBINDS         ?= cbindgen
FORMAT         ?= clang-format
TIDY           ?= clang-tidy
CPPCHECK       ?= cppcheck
COMPDB         ?= bear

VENV           := .venv
PYTHON         := $(VENV)/bin/python3
PIP            := $(VENV)/bin/pip3
REQS           := requirements.txt

POKERED_DIR    := pokered
POKERED_ROM    := $(POKERED_DIR)/pokered.gbc
ALL_REFS       := $(shell python3 scripts/gen_boot_refs.py --list)

BUILD_DIR      := build
SRC_DIR        := src
INC_DIR        := include
LIB_DIR        := vendor
SDL_DIR        := sdl
HEADLESS_DIR   := headless

RUST_DIR       := rust
RUST_LIB       := $(RUST_DIR)/target/release/librust.a
RUST_HDR       := $(INC_DIR)/rust.h
RUST_MANIFEST  := $(RUST_DIR)/Cargo.toml
RUST_CRATE     := rust

BOOTROMS_DIR   := gb-bootroms
DMG_BIN        := $(BOOTROMS_DIR)/bin/dmg.bin
CGB_BIN        := $(BOOTROMS_DIR)/bin/cgb.bin
BOOT_ROMS_SRC  := $(SRC_DIR)/boot_roms.c

CFLAGS         := -O2 -std=c2x
CPPFLAGS       := $(foreach d,$(INC_DIR),-I$(d)) $(foreach d,$(LIB_DIR),-isystem $(d))
SDL_CFLAGS     := $(shell pkg-config --cflags sdl3)
SDL_LDLIBS     := $(shell pkg-config --libs sdl3)
CXXFLAGS       := -O2 -std=c++17 
LDFLAGS        :=

CORE_SRCS      := $(filter-out $(BOOT_ROMS_SRC),$(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c))
CORE_HDRS      := $(wildcard include/gbemu/*.h)
VENDOR_SRCS    := $(wildcard $(LIB_DIR)/*.c)
VENDOR_CXXSRCS := $(wildcard $(LIB_DIR)/*.cpp)
SDL_SRCS       := $(wildcard $(SDL_DIR)/*.c)
SDL_CXXSRCS    := $(wildcard $(SDL_DIR)/*.cpp)
HEADLESS_SRCS  := $(wildcard $(HEADLESS_DIR)/*.c)
RUST_SRCS      := $(wildcard $(RUST_DIR)/src/*.rs $(RUST_DIR)/src/*/*.rs)

CORE_OBJS      := $(CORE_SRCS:%.c=$(BUILD_DIR)/%.o)
VENDOR_OBJS    := $(VENDOR_SRCS:%.c=$(BUILD_DIR)/%.o)
VENDOR_CXXOBJS := $(VENDOR_CXXSRCS:%.cpp=$(BUILD_DIR)/%.o)
SDL_OBJS       := $(SDL_SRCS:%.c=$(BUILD_DIR)/%.o) $(SDL_CXXSRCS:%.cpp=$(BUILD_DIR)/%.o)
HEADLESS_OBJ   := $(HEADLESS_SRCS:%.c=$(BUILD_DIR)/%.o)
BOOT_ROMS_OBJ  := $(BOOT_ROMS_SRC:%.c=$(BUILD_DIR)/%.o)

ALL_OBJS       := $(CORE_OBJS) $(VENDOR_OBJS) $(VENDOR_CXXOBJS) $(SDL_OBJS) $(HEADLESS_OBJ) $(BOOT_ROMS_OBJ)
DEPS           := $(ALL_OBJS:.o=.d)

.PHONY: all gbemu gbemu_headless clean format test verify lsp

all: gbemu gbemu_headless

gbemu: $(BUILD_DIR)/gbemu
gbemu_headless: $(BUILD_DIR)/gbemu_headless

$(BUILD_DIR)/gbemu: $(RUST_LIB) $(CORE_OBJS) $(VENDOR_OBJS) $(VENDOR_CXXOBJS) $(SDL_OBJS) $(BOOT_ROMS_OBJ)
	$(CXX) $(CORE_OBJS) $(VENDOR_OBJS) $(VENDOR_CXXOBJS) $(SDL_OBJS) $(BOOT_ROMS_OBJ) $(RUST_LIB) $(LDFLAGS) -o $@ $(SDL_LDLIBS)

$(BUILD_DIR)/gbemu_headless: $(RUST_LIB) $(CORE_OBJS) $(VENDOR_OBJS) $(HEADLESS_OBJ) $(BOOT_ROMS_OBJ)
	$(CC) $(CORE_OBJS) $(VENDOR_OBJS) $(HEADLESS_OBJ) $(BOOT_ROMS_OBJ) $(RUST_LIB) $(LDFLAGS) -o $@

$(CORE_OBJS) $(VENDOR_OBJS) $(HEADLESS_OBJ): $(RUST_HDR)
$(SDL_OBJS): $(RUST_HDR)
$(BOOT_ROMS_OBJ): $(BOOT_ROMS_SRC)
$(RUST_HDR): $(RUST_LIB)

$(BUILD_DIR)/$(SRC_DIR)/sdl/%.o: $(SRC_DIR)/sdl/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(RUST_LIB): $(RUST_MANIFEST) $(RUST_SRCS)
	$(CARGO) build --manifest-path $(RUST_MANIFEST) --release
	$(CBINDS) $(RUST_DIR) --crate $(RUST_CRATE) --output $(RUST_HDR)

clean:
	rm -rf $(BUILD_DIR) $(RUST_HDR) $(VENV) $(BOOT_ROMS_SRC)
	$(CARGO) clean --manifest-path $(RUST_MANIFEST)
	$(MAKE) -C $(BOOTROMS_DIR) clean

format:
	$(FORMAT) -i $(CORE_SRCS) $(SDL_SRCS) $(HEADLESS_SRCS) $(CORE_HDRS)
	$(CARGO) fmt --manifest-path rust/Cargo.toml

$(PYTHON):
	$(PYTHON3) -m venv $(VENV)
	$(PIP) install --upgrade pip
	$(PYTHON) -m pip install -r $(REQS)

$(ALL_REFS) &: | $(PYTHON) $(BUILD_DIR)/gbemu_headless $(POKERED_ROM)
	$(PYTHON) scripts/gen_boot_refs.py

$(DMG_BIN) $(CGB_BIN):
	$(MAKE) -C $(BOOTROMS_DIR)

$(BOOT_ROMS_SRC): $(DMG_BIN) $(CGB_BIN)
	$(PYTHON3) scripts/gen_boot_roms.py

$(POKERED_ROM):
	$(MAKE) -C $(POKERED_DIR) red

test: $(PYTHON) $(BUILD_DIR)/gbemu_headless $(ALL_REFS)
	$(PYTHON) -m pytest

verify: gbemu_headless
	$(FORMAT) --dry-run -Werror $(CORE_SRCS) $(SDL_SRCS) $(HEADLESS_SRCS) $(CORE_HDRS)
	$(CARGO) fmt --check --manifest-path $(RUST_MANIFEST)
	$(CARGO) clippy --manifest-path $(RUST_MANIFEST) -- -D warnings
	# $(TIDY) -p . $(CORE_SRCS) $(SDL_SRCS) $(HEADLESS_SRCS)
# $(CPPCHECK)
	$(MAKE) test

lsp: clean
	$(COMPDB) -- $(MAKE) all

-include $(DEPS)
