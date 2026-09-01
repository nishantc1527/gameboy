# Testing Checklist

Everything in the **Required** section must pass before a v1.0 release. Optional items are
nice-to-have post-launch improvements.

______________________________________________________________________

## Required

### Automated test suite

Run from the project root:

```bash
make test
```

All of the following must pass with no failures:

| Suite               | Command (individual)                                   |
| ------------------- | ------------------------------------------------------ |
| blargg cpu_instrs   | `pytest tests/blargg_test.py::test_blargg_cpu -v`      |
| blargg instr_timing | `pytest tests/blargg_test.py::test_blargg_cpu_time -v` |
| blargg mem_timing   | `pytest tests/blargg_test.py::test_blargg_mem_time -v` |
| dmg-acid2           | `pytest tests/dmg-acid2_test.py -v`                    |
| cgb-acid2           | `pytest tests/cgb-acid2_test.py -v`                    |
| mbc3-tester         | `pytest tests/mbc3-tester_test.py -v`                  |
| rtc3test basic      | `pytest tests/rtc3test_test.py::test_rtc3_basic -v`    |
| rtc3test range      | `pytest tests/rtc3test_test.py::test_rtc3_range -v`    |
| rtc3test subsecond  | `pytest tests/rtc3test_test.py::test_rtc3_sub -v`      |

______________________________________________________________________

### ROM loading

- [ ] **CLI flag**: `build/gbemu -r path/to/game.gb` launches directly into the game
- [ ] **Ctrl+O dialog**: opens a native file picker filtered to `.gb`/`.gbc`; selecting a file loads it
- [ ] **Recent ROMs list**: after loading a ROM, quit and relaunch — it appears in the recent list; clicking it loads it
- [ ] **Drag and drop (.gb)**: drag a DMG ROM file onto the window and it loads
- [ ] **Drag and drop (.gbc)**: drag a CGB ROM file onto the window and it loads
- [ ] **Invalid file**: drag a non-ROM file onto the window — emulator shows an error and does not crash
- [ ] **Recent list cap**: load 11 different ROMs; verify the list shows only the 10 most recent

______________________________________________________________________

### Screenshots

- [ ] **Basic**: while a game is running, press the screenshot key — a file named `screenshot_0001.png` (or similar) appears in the working directory and contains a correct frame of gameplay
- [ ] **Incrementing within a session**: take three screenshots in a row — files are `screenshot_0001.png`, `screenshot_0002.png`, `screenshot_0003.png` with no overwrites
- [ ] **Incrementing across sessions**: take a screenshot (creates `screenshot_0001.png`), quit, relaunch, take another screenshot — it is named `screenshot_0002.png` (or higher), NOT `screenshot_0001.png`
- [ ] **CGB screenshot**: take a screenshot during a CGB game — the PNG is in full color, not grayscale
- [ ] **DMG screenshot**: take a screenshot during a DMG game — the PNG uses the configured palette colors

______________________________________________________________________

### Save / load persistence

**Pokemon Red (MBC3, no RTC):**

- [ ] Load Pokemon Red, get through the intro, reach the first save point (player's bedroom PC), save the game, quit
- [ ] Relaunch and load Pokemon Red — verify the save was loaded: player is in their bedroom, starter is in the party, progress is preserved
- [ ] Verify that a `.sav` file was created in the working directory alongside the ROM

**Pokemon Crystal (MBC3 + RTC):**

- [ ] Load Pokemon Crystal, start a new game, progress to the first save opportunity, save, note the in-game time, quit
- [ ] Relaunch and load Pokemon Crystal — verify the save loaded correctly and the in-game clock is running

**Save file location:**

- [ ] Confirm `.sav` files are written to the working directory (same folder as the ROM), not a fixed path

______________________________________________________________________

### Gameplay — visual correctness

The goal is to catch rendering regressions in real games before users do.

**Pokemon Red (CGB-compatible, MBC3):**

- [ ] Boot into the Game Freak intro — colors and animation look correct
- [ ] Reach the overworld (player's bedroom) — player sprite is the correct color (green/tan, not pink/wrong)
- [ ] Walk outside — background tiles, NPCs, and buildings render without color glitches

**Tetris (No MBC, DMG):**

- [ ] Boot into the title screen — A-TYPE / B-TYPE menu appears correctly
- [ ] Start a game — pieces fall, controls respond, lines clear, score increments

**Super Mario Land (MBC1, DMG):**

- [ ] Boot into the title screen
- [ ] Start world 1-1 — Mario moves, enemies appear, coins are collected

**Kirby's Dream Land (MBC1, DMG):**

- [ ] Boot into the title screen
- [ ] Enter level 1 — Kirby moves, can inhale enemies, scrolling works correctly

**The Legend of Zelda: Link's Awakening DX (MBC5, CGB):**

- [ ] Boot into the title screen — CGB colors display correctly
- [ ] Walk around the starting area — overworld tiles, Link sprite, and HUD are correct colors

______________________________________________________________________

### Gameplay — controls

Using the default keyboard bindings:

- [ ] **D-pad**: all four directions respond correctly in a game (e.g. move Mario or Link in each direction)
- [ ] **A / B buttons**: perform actions (e.g. jump in Mario, swing sword in Zelda)
- [ ] **Start / Select**: open/close menus
- [ ] **Tab (fast-forward)**: hold Tab — game runs noticeably faster; release Tab — returns to normal speed
- [ ] **Ctrl+R (reset)**: resets the currently loaded game to the boot screen
- [ ] **F11 (fullscreen)**: toggles fullscreen; toggling back returns to windowed mode
- [ ] **Gamepad (if hardware available)**: South=A, East=B, Start=Start, Back=Select, D-pad=D-pad all work

______________________________________________________________________

### Audio

- [ ] **DMG game**: load Tetris — the Tetris theme plays and sound effects trigger on line clears
- [ ] **CGB game**: load Pokemon Red — the intro music plays without distortion
- [ ] **Volume**: audio is at a reasonable default volume (not muted, not ear-splitting)
- [ ] **Mute via volume 0**: set `volume = 0.0` in `~/.config/gbemu/settings.toml`, relaunch — audio is silent

______________________________________________________________________

### Display

- [ ] **Default scale**: window opens at a sensible default size (not 1×, not giant)
- [ ] **CGB color**: CGB game renders in full color
- [ ] **DMG palette**: DMG game uses the 4-color palette from settings; changing `dmg_palette` in settings and relaunching reflects the change
- [ ] **Window title**: title bar shows the ROM title after loading

______________________________________________________________________

## Optional (post-v1.0)

These do not block release but should be tested when implemented.

### Screenshot notification

- [ ] After taking a screenshot, a visible in-game overlay or notification briefly appears confirming the file was saved

### Menu bar

- [ ] A menu bar is visible during gameplay with working Pause, Reset, Open ROM, and Fast-Forward controls

### First-launch settings prompt

- [ ] On first launch (no settings file present), the user is prompted to configure palette, volume, and key bindings before loading a ROM

### Configurable save/screenshot directory

- [ ] Setting a custom path in settings redirects `.sav` files and screenshots to that directory

### blargg halt_bug

- [ ] `pytest tests/blargg_test.py::test_blargg_halt_bug -v` passes

### blargg mem_timing-2

- [ ] `pytest tests/blargg_test.py::test_blargg_mem_time2 -v` passes

### Save states (BESS)

- [ ] Create a save state mid-game, load it — game resumes from exactly that moment
- [ ] Save state files are compatible with the BESS specification

______________________________________________________________________

## Future

These are tracked but not expected for v1.0 or immediately after.

- Mooneye test suite passing
- cgb-acid-hell passing (extreme CGB PPU edge cases)
- Enhanced Pokemon features (party viewer, speedrun helper)
- Configurable DMG palette presets
