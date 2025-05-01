#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 5  // Chip select pin

void listFiles(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("❌ Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("❌ Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("📁 DIR : ");
      Serial.println(file.name());
      if (levels) {
        listFiles(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("📄 FILE: ");
      Serial.print(file.name());
      Serial.print(" (");
      Serial.print(file.size());
      Serial.println(" bytes)");
    }
    file = root.openNextFile();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD Card initialization failed!");
    return;
  }
  Serial.println("✅ SD Card initialized.");

  listFiles(SD, "/", 2);  // Start listing from root folder, 2 levels deep
}

void loop() {
  // Nothing
}
