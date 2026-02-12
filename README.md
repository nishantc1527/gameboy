# Gameboy Emulator

A cycle-accurate emulator for the [Nintendo Gameboy](https://en.wikipedia.org/wiki/Game_Boy) that can play most commercial games (such as Pokemon and Tetris).


# Demos

Pokemon Red:

![Alt](assets/pokemon_demo.gif)

Tetris:

![Alt](assets/tetris.gif)

View more demos in the [assets](assets/) folder.

# Motivation


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

# References

All the technical research for this project was done from [this site](https://gbdev.io/pandocs/About.html). For implementing a cycle accurate CPU, I used [this](https://meganesu.github.io/generate-gb-opcodes/) opcode table. If you plan to make your own emulator, these are the primary resources used for development.

