# Smart Light System

## Basic Knowledge for this system

### Embeded concepts

- What is a microcontroller (like ESP32)?
- What is GPIO? (General Purpose Input/Output)
- What is PWM (used in LED brightness control)?
- What is a digital signal vs analog?

## Hardware

- ESP32-S3-DevKitC-1-N16R8
- WS2812B 5m 30 leds/m
- breadboard(830 孔带背胶), jump wires
- capacitor, resistor
- INMP441 I2S
- power supply (5v 10A), input: 200~265V AC 50Hz, output: 5V 10A
- logic level converter (3.3V to 5V)
- Macbook air

## Software

- Install Arduino IDE
  - https://www.arduino.cc/en/software
- Install Required Libraries (via Arduino Library Manager)
  - Install the ESP32-S3 board support
    - Add Board Manager URL:
      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
    - Install "ESP32 by Espressif Systems" in Boards Manager.
  - FastLED (for controlling LEDs)
  - arduinoFFT (for audio frequency analysis)
  - ESP32_I2S (for INMP441 microphone input)
