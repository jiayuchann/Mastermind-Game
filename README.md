# Mastermind-Game

Mastermind is a code-breaking game [created in 1970 by Mordecai Meirowitz](https://en.wikipedia.org/wiki/Mastermind_(board_game)). For the final project you will recreate this game using the MSP430, a potentiometer, RGB LED, onboard button and 7-segment display.

## Example Output

```
> CMYK
? CMYK  P L
0 KBYY: 1 1
1 CMBR: 2 0
2 RGYK: 2 0
3 KMYC: 2 2
4 CMYK: 4 0 Won!
> RGBW
? RGBW  P L
0 RYBM: 2 0
1 MCBM: 1 0
2 MYRB: 0 2
3 CCMY: 0 0
4 MMWW: 1 0
5 WCMB: 0 2
6 YYYY: 0 0
7 YYYY: 0 0
8 YYYY: 0 0
9 YYYY: 0 0 Lost :(
>
```

##### NOTE: [`K` is Black](https://en.wikipedia.org/wiki/CMYK_color_model) (all lights off)

## Wiring

[Please see the wiring video.](https://youtu.be/HcnwtzK9YsY)

To use the `P2.6 (XIN)` and `P2.7 (XOUT)` pins as regular GPIO (general purpose input/output), you must clear the bits from the first special function select memory location:

```c
P2SEL  &= ~(BIT6 | BIT7);
```

This line is provided in the template file for you.

## Design

For the final assignment, it will be easier to use C instead of assembly, so that is what the template file is written in. Feel free to do it in assembly if you really want, though.

### Step 1: Obtaining Keyboard Input

Initially the microcontroller should wait for input from the "mastermind" to choose the code to be broken (shown as a `>` prompt). I have given you an "interface" to implement in the `input.c` file: the `prompt_for_line()` function. The `MAX_LINE` defines how many character (minus one for the `\0` terminator!) should be allowed to be typed. Remember that for anything to be shown on the screen you must actually print it. Key presses are sent to the microcontroller, but it's up to your code what to do something with them. A very simple example is given in this file initally.

When the backspace key is pressed, the ASCII code `0x08` or `\b` is sent. You can check for this character to know when to delete a character from the buffer and move backward.

### Step 2: Playing the Game

```
? CMYK  P L
```

* `?`: number of guess (starting at 0)
* `CMYK`: the code to break
* `P`: number of colors _that are in the correct position_
* `L`: number of correct colors _but in the wrong position_

Once the code is selected, the game then waits for the "player" to dial in the color with the potentiometer and enter that color by pressing the onboard button. I have provided a function that handles the potentiometer hardware settings for you called `initialize_dtc()` that is given in `dtc.c` and `dtc.h`. This is used in the template file and you won't need to mess with it. At any point in your program you can read from the variable `pot_value` to obtain a number between 0-1023 that indicates what position the potentiometer is at (512 is halfway or 50%, 256 is a quarter or 25%, etc.)

When four colors have been entered for the code, the game will check it against the answer and output:

1. correct colors in correct positions (black or colored pegs, `P` in the example output)
1. correct colors, but in incorrect positions (white pegs, `L` in the example output)

These need to be shown on the 7-segment display to the "player". `P` on the left and `L` on the right.

If the "player" correctly guessed all 4 colors in the correct position then the game is won. If the "player" has tried to guess 10 times and still doesn't have the correct answer, then the game is lost.
