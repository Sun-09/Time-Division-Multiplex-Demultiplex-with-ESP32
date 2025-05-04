#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SD_CS 5            // SD card CS pin
#define DAC_PIN 25         // DAC output pin
#define SAMPLE_RATE 44100  // 44.1kHz audio
#define NEXT_BUTTON 32     // GPIO for Next
#define PREV_BUTTON 33     // GPIO for Previous

File root;
File currentFile;
String fileList[50];       // Store up to 50 files
int totalFiles = 0;
int currentIndex = 0;
bool playing = false;


void buildPlaylist() {
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    
    if (!entry.isDirectory()) {
      String name = entry.name();
      if (name.endsWith(".wav")) {
        fileList[totalFiles++] = "/" + name;  // <-- IMPORTANT: Add "/" before filename
        Serial.print("Found: ");
        Serial.println(name);
      }
    }
    entry.close();
  }
}

void playCurrentFile() {
  if (currentFile) {
    currentFile.close();
  }

  if (totalFiles == 0) {
    Serial.println("❌ No audio files found!");
    return;
  }

  Serial.print("▶️ Now Playing: ");
  Serial.println(fileList[currentIndex]);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Playing:");
  display.setCursor(0, 10);
  display.print(fileList[currentIndex]);
  display.display();
  

  currentFile = SD.open(fileList[currentIndex]);
  if (currentFile) {
    for (int i = 0; i < 44; i++) currentFile.read();  // Skip header
    playing = true;
  } else {
    Serial.println("❌ Failed to open file!");
    playing = false;
  }
}

void nextSong() {
  currentIndex++;
  if (currentIndex >= totalFiles) currentIndex = 0;
  playCurrentFile();
}

void prevSong() {
  if (currentIndex == 0) currentIndex = totalFiles - 1;
  else currentIndex--;
  playCurrentFile();
}





void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED init failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(PREV_BUTTON, INPUT_PULLUP);

  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD Card initialization failed!");
    return;
  }
  Serial.println("✅ SD Card initialized.");

  root = SD.open("/");
  if (!root) {
    Serial.println("❌ Failed to open root directory!");
    return;
  }

  buildPlaylist();
  playCurrentFile();
}

void loop() {
  // Play audio
  if (playing && currentFile.available()) {
    uint8_t sample = currentFile.read();
    dacWrite(DAC_PIN, sample);
    // delayMicroseconds(1000000 / SAMPLE_RATE);
  } 
  else if (playing && !currentFile.available()) {
    currentFile.close();
    nextSong();
  }

  // Check buttons
  if (digitalRead(NEXT_BUTTON) == LOW) {
    delay(200);  // Debounce
    nextSong();
  }
  if (digitalRead(PREV_BUTTON) == LOW) {
    delay(200);  // Debounce
    prevSong();
  }
}


