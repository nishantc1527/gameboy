# Gameboy Emulator

An educational project to learn how the [Gameboy](https://en.wikipedia.org/wiki/Game_Boy) operates. In it's current state it can run games that use [no mapper](https://gbhwdb.gekkio.fi/cartridges/no-mapper.html), [MBC1](https://gbhwdb.gekkio.fi/cartridges/mbc1.html), and [MBC3](https://gbhwdb.gekkio.fi/cartridges/mbc3.html), which covers most commercial games (such as Pokemon and Tetris). More obscure mappers will be implemented later.

# Demo

Save file taken from [here](https://projectpokemon.org/home/files/file/4898-my-old-pok%C3%A9mon-red-save-file/).

![Alt](https://i.giphy.com/media/v1.Y2lkPTc5MGI3NjExZ3VlZjc4bjFpaHZpNnJhcmc0YTlxMjA0bXhoZ3B5Z2c2YW10aHl6biZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/DJDkUtqYWHICSCPZ4T/giphy.gif)

# Features

## Mappers

* [No mapper](https://gbhwdb.gekkio.fi/cartridges/no-mapper.html)
* [MBC1](https://gbhwdb.gekkio.fi/cartridges/mbc1.html)
* [MBC3](https://gbhwdb.gekkio.fi/cartridges/mbc3.html)

## CPU & Interrupts

Passes all of [Blargg's test ROMS](https://github.com/retrio/gb-test-roms) (excluding CGB and sound tests). This means that all CPU instructions are implemented and functional, as well as cycle accurate. Interrupts all work as intended.

## Display

Passes [DMG Acid](https://github.com/mattcurrie/dmg-acid2), which tests the display. This means that the PPU is working correctly and is fully functional.

## Audio

Audio is not implemented yet but will be in the future.

## Boot ROM

Supports boot ROM functionality, meaning whenever the emulator is turned on it loads the boot ROM at location 0x0000 and starts from there. You may get a copy of the boot ROM from [here](https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM#Disassemblies).

## Pokemon Hacking

Whenever Pokemon is booted up, it runs a checksum verification to ensure the save file hasn't been messed with. This emulator automatically sets the correct checksum so that you can hack the save file without worrying about the game not starting up. To turn on this functionality, search for the variable named `pokemon` near the top of the file and set its value to 1.

If you want to learn more about hacking the save file, [here](https://bulbapedia.bulbagarden.net/wiki/Save_data_structure_(Generation_I)) is a good reference file.

## Customization

Customization is not featured directly in the GUI, but it is easy to tweak anything through the file itself.

For colors, there are macros defined at the top of the page called `HEX_WHT` through `HEX_BLK` (ignore `HEX_EXT` that's for debugging). You can change the hexcode of each tone (white, light grey, dark grey, and black).

For screen size, you can change the variables `FCT_X` and `FCT_Y`, which scale the x and y axises. 

For controls, look for the function called `handle_in()`. You will see two switch cases, one for key pressed and one for key released. Edit the corresponding key (usually in both switch cases) to change the key. You can use [this file](https://github.com/libsdl-org/SDL/blob/SDL2/include/SDL_keycode.h) to see the keycode needed.

# Usage

This project is mostly educational and wasn't intended to be distributed. If you want to run it, however, you can do so by doing the following:

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) and [SDL](https://www.libsdl.org/) and set it up using [this tutorial](https://lazyfoo.net/tutorials/SDL/01_hello_SDL/windows/msvc2019/index.php). Make sure you have the C++ component installed for Visual Studio then create a project. Note: Visual Studio is only supported on Windows, so if you are on another operating system you may have to tweak the file to get SDL working on whatever environment you are on ([example of setting it up on Linux](https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php)).
2. Install the boot ROM [here](https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM#Disassemblies) and name it `bootrom.rom`. Make sure the file is in the top level of the project directory (...\<solution>\\\<project>).
3. Dump the ROM file to your computer. GBxCart is an affordable cartridge dumper for the Gameboy. Downloading the ROM from the internet is piracy and it is illegal to do.
4. Place the rom file in the same place you put the boot rom. Find the variable named `rom_name` and rename the variable to the name of your file.

Now when you run the file hopefully the emulator should be working. If you wanna make the window smaller/bigger, you can change the `FCT_X` and `FCT_Y` variables.

# References

All research for this project was done from [this site](https://gbdev.io/pandocs/About.html). If you plan to make your own emulator, this is the only site that you will need.
