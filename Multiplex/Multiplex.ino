#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 5         // Chip select pin for SD card
#define DAC_PIN 25      // DAC output pin (GPIO25 or GPIO26)

File audio1, audio2, tdmOutput;
const uint32_t SAMPLE_RATE = 44100;  // Hz

void writeWavHeader(File file, uint32_t dataLength) {
  uint32_t fileSize = dataLength + 44 - 8;
  uint16_t audioFormat = 1;
  uint16_t numChannels = 1;
  uint16_t bitsPerSample = 8;
  uint32_t byteRate = SAMPLE_RATE * numChannels * bitsPerSample / 8;
  uint16_t blockAlign = numChannels * bitsPerSample / 8;

  file.seek(0);
  file.write((const uint8_t *)"RIFF", 4);
  file.write((uint8_t *)&fileSize, 4);
  file.write((const uint8_t *)"WAVE", 4);
  file.write((const uint8_t *)"fmt ", 4);

  uint32_t subchunk1Size = 16;
  file.write((uint8_t *)&subchunk1Size, 4);
  file.write((uint8_t *)&audioFormat, 2);
  file.write((uint8_t *)&numChannels, 2);
  file.write((uint8_t *)&SAMPLE_RATE, 4);
  file.write((uint8_t *)&byteRate, 4);
  file.write((uint8_t *)&blockAlign, 2);
  file.write((uint8_t *)&bitsPerSample, 2);

  file.write((const uint8_t *)"data", 4);
  file.write((uint8_t *)&dataLength, 4);
}

uint32_t bytesWritten = 0;
uint32_t totalSamples = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  audio1 = SD.open("/audio1.wav");
  audio2 = SD.open("/audio2.wav");

  if (!audio1 || !audio2) {
    Serial.println("Failed to open input audio files");
    return;
  }
  Serial.println("Audio files opened successfully.");

  // Skip WAV headers
  for (int i = 0; i < 44; i++) {
    audio1.read();
    audio2.read();
  }

  // Calculate total samples
  totalSamples = min(audio1.size(), audio2.size()) - 44;
  Serial.printf("Total samples per file: %lu\n", totalSamples);
  Serial.printf("Total TDM samples to write: %lu\n", totalSamples * 2);

  tdmOutput = SD.open("/tdm_output.wav", FILE_WRITE);
  if (!tdmOutput) {
    Serial.println("Failed to create output file!");
    return;
  }

  for (int i = 0; i < 44; i++) tdmOutput.write((uint8_t)0);
}

void loop() {
  if (audio1.available() && audio2.available()) {
    uint8_t sample1 = audio1.read();
    uint8_t sample2 = audio2.read();

    dacWrite(DAC_PIN, sample1);
    delayMicroseconds(11);
    dacWrite(DAC_PIN, sample2);
    delayMicroseconds(11);

    tdmOutput.write(sample1);
    tdmOutput.write(sample2);
    bytesWritten += 2;

    if (bytesWritten % 1000 == 0) {
      float percentage = (bytesWritten * 100.0) / (totalSamples * 2);
      Serial.printf("Processed %lu out of %lu (%.2f%%)\n", bytesWritten, totalSamples * 2, percentage);
    }

    // if(bytesWritten>=totalSamples){
    //   Serial.println("Playback finished. Finalizing file...");
    //   writeWavHeader(tdmOutput, bytesWritten);
    //   tdmOutput.close();
    //   Serial.println("Output file saved successfully as /output.wav");
    //   while (true);
    // }
  } else {
    Serial.println("Playback finished. Finalizing file...");
    writeWavHeader(tdmOutput, bytesWritten);
    tdmOutput.close();
    Serial.println("Output file saved successfully as /output.wav");
    while (true);
  }
}
