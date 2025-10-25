# STM32F411 7-Segment Display Counter

## Overview

This repository contains a simple yet instructive embedded systems project for the  
STM32F411E-Discovery development board.  
It demonstrates how to drive a single **7-segment LED display** to count  
through the digits **0 – 9**. The code is written in **C** and runs on bare  
metal – no HAL libraries or external frameworks – and uses direct register  
manipulation for clock enabling, pin configuration and digital I/O.

The 7-segment module used is an **ELS-516SURWA/S530-A2** display. It is a  
**common-anode** device, which means all segment cathodes must be pulled low  
relative to the shared anode to turn a segment on. Understanding how common  
anode displays differ from common cathode displays is crucial for correctly  
driving the segments.

## Hardware

- **Microcontroller:** STM32F411E-Discovery
- **Display:** ELS-516SURWA/S530-A2 7-segment LED with common anode
- **Connections:**

  | STM32 pin | Display segment | Notes |
  |------------|-----------------|-------|
  | PD0 | `a` | segment a (bit 0) |
  | PD1 | `b` | segment b (bit 1) |
  | PD2 | `c` | segment c (bit 2) |
  | PD3 | `d` | segment d (bit 3) |
  | PD4 | `e` | segment e (bit 4) |
  | PD5 | `f` | segment f (bit 5) |
  | PD6 | `g` | segment g (bit 6) |
  | PD13 | on-board LED | used as a status LED |
  | PA0 | blue push-button | used for manual increment/reset |
  | VCC/GND | display anode & cathode | connect display common anode to 3.3 V and cathodes through current-limiting resistors to PD0-PD6 |

The segments are wired so that writing a low level on PD0–PD6 lights the  
corresponding segment. Because the module is common anode, the firmware uses  
bitwise **inversion** of the lookup table to obtain the correct active-low  
pattern.

## Software Overview

The firmware resides entirely in `Src/main.c`. It defines base addresses for  
clock and GPIO registers and maps them to **volatile** pointers  
so that the compiler generates direct memory accesses without optimisation  
issues. There are also lookup tables and macros to simplify  
turning individual pins on and off.

A lookup table `digit_to_segments` holds the **active-high** bit patterns for  
numbers 0–9 assuming a **common cathode** display. Because the  
ELS-516SURWA/S530-A2 is a common-anode module, the code inverts each entry  
before writing it to the `GPIOD_ODR` register. Each bit 0–6  
corresponds to segments `a`–`g`, respectively; setting a bit low turns on that  
segment.

## Code Structure

1. **Clock enabling:**  
   The RCC AHB1 peripheral clock register is updated to  
   enable GPIO A and GPIO D clocks by setting bits 0 and 3.

2. **Pin configuration:**  
   - PD13 (on-board LED) is configured as a push-pull output.  
   - PA0 is configured as an input for the blue push-button.  
   - PD0–PD6 are configured as outputs for the 7-segment display.

3. **Lookup table:**  
   `digit_to_segments` defines which segments should be  
   illuminated for each digit. For example, digit 0 uses segments a–f and turns  
   segment g off (`0b00111111`), while digit 8 turns on all segments  
   (`0b01111111`).

4. **Main loop:**  
   - A `digit` variable holds the current number to show.  
   - The current digit pattern is written to the display by combining the  
     existing output data register value with the inverted lookup value for  
     `digit`.  
   - A crude delay loop approximates a 1-second period by looping a large  
     number of times. Inside the loop, the code samples the button to  
     provide responsive incrementing/reset behaviour.  
   - After the delay, `digit` is incremented. When it exceeds 9 it wraps  
     around to 0.

5. **Button handling:**  
   The blue button on PA0 is debounced with a simple  
   delay. `button_previous` stores the previous state, and a change from low to  
   high triggers the button event. If the button is  
   still pressed after a short debounce delay, the firmware updates the  
   counter immediately – either incrementing it or resetting it depending on  
   the selected mode (see below).

6. **Delays:**  
   Because the code runs with the internal 16 MHz HSI clock and  
   no SysTick timer, crude delay constants (`LOOP_1S` and `LOOP_DELAY`) are  
   used to approximate one second and ~20 ms, respectively. In a  
   production system you would replace these with a timer peripheral or  
   SysTick for more accurate timing.

## Modes and Alternatives

The code contains a commented line inside the button handler that illustrates a  
**different mode of operation**. When the button is pressed, the firmware  
currently increments the digit and immediately updates the display:

```c
//digit = 0;     // Reset to zero
 digit = (digit + 1) % 10;       // Increment
 GPIOD_ODR = (GPIOD_ODR & ~SEGMENT_MASK) | (~digit_to_segments[digit] & SEGMENT_MASK);
```

- **Increment mode (default):**  
  The `digit = (digit + 1) % 10` line causes the  
  counter to advance by one each time the button is pressed.  
  This mode allows the user to skip ahead without waiting for the one-second  
  automatic increment.

- **Reset mode:**  
  By un-commenting the line `digit = 0;` and commenting out the  
  increment line, the button would instead **reset** the counter to zero.  
  This is useful if you want the push-button to act as a reset rather than an  
  advance. The comment hints that the same button input can be repurposed  
  depending on the desired behaviour.

The final section of the main loop automatically increments the digit after  
one second and wraps back to zero when the count exceeds 9.  
Combining the automatic increment with the button gives two ways to advance  
through the sequence.

## Customising the Project

- **Timing adjustments:**  
  The delay constants in `main.c` assume a 16 MHz HSI  
  clock. If you enable the external high-speed crystal or change the system  
  clock configuration, you must recalibrate `LOOP_1S` and `LOOP_DELAY` to keep  
  the display timing accurate.

- **Multiple digits:**  
  Driving multiple 7-segment digits requires a multiplexing  
  strategy, enabling one digit at a time and cycling through them quickly  
  enough to appear continuous. This example drives only a single digit for  
  clarity.

- **Peripheral libraries:**  
  For larger projects you may prefer using the  
  STM32Cube HAL or LL libraries for portability and maintainability instead  
  of direct register writes.

## Conclusion

This project illustrates how to control a 7-segment display using the  
STM32F411 microcontroller without relying on high-level libraries. It  
highlights the differences between common-anode and common-cathode displays  
through the use of a lookup table and bitwise inversion. The code is easy to  
adapt for different behaviours by toggling a few lines – for example, turning  
the push-button into a reset or increment control. Feel free to  
extend it to multi-digit displays, add timer interrupts for more precise  
scheduling, or integrate it into a larger embedded application.

## License

This project is released under the **MIT License**.  
You are free to use, modify, and distribute this code for educational or  
commercial purposes as long as the original copyright notice is retained.
