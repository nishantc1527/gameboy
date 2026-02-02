![Build Status](https://github.com/nishantc1527/fractl/actions/workflows/ci.yml/badge.svg)

# Gameboy Emulator

A cycle-accurate emulator for the [Nintendo Gameboy](https://en.wikipedia.org/wiki/Game_Boy) that can play most commercial games (such as Pokemon and Tetris).

* Emulator core written in C, with Rust implementing the memory management unit via [FFI](https://en.wikipedia.org/wiki/Foreign_function_interface).
* Uses the [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) library for windowing and rendering.
* Python3 + [pytest](https://docs.pytest.org/en/stable/) for automated testing + continuous integration.
* Built with [GNU Make](https://www.gnu.org/software/make/).

# Demos

Pokemon Red:

![Alt](assets/pokemon_demo.gif)

Tetris:

![Alt](assets/tetris.gif)

View more demos in the [assets](assets/) folder.

# Motivation

I made this project to learn more about systems concepts such as CPU emulation, virtual memory management, scanline-based rendering, and hardware interrupts. However, the emulator still prioritizes accuracy and user experience. There is a CLI interface, customizable keyboard controls and display options, and there is an automated testing pipeline to ensure accuracy throughout development.

# Features

## Mappers

* [No mapper](https://gbhwdb.gekkio.fi/cartridges/no-mapper.html)
* [MBC1](https://gbhwdb.gekkio.fi/cartridges/mbc1.html)
* [MBC3](https://gbhwdb.gekkio.fi/cartridges/mbc3.html)

## CPU & Interrupts

Passes all of [Blargg's test ROMs](https://github.com/retrio/gb-test-roms) (excluding CGB and sound tests). This means that all CPU instructions are implemented and functional with cycle-accurate timing, and interrupts all work as intended.

[CPU Instr](https://github.com/retrio/gb-test-roms/tree/master/cpu_instrs):

![Alt](assets/cpu_instr.png)

[Instr Timing](https://github.com/retrio/gb-test-roms/tree/master/instr_timing):

![Alt](assets/instr_timing.png)

## Display

Passes [DMG Acid 2](https://github.com/mattcurrie/dmg-acid2), which tests the display. This means that the PPU is working correctly and is fully functional.

[DMG Acid 2](https://github.com/mattcurrie/dmg-acid2):

![Alt](assets/dmg-acid2.png)

## Boot ROM

Supports boot ROM functionality, meaning whenever the emulator is turned on it loads the boot ROM at memory location 0x0000 and starts from there.

![Alt](assets/boot_rom.gif)

## Automated Testing

This emulator supports a headless mode to not spawn a window (`-h or --headless`). It also includes an option to tell it that it's running a specific kind of test ROM (eg. `-t or --test blargg`). It will then watch the test and output the result when finished (pass or fail).

Using this interface, there are several python testing scripts in the [tests](tests/) directory that batch run multiple test ROM suites using the headless mode and verify the output using [pytest](https://docs.pytest.org/en/stable/). These tests have been integrated into the repository's continuous integration workflow to catch regressions and breaking changes.

## Pokemon Save File Patching (Gen I)

Whenever Pokemon Red or Blue is booted up, it runs a checksum verification to ensure the save file hasn't been messed with. This emulator automatically sets the correct checksum so that you can edit the save file without worrying about corrupting the game. This feature is automatically turned on if Pokemon Red or Blue is detected.

If you want to learn more about the save file structure, [here](https://bulbapedia.bulbagarden.net/wiki/Save_data_structure_(Generation_I)) is a good reference file.

## Customization

Customization is done through the config files in the [config](config/) directory.

For colors, use the macros called `HEX_WHT` through `HEX_BLK` in the [colors.h](config/colors.h) file. You can change the hex code of each tone (white, light grey, dark grey, and black).

For screen size, you can change the variables `SCALE_X` and `SCALE_Y` in the [display.h](config/display.h) file, which scale the X and Y axes. 

For controls, use the macros KEY_[button name] in the [controls.h](config/controls.h). For example, to set a custom key to press the A button, the variable is called KEY_A. You can use [this file](https://wiki.libsdl.org/SDL3/SDL_Keycode) to find the corresponding keycode for each key on your keyboard.

## Roadmap

These features are planned or in active development.

* Audio
* Basic UI
* More memory bank controllers

# Installing

NOTE: On Windows, it would be easiest to use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install). Using Cygwin or Mingw-w64 is possible, but you'll need to configure the rustup toolchain and manually download a lot of software not available through their package managers.

Here's an example of setting everything up on Ubuntu:

```shell
sudo apt update
sudo apt install -y build-essential python3 make cmake git pkg-config
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh # install rust toolchain
cargo install cbindgen # install cbindgen
git clone https://github.com/libsdl-org/SDL.git # some apt repositories don't have sdl3, so install and build it from source
cd SDL
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

Here are the step by step instructions. 

1. Install the latest stable release of [SDL3](https://github.com/libsdl-org/SDL/releases) for your operating system. For some systems, you may need to [build it from source](https://github.com/libsdl-org/SDL/blob/main/docs/INTRO-cmake.md#configure-and-build). Install [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) and make sure it can see `SDL3` (run `pkg-config --cflags sdl3` to verify).
2. Install a rust toolchain. The easiest way to do this is through [rustup](https://rustup.rs/). If you're on Windows (and not on WSL), make sure to use the `stable-x86_64-pc-windows-gnu` toolchain. Install [cbindgen](https://github.com/mozilla/cbindgen) using cargo by running `cargo install cbindgen`.
3. Dump the ROM file of whatever game you want to play to your computer. [GBxCart](https://www.gbxcart.com/) is an affordable cartridge dumper for the Gameboy that you can order on most online shopping websites. Downloading the ROM file from the internet is piracy and it is illegal to do. (NOTE: If you wanted to play Pokemon Red/Blue but didn't have the original game, check out the [pokered disassembly project](https://github.com/pret/pokered). You can build it from source by following their [build instructions](https://github.com/pret/pokered/blob/master/INSTALL.md)).
4. (optional) In order to run tests, you need to install [python3](https://www.python.org/downloads/).
5. (optional) In order to format the project using `make format`, you need to install `clang-format`. The easiest way to do this is by installing the [LLVM](https://releases.llvm.org/download.html) project. 

After setting up all dependencies, run `make all` to build the project. After building, the executable should be in `build/gbemu[.exe]`. To pass a ROM file, use the command line option `[-r/--rom rom file]`. Run the executable with no command line options (`./build/gbemu`) to see usage.

If you installed python3, run `make test` to run the pytest tests. If you installed `clang-format` then run `make format` to format the project.

The emulator should now run correctly. If you have any issues, [open an issue](https://github.com/nishantc1527/gameboy/issues/new) on Github or email me at <nishantc1527@gmail.com>.

# References

All the technical research for this project was done from [this site](https://gbdev.io/pandocs/About.html). For implementing a cycle accurate CPU, I used [this](https://meganesu.github.io/generate-gb-opcodes/) opcode table. If you plan to make your own emulator, these are the primary resources used for development.


