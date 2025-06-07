# 🎧 Time Division Multiplexing and Demultiplexing of Audio using ESP32



A part of final year engineering project that implements **Time Division Multiplexing (TDM)** and **Demultiplexing** of two `.wav` audio files using the **ESP32 Dev Module**, with real-time playback through its internal **DAC**, command control via **MQTT**, and a visual interface on an **OLED display**.


![Image](https://github.com/user-attachments/assets/61bb462b-f707-4c99-a437-53a93f83328a)

---

## 📌 Features

- 🔁 Time Division Multiplexing of two `.wav` audio files from SD card
- 🔃 Demultiplexing and separate playback of audio channels
- 🎚 Playback via **ESP32 DAC** in real-time
- 🗂 WAV file read/write with proper header handling
- 🌐 MQTT command support (via **HiveMQ broker**)
- 🖥 OLED display for system state updates
- 🎮 Navigation buttons for manual control

---

## 🎯 Objective

To demonstrate digital multiplexing principles in embedded systems by building a working prototype that can:
- Read two audio files
- Interleave samples in time domain (TDM)
- Write and store the combined result
- Separate the interleaved data (Demux)
- Play output using ESP32 DAC

---

## 🛠️ Hardware Requirements

| Component          | Description                     |
|-------------------|---------------------------------|
| ESP32 Dev Module  | Core microcontroller             |
| MicroSD Module    | For reading/writing WAV files    |
| 0.96" OLED (I2C)  | For displaying system status     |
| Push Buttons (3x) | For playback control/navigation  |
| 3.5mm Audio Jack  | For output via DAC               |
| Resistors/Wires   | Basic components for connections |

---

## 🧱 Software Stack

- PlatformIO / Arduino IDE
- ESP32 Arduino Core
- PubSubClient (MQTT)
- Adafruit SSD1306 / GFX (OLED)
- SD / SPI libraries for file I/O

---

## Website UI

![Image](https://github.com/user-attachments/assets/50be60ed-fd02-4c43-9185-56430883aef4)

---

## ⚙️ Functionality Overview

### 🔄 Multiplexing

- Reads equal-length samples from `audio1.wav` and `audio2.wav`
- Alternates sample-by-sample to create a new `.wav` file (`tdm.wav`)
- Preserves WAV header with adjusted chunk size

### 🔁 Demultiplexing

- Reads interleaved `tdm.wav` file
- Separates into `output1.wav` and `output2.wav`
- Reconstructs original files accurately

### ▶️ Playback

- Plays selected file (`audio1.wav`, `audio2.wav`, or `tdm.wav`)
- Uses `DAC1 (GPIO25)` for analog output
- Timing delays match sample rate to preserve audio speed

---

## HOW MQTT Works

![Image](https://github.com/user-attachments/assets/a36686f1-be98-4317-abad-cad040f59722)

## 🛰️ MQTT Control

- Uses public broker: **HiveMQ**
- Topic subscribed: `sonu`
- Commands:
  - `"P"` = Play `audio1.wav`
  - `"Q"` = Play `audio2.wav`
  - `"R"` = Play `tdm.wav`
  - `"M"` = Start multiplexing
  - `"D"` = Start demultiplexing

Example Publisher (from web):
```js
client.publish("sonu", "M"); 
```

## 🖥 OLED Display Messages

"MULTIPLEXING" – When TDM starts

"DEMULTIPLEXING" – During separation

"PLAYING FILE" – During playback

"MQTT CONNECTED" – On broker connect

## 🧪 Testing Procedure

Load .wav files into SD card.

Power ESP32 and ensure OLED initializes.

Send MQTT command from HiveMQ test client.

Observe action on OLED and listen to output.

Validate tdm.wav, output1.wav, and output2.wav from SD.

## 🚀 Future Enhancements

📶 Add Web UI for file selection and control

🎵 Support more than two audio streams

🔊 Use external DAC for higher-quality playback

🔋 Add sleep/power-saving mode

📁 FAT filesystem error handling and logging



