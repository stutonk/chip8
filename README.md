chip8
===

This is an SDL2-based emulator for the popular [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) computer. 
It comes equiped with cutting-edge graphics, boasting an astonishing 
64x32 resolution and brain-melting two colors. Here's what it looks like:

![epic in-game screenshot](https://raw.githubusercontent.com/stutonk/chip8/master/screenshot.png)

### Building
1. Make sure you have SDL2 with headers installed somewhere
2. `make`

### Usage
`./chip8 [-s scale] [-e entry_point] path/to/chip8/rom`

-s changes the resolution to (64\*scale)x(32\*scale) where 3 is the default scale.  
-e specifices the CHIP-8 memory location to load your rom. Don't set
this unless you know what you're doing.

### Input
The CHIP-8 has a 4x4 input keypad which maps to QWERTY like:
```
+---+---+---+---+    +---+---+---+---+
| 1 | 2 | 3 | C | => | 1 | 2 | 3 | 4 |
+---+---+---+---+    +---+---+---+---+
| 4 | 5 | 6 | D | => | Q | W | E | R |
+---+---+---+---+    +---+---+---+---+
| 7 | 8 | 9 | E | => | A | S | D | F |
+---+---+---+---+    +---+---+---+---+
| A | 0 | B | F | => | Z | X | C | V |
+---+---+---+---+    +---+---+---+---+
```
Press `ESC` at any time to quit the program immediately.

### Games
You can find some [here](https://www.zophar.net/pdroms/chip8/chip-8-games-pack.html).
