# SWRSToys [![builds.sr.ht status](https://builds.sr.ht/~delthas/SWRStoys.svg)](https://builds.sr.ht/~delthas/SWRStoys?)

A modding framework for Touhou Hisoutensoku (12.3).

## Using

- Download the **[latest release](https://delthas.fr/swrstoys.zip)** and extract it in your game folder.
- Open SWRSToys.ini in Notepad and enable some modules by deleting their `;`, then save the file.
- Modify the `.ini` configuration files of modules you enabled (in the `modules/` folder)
- Run the game as usual.

## Modules

### BGMChanger

**Replace the game BGM.**

*The only supported format is OGG Vorbis.*

### MemoryPatch

**Do some misceallenous patches to the game, such as disallowing spectating by default.**

### NetBattleCounter

**Display the number of consecutive online matches you play, and optionally play specific sounds on consecutive games played.**

### NetBellChanger

**Change the game start bell sound for online matches.**

### NetProfileView

**Display profile name for players in a game with a specific formatting (color, font, ...) (spectating and/or playing).**

### ReplayDnD

**Drag and drop a replay file to the game executable file to watch it immediately.**

*Dragging a replay to a running game window does not work; you need to drag it to the game exacutable file in Windows Explorer.*

### ReplayInputView

**Display keystrokes in game replays in real time.**

*The replay speed can also be changed with hotkeys: F10 speeds up; F11 slows down; F12 returns to original speed; F8 and F9 cycle through different game inputs display, for the left and right player respectively.*

*This module does not have a configuration file.*

### ReplayInputView+

**Display keystrokes in game replays in real time, as well as hitboxes, ...**

*The replay speed can also be changed with hotkeys: F10 speeds up; F11 slows down; F12 steps a single frame forward; F4 toggles hitboxes display; F6 displays additional info display; F7 cycles through different game inputs display.*

*This module does not have a configuration file.*

### WindowResizer

**Make the game window resizable.**

## Making a module

- Make a new folder in `modules/`. You can copy an existing module and adapt it.
- Your module will automatically be injected.

## Building

Install Visual Studio (or CMake and the Visual C++ Build Tools). Import the directory into Visual Studio, it will be recognized automatically.

MinGW and Cygwin are not supported (`__thiscall` is needed in order to be compatible with the base game).

After building, run the install target, which will create an `install` folder with all the built files.

## Credits

- *Anonymous Coward* for most of SWRSToys (most of the code here was originally written by him)
- *Shinki* and *PC_volt* for ReplayInputView+

## License

- Files in `include/directx/` are licensed according to their license header
- All other files are licensed according to the `LICENSE` file

