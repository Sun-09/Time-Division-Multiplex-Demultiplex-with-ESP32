#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 5

File inputFile;
File audio1File;
File audio2File;

const char* inputFileName = "/tdm_output.wav";
const char* audio1Name = "/recovered1.wav";
const char* audio2Name = "/recovered2.wav";

void writeWavHeader(File file, uint32_t dataSize) {
  uint32_t fileSize = dataSize + 44 - 8;
  file.seek(0);
  file.write((const uint8_t *)"RIFF", 4);
  file.write((uint8_t*)&fileSize, 4);
  file.write((const uint8_t *)"WAVE", 4);
  file.write((const uint8_t *)"fmt ", 4);

  uint32_t subchunk1Size = 16;
  uint16_t audioFormat = 1;
  uint16_t numChannels = 1;
  uint32_t sampleRate = 44100;
  uint16_t bitsPerSample = 8;
  uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
  uint16_t blockAlign = numChannels * bitsPerSample / 8;

  file.write((uint8_t*)&subchunk1Size, 4);
  file.write((uint8_t*)&audioFormat, 2);
  file.write((uint8_t*)&numChannels, 2);
  file.write((uint8_t*)&sampleRate, 4);
  file.write((uint8_t*)&byteRate, 4);
  file.write((uint8_t*)&blockAlign, 2);
  file.write((uint8_t*)&bitsPerSample, 2);

  file.write((const uint8_t *)"data", 4);
  file.write((uint8_t*)&dataSize, 4);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD Card init failed");
    return;
  }
  Serial.println("✅ SD Card ready");

  inputFile = SD.open(inputFileName);
  if (!inputFile) {
    Serial.println("❌ Failed to open input file");
    return;
  }

  uint32_t totalSamples = inputFile.size() - 44;
  Serial.printf("📂 Input file opened. Total samples to process: %lu\n", totalSamples);

  SD.remove(audio1Name);
  SD.remove(audio2Name);

  audio1File = SD.open(audio1Name, FILE_WRITE);
  audio2File = SD.open(audio2Name, FILE_WRITE);

  if (!audio1File || !audio2File) {
    Serial.println("❌ Failed to create output files");
    return;
  }

  for (int i = 0; i < 44; i++) {
    audio1File.write((byte)0);
    audio2File.write((byte)0);
  }

  inputFile.seek(44);  // Skip WAV header

  uint32_t count1 = 0, count2 = 0;
  bool toggle = true;
  uint32_t processedSamples = 0;

  unsigned long startTime = millis();

  while (inputFile.available()) {
    uint8_t sample = inputFile.read();

    if (toggle) {
      audio1File.write(sample);
      count1++;
    } else {
      audio2File.write(sample);
      count2++;
    }
    toggle = !toggle;

    processedSamples++;

    if (processedSamples % 1000 == 0) {
      float percent = (processedSamples * 100.0) / totalSamples;
      Serial.printf("🔄 Processed %lu of %lu samples (%.1f%%)\n", processedSamples, totalSamples, percent);
    }
  }

  unsigned long endTime = millis();
  float seconds = (endTime - startTime) / 1000.0;

  writeWavHeader(audio1File, count1);
  writeWavHeader(audio2File, count2);

  audio1File.close();
  audio2File.close();
  inputFile.close();

  Serial.println("✅ Demux complete. Files saved as recovered1.wav and recovered2.wav");
  Serial.printf("⏱ Total time taken: %.2f seconds\n", seconds);
}

void loop() {
  // Do nothing
}
