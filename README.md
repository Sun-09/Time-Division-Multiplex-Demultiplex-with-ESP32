# ESP32 Audio System: TDM, Playback, and MQTT Control

![Image](https://github.com/user-attachments/assets/f3c68c4a-eb75-4aaf-9b64-f792a2caa6d2)

## Overview

This project is a complete ESP32-based audio system that:

- **Reads multiple audio files** from an SD card
- **Performs Time Division Multiplexing (TDM)** to combine audio
- **Demultiplexes and plays audio** using DAC output
- Includes:
  - OLED display support
  - Physical push buttons for control
  - **MQTT Control** :- Action of the MQTT Can be controlled wirelessly from laptop, mobile with the help of Internet from a website. 
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
- Auto-plays songs from the SD card
- OLED displays the current song title

### 3. **Push Button Control**
- GPIO32: Next Song
- GPIO33: Previous Song
- Allows physical interaction without needing a screen

### 4. **MQTT Control**
- Uses MQTT Protocol...
- There is a website for user access.
- Website UI has four buttons - Multiplex, Demultiplex, Play Audio & void (unnecessary button).
- Whenever the User presses the Multiplex button, a message is sent to the Microcontroller(ESP32) that triggers the Multiplex action.
- Same for Demultiplex and playing audio ( for listening to the output - the multiplexed and recovered signals).

### 5. **DIP Switch Song Selection (Optional)(Not Included Yet)**
- 8 DIP switches connected to GPIOs
- One "enable" pin decides if the DIP input is read
- Converts binary input to song index and plays it

---

## **Website UI**
![Image](https://github.com/user-attachments/assets/50be60ed-fd02-4c43-9185-56430883aef4)

## Required Hardware

- **ESP32 Dev Module**
- **MicroSD Card module**
- **Speaker** (with external amplifier)
- **2 Push Buttons**
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
#include <WiFi.h>
#include <PubSubClient.h>
