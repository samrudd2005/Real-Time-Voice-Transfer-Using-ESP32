# Wiring and Components — Task 1.1

## Sender ESP32 and two INMP441 microphones

| ESP32 pin | Connection |
|---|---|
| 3V3 | VDD on both INMP441 modules |
| GND | GND on both modules |
| GPIO 26 | SCK/BCLK on both modules |
| GPIO 25 | WS/LRCLK on both modules |
| GPIO 33 | SD on microphone modules (shared data line, verify module tri-state behavior) |
| GND | L/R on microphone A (left slot) |
| 3V3 | L/R on microphone B (right slot) |

Use short wires and a common ground. Confirm each microphone board supports the
selected left/right slot wiring. Do not connect microphone modules to 5 V unless
their specific board documentation explicitly permits it.

## Receiver ESP32, MAX98357A amplifier and speaker

| ESP32 pin | MAX98357A pin |
|---|---|
| 3V3 | VIN (check the amplifier breakout's allowed supply range) |
| GND | GND |
| GPIO 26 | BCLK |
| GPIO 25 | LRC/WS |
| GPIO 22 | DIN |

Connect the speaker to the amplifier's speaker output terminals, not directly to
ESP32 GPIO pins. Follow the amplifier board's power and speaker impedance ratings.

## Block diagram
See `system_block_diagram.svg`.

## Safety / practical notes
- Disconnect power before changing wiring.
- Never short the amplifier's speaker output to ground.
- Verify pin labels against the exact breakout-board documentation; labels vary.
- The pin assignments match the example code and can be changed in the code.
