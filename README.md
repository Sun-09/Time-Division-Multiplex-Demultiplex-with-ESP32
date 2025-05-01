# ESP32 Audio System: TDM, Playback, and IR Control

## Overview

This project is a complete ESP32-based audio system that:

- **Reads multiple audio files** from an SD card
- **Performs Time Division Multiplexing (TDM)** to combine audio
- **Demultiplexes and plays audio** using DAC output
- Includes:
  - OLED display support
  - Physical push buttons for control
  - **IR Remote** for wireless song selection and pause/resume
  - (Optional) DIP switch input system for random song access

---

## Features

### 1. **TDM & Demultiplexing**
- Two WAV files (`audio1.wav` and `audio2.wav`) are read from SD card
- Interleaved into a new `output.wav` via TDM (byte-by-byte)
- Can be demultiplexed and written back as individual files

### 2. **Audio Playback**
- DAC (GPIO25) outputs 8-bit WAV data at 44.1kHz
- Skips 44-byte WAV headers
- Auto-plays songs from SD card
- OLED displays current song title

### 3. **Push Button Control**
- GPIO32: Next Song
- GPIO33: Previous Song
- Allows physical interaction without needing a screen

### 4. **IR Remote Integration**
- Uses standard IR remotes
- Supports:
  - Song index selection (e.g., 0–9 keys)
  - Pause/Resume toggle
- IR decoding using `IRremote` library

### 5. **DIP Switch Song Selection (Optional)**
- 8 DIP switches connected to GPIOs
- One "enable" pin decides if DIP input is read
- Converts binary input to song index and plays it

---

## Required Hardware

- **ESP32 Dev Module**
- **MicroSD Card module**
- **Speaker** (with external amplifier)
- **2 Push Buttons**
- **IR Receiver module** (e.g., TSOP1838)
- **OLED Display (128x32 or 128x64)**
- (Optional) **DIP Switch Array**

---

## Dependencies

```cpp
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.h>
