# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

```bash
make all              # Build both SDL GUI (build/gbemu) and headless (build/gbemu_headless)
make gbemu            # Build SDL GUI only
make gbemu_headless   # Build headless binary only
make test             # Build, create Python venv, and run full pytest suite
make format           # Auto-format all C sources with clang-format
make verify           # Full CI check: format, cargo fmt, cargo clippy, strict build, tests
make clean            # Remove all build artifacts
```

Running a single test file:

```bash
make gbemu_headless && .venv/bin/python3 -m pytest tests/blargg_test.py -v
```

Running a single test by name:

```bash
.venv/bin/python3 -m pytest tests/blargg_test.py::test_cpu_instrs -v
```

Running the emulator:

```bash
build/gbemu [-r <rom.gb>] [-d]
build/gbemu_headless -r <rom.gb> -t <category> [-b <boot.rom>] [-s screenshot.png] [-d] [-w <hex_addr>]
build/gbemu_headless --version
```

**Boot ROM:** Defaults to `boot.rom` in the working directory. Configurable via `-b`/`--boot-rom` (headless) or `settings.toml` (SDL). Passing an empty string skips the boot ROM and applies post-boot CPU state directly.

## Architecture

The emulator is split into a **C core** and a **Rust MMU**, with two front-ends (SDL3 GUI and headless for testing).

### C/Rust split

- **Rust** (`rust/`): The MMU — all memory reads/writes, MBC mapper logic, RTC, and save/load. Compiled as a static library (`librust.a`). The public API is exported via `extern "C"` and auto-generated into `include/rust.h` by cbindgen.
- **C** (`src/`): CPU, PPU, APU, settings, Pokémon editor, and the top-level `gbemu` struct. All C components access memory exclusively through the Rust MMU functions (`mmu_r_mem`, `mmu_w_mem`, etc.).

### Main emulation loop

`gbemu_step_frame()` in `src/core/gbemu.c` runs one full frame (154 scanlines × 456 cycles). Each iteration:

1. `step()` — execute one CPU instruction, returns cycles consumed
1. `mmu_advance_rtc()` — advance the MBC3 real-time clock by those cycles
1. Accumulate cycles into `ppu->scn`; call `do_scanline()` every 456 cycles
1. Handle LCDC bit-7 re-enable (resets scanline counter to 4)
1. `update_lcd()` — update LCDC STAT mode bits, using `cyc - cpu->cyc_ext` to avoid double-counting interrupt dispatch cycles
1. `update_timer()` — advance DIV and TIMA; tick the APU div counter
1. `check_dma()` — handle OAM DMA
1. `upd_apu()` — advance APU channel state machines
1. `check_interrupt()` — fire pending interrupts if IME set; if an interrupt dispatches, runs a second pass of scanline/lcd/timer/apu for the dispatch cycles

### Key structs

**`gbemu`** (`include/gbemu/gbemu.h`) — top-level:

- `Apu* apu`, `Cpu* cpu`, `Mmu* mmu`, `Ppu* ppu`
- `char* rom_name`, `const char* boot_rom`, `int test_category`
- `uint8_t disassemble_enable`, `bdone`, `paused`, `fast_forward`
- `uint16_t watch_addrs[8]`, `uint8_t watch_count`
- `uint64_t total_cycles`, `total_frames`

**`Cpu`** (`include/gbemu/cpu.h`):

- Registers: `A B C D E F H L`, `PC`, `SP`
- Flags: `bHALT`, `bIME`, `bIME_pending`, `bHALT_BUG`, `tima_overflow_pending`
- Timer internals: `tim_cnt`, `tim_thresh`, `div_cnt`
- `uint16_t intr_loc[5]` — interrupt vector addresses
- `uint8_t cyc_ext` — extra cycles from interrupt dispatch (subtracted before timer/APU update)
- `uint8_t cgb_mode`

**`Ppu`** (`include/gbemu/ppu.h`):

- `uint8_t dsp[144][160]` — DMG framebuffer (color indices 0–3)
- `uint16_t cgb_dsp[144][160]` — CGB framebuffer (RGB555)
- `uint16_t scn` — scanline cycle counter, `uint8_t frame` — set at frame end
- `uint8_t WIN_CNT`, `off_scn`, `cgb_mode`
- `int in[8]` — button state (BTN_A=0 … BTN_RIGHT=7)

**`Apu`** (`include/gbemu/apu.h`): 4-channel state, `div_apu` counter, 1024-sample output buffer at 48 kHz, high-pass filter capacitor values.

**`Settings`** (`include/gbemu/settings.h`) — runtime configuration, global instance `g_settings`:

- `int scale`, `uint32_t dmg_palette[4]`, `bool fullscreen`, `float volume`
- Per-button key name strings (`key_a`, `key_b`, …, `key_pause`, `key_screenshot`)
- `char boot_rom[512]`, `char last_rom[512]`
- `char recent_roms[10][512]`, `int recent_rom_count`
- Persisted to `~/.config/gbemu/settings.toml` (Linux) or `%APPDATA%\gbemu\settings.toml` (Windows)

### SDL front-end (`sdl/`)

- **Idle UI** (Nuklear): centered dialog with "Open ROM..." button and a recent-ROMs list (up to 10 entries, filename only)
- **ROM loading**: via Ctrl+O / "Open ROM..." (native file dialog filtered to `.gb`/`.gbc`), CLI `-r` flag, or recent-ROMs list; recent list and `last_rom` are saved to settings on each load
- **Hardcoded keys**: Tab (fast-forward, hold), F11 (fullscreen toggle), Ctrl+O (open ROM dialog), Ctrl+R (reset)
- **Configurable keys** (from settings): A, B, Start, Select, Up, Down, Left, Right, Pause
- **Gamepad**: SDL3 gamepad API — South=A, East=B, Start=Start, Back=Select, D-pad=D-pad
- **Fast-forward**: runs 4 frames per render tick while Tab is held (`fast_forward` flag on `gbemu`)
- **Settings**: loaded on startup (`settings_load`), saved on ROM load and on quit

### CGB (Game Boy Color) support

CGB mode is detected from ROM header byte `0x0143` (`0x80` = CGB-compatible, `0xC0` = CGB-only). When active:

- `cpu->cgb_mode` and `ppu->cgb_mode` are both set to 1
- PPU renders to `cgb_dsp` (RGB555) instead of `dsp` (index 0–3)
- SDL renderer and headless screenshot both convert RGB555 → RGB888
- MMU supports VRAM bank switching (2 banks), WRAM bank switching (8 banks), background/object palette RAM (64 bytes each)
- CGB boot ROM is `0x900` bytes; DMG boot ROM is `0x100` bytes

### Supported mappers

All implemented in `rust/src/mmu/`:

- **No MBC** — flat ROM
- **MBC1** — up to 2 MB ROM, 32 KB RAM, multicart detection
- **MBC2** — 256 KB ROM, 512-byte internal RAM
- **MBC3** — up to 2 MB ROM, 32 KB RAM, full RTC (seconds/minutes/hours/days, carry, halt, latching)
- **MBC5** — up to 8 MB ROM, 128 KB RAM

### Save files

Saved as `<rom-title>.sav` in the working directory (`rust/src/mmu/io.rs`). Loaded on `gbemu_init`, saved on `gbemu_free` and `gbemu_reset`.

### Pokémon save editor (`src/pokemon/`)

Experimental feature in `src/pokemon/` (`pokemon.c`, `util.c`). Activated automatically when the ROM title matches (`pokemon_enabled` flag). Provides GB string encoding/decoding, checksum calculation, and a character table. Initialized via `p_init_data()` in the SDL front-end after ROM load.

## Testing framework

All tests use `build/gbemu_headless`. Two mechanisms in `tests/core.py`:

- **`check_stream(rom, category)`** — monitors stdout for "Passed"/"Failed"
- **`check_screenshot(rom, ref_path, category)`** — runs with `-s`, pixel-compares against PNG reference using Pillow

**Test categories** (`-t` flag) and their current status:

| Category                                 | Suite                                           | Status                 |
| ---------------------------------------- | ----------------------------------------------- | ---------------------- |
| `blargg_cpu`                             | Blargg cpu_instrs                               | active                 |
| `blargg_cpu_time`                        | Blargg instr_timing                             | active                 |
| `blargg_mem_time`                        | Blargg mem_timing                               | active                 |
| `blargg_audio`                           | Blargg dmg_sound                                | skipped                |
| `blargg_halt_bug`                        | Blargg halt_bug                                 | skipped                |
| `blargg_interrupt_time`                  | Blargg interrupt_time                           | skipped                |
| `blargg_mem_time2`                       | Blargg mem_timing-2                             | skipped                |
| `blargg_oam_bug`                         | Blargg oam_bug                                  | skipped                |
| `blargg_cgb_sound`                       | Blargg cgb_sound                                | skipped                |
| `acid2`                                  | DMG Acid2 + CGB Acid2 (screenshot)              | active                 |
| `mbc3`                                   | mbc3-tester (screenshot)                        | active                 |
| `rtc3_basic` / `rtc3_range` / `rtc3_sub` | rtc3test (screenshot, simulates button presses) | active                 |
| `gambatte`                               | Gambatte (screenshot)                           | skipped — refs missing |
| `micro`                                  | gbmicrotest (reads `0xFF82`)                    | skipped                |
| `little`                                 | little-things-gb (screenshot)                   | skipped                |
| `mealybug`                               | mealybug-tearoom (screenshot)                   | skipped                |
| `mooneye`                                | Mooneye test suite                              | skipped                |
| `same`                                   | same-suite                                      | skipped                |
| `age`                                    | age-test-roms                                   | skipped                |
| `bully`                                  | bully                                           | skipped                |
| `scribble`                               | scribbltests                                    | skipped                |
| `strike`                                 | strikethrough                                   | skipped                |
| `turtle`                                 | turtle-tests                                    | skipped                |

The RTC3 categories simulate button presses by writing directly to `ppu->in[BTN_DOWN]` / `ppu->in[BTN_A]` at specific frame offsets after the boot ROM finishes.

## Debugging Test ROMs

### Test ROM source code

Source for test suites is available locally:

- Blargg: `test_roms/blargg/<suite>/source/`
- Mooneye: `mooneye/`
- Age: `age/src/`

Always read the source before touching emulator code — helper routines often do more than their names suggest.

### Pan Docs reference

A local copy of the [Pan Docs](https://gbdev.io/pandocs/) GB technical reference is in `pandocs/src/`. Read the relevant `.md` file there rather than fetching the web version. Key files: `CPU_Instruction_Set.md`, `Interrupts.md`, `LCDC.md`, `Graphics.md`, `Audio.md`, `Timer_and_Divider_Registers.md`, `MBC1.md`, `MBC3.md`, `halt.md`.

### Sameboy Source Code

As a last resort, the source code for the Sameboy emulator can be found in `Sameboy/`. Read the relevant files to see the correct implementation.

### Headless flags useful for debugging

```bash
build/gbemu_headless -r <rom.gb> -t <category> -d          # per-instruction disassembly to stdout
build/gbemu_headless -r <rom.gb> -t <category> -w FF80     # print value at 0xFF80 on every change
build/gbemu_headless -r <rom.gb> -t <category> -s out.png  # screenshot at end of run
```

Up to 8 `-w` addresses can be specified simultaneously.

### scripts/trace.py — execution tracer

Wraps the headless binary with `-d` and provides filtering over the disassembly stream.

```bash
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye --last 50        # last N instructions before pass/fail
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye --grep "CALL"    # filter by regex
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye --around FF80    # 20 lines before/after each hit
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye --limit 100000   # stop after N instructions
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye --watch FF80     # forwarded to headless -w
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye --max-hits 5     # cap hits per --around address
# Options can be combined:
.venv/bin/python3 scripts/trace.py <rom.gb> --test mooneye --last 100 --watch FF44 --limit 500000
```

### scripts/rom_info.py — ROM header inspector

```bash
.venv/bin/python3 scripts/rom_info.py <rom.gb>
```

Prints title, CGB flag, mapper type, ROM/RAM sizes, destination, and checksums.

### scripts/disassembler.py — static disassembler

```bash
.venv/bin/python3 scripts/disassembler.py <rom.gb>
.venv/bin/python3 scripts/disassembler.py <rom.gb> | grep "CALL"   # find all subroutine calls
.venv/bin/python3 scripts/disassembler.py <rom.gb> | grep "FF0F"   # find IF register accesses
```

Linear-sweep over the full ROM. Note: Blargg ROMs copy themselves to WRAM at `$C000` — map ROM→runtime via subtract `$4000`, add `$C000`. Mooneye ROMs run directly from ROM.

### scripts/bank_dump.py — ROM bank dumper

```bash
.venv/bin/python3 scripts/bank_dump.py <rom.gb>                             # one byte per bank at offset 0
.venv/bin/python3 scripts/bank_dump.py <rom.gb> --addr 4000 --count 4      # 4 bytes at GB addr $4000, per bank
.venv/bin/python3 scripts/bank_dump.py <rom.gb> --raw 0 FF                 # raw ROM bytes $0000–$00FF
```

Useful for debugging MBC bank-switching behavior.

### scripts/check_coverage.py — test coverage checker

```bash
.venv/bin/python3 scripts/check_coverage.py
```

Scans `test_roms/` and reports which ROMs have no pytest coverage. Exits 1 if any are truly uncovered (excludes known-excluded suites and gambatte ROMs without reference images).

### C-level disassembly

`src/cpu/disassembler.c` is compiled into both binaries. Activated by `-d`, it prints each instruction as it executes with the current PC.

## Debugging Strategy

When a test ROM fails, work through these steps in order:

### Step 1: Read the test source and understand what it checks

Find the source for the failing test and read it carefully. Identify what registers or memory values are asserted, and what setup the test does before the assertion.

### Step 2: Disassemble and trace to understand real execution

**Static disassembler first.** The source alone can be misleading — helper routines often have side effects that aren't obvious from their names. Disassemble the ROM and grep for the registers or addresses relevant to the failure.

Then run an execution trace to see the actual instruction stream:

```bash
.venv/bin/python3 scripts/trace.py <rom.gb> --test <category> --last 100
```

Use `--around <addr>` to focus on a specific subroutine, and `--watch <addr>` to track when a register changes value.

### Step 3: Instrument the emulator with fprintf

When the trace alone isn't enough, add `fprintf(stderr, ...)` calls in the relevant C source (`src/cpu/step.c`, `src/ppu/ppu.c`, `src/apu/apu.c`, etc.) to print internal state at key moments:

```bash
./build/gbemu_headless -r <rom.gb> -t <category> 2>&1
```

Remember to add `#include <stdio.h>` if needed, and remove all debug prints before committing.

### Step 4: Check your mental model against the hardware spec

Before concluding the emulator is wrong, verify your understanding in `pandocs/src/`. Common traps:

- **`mmu_r_mem` vs `mmu_r_mem_raw`**: `mmu_r_mem` applies hardware read masks (what the CPU sees); `mmu_r_mem_raw` returns the raw stored byte.
- **`mmu_w_mem` vs `mmu_w_mem_raw`**: `mmu_w_mem` goes through all write side-effects (APU gating, DIV-reset logic, etc.); `mmu_w_mem_raw` bypasses all of it.
- **`cyc_ext`**: cycles consumed by interrupt dispatch are stored here and subtracted before `update_timer`/`upd_apu` to avoid double-counting.
- **`upd_apu` is called once per CPU instruction**, not per clock cycle.
- **Register read masks**: Many hardware registers OR certain bits with 1 on read — `mmu_r_mem` applies these, `mmu_r_mem_raw` does not.

### Step 5: Distinguish "emulator not doing what I want" vs "what I wanted was wrong"

Before making a second attempt at a fix, explicitly ask: is the emulator failing to do what I implemented, or did I implement the wrong thing? Use `fprintf` debug to verify what the emulator is actually doing, then compare against the pandocs spec. Fixing the wrong model wastes attempts.
