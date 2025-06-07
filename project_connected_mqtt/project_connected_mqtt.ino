#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool lastButtonState = HIGH;
bool actionTriggered = false;


#define SD_CS 5
#define DAC_PIN 25
const uint32_t SAMPLE_RATE = 44100;
#define NEXT_BUTTON 32     // GPIO for Next
#define PREV_BUTTON 33      // GPIO for previous

// WiFi credentials
const char* ssid = "Sun-09";
const char* password = "11111111";

// MQTT details
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic_sub = "sonu";
const char* mqtt_topic_pub = "esp32_mqtt_tempvalue";

WiFiClient espClient;
PubSubClient client(espClient);
char receivedChar = '\0';
char displayText[20] = "";

void callback(char* topic, byte* payload, unsigned int length) {
  if (length > 0) {
    receivedChar = (char)payload[0];  // Only taking first character
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32Client_" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_sub);
      display.setCursor(0, 10);
      display.setTextSize(1);
      display.print("MQTT Connected");
      display.display();
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s");
      display.setCursor(0, 10);
      display.setTextSize(1);
      display.print("Failed, Retrying in 5 sec.");
      display.display();
      delay(5000);
    }
  }
}

File audio1, audio2, tdmOutput;
File inputFile, audio1File, audio2File;
File currentFile;
File root;
String fileList[50];
int totalFiles = 0;
int currentIndex = 0;
bool playing = false;

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

void buildPlaylist() {
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory()) {
      String name = entry.name();
      if (name.endsWith(".wav")) {
        fileList[totalFiles++] = "/" + name;
        Serial.println(name);
      }
    }
    entry.close();
  }
}

void playCurrentFile() {
  if (currentFile) currentFile.close();
  if (totalFiles == 0) return;

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
    for (int i = 0; i < 44; i++) currentFile.read();
    playing = true;
  } else {
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


void multiplex(){
  Serial.print("Entered the block");
  audio1 = SD.open("/audio1.wav");
  audio2 = SD.open("/audio2.wav");
  for (int i = 0; i < 44; i++) { audio1.read(); audio2.read(); }

  uint32_t totalSamples = min(audio1.size(), audio2.size()) - 44;
  tdmOutput = SD.open("/tdm_output.wav", FILE_WRITE);
  for (int i = 0; i < 44; i++) tdmOutput.write((uint8_t)0);
  uint32_t bytesWritten = 0;

  while (audio1.available() && audio2.available()) {
    uint8_t s1 = audio1.read(), s2 = audio2.read();
    dacWrite(DAC_PIN, s1); delayMicroseconds(11);
    dacWrite(DAC_PIN, s2); delayMicroseconds(11);
    tdmOutput.write(s1); tdmOutput.write(s2);
    bytesWritten += 2;
    if (bytesWritten % 1000 == 0) {
      float percentage = (bytesWritten * 100.0) / (totalSamples * 2);
      if((bytesWritten / 1000) % 10 == 0){
        display.clearDisplay();
        display.setCursor(0, 10);
        display.setTextSize(1);
        display.print("Multiplexing, Completed:");
        display.setCursor(0, 25);
        display.print(percentage);
        display.display();
      }
      Serial.printf("Processed %lu out of %lu (%.2f%%)\n", bytesWritten, totalSamples * 2, percentage);
    }
  }

  writeWavHeader(tdmOutput, bytesWritten);
  tdmOutput.close();
  audio1.close(); audio2.close();
}


void demultiplex(){
// Demultiplexing
  inputFile = SD.open("/tdm_output.wav");
  inputFile.seek(44);
  audio1File = SD.open("/recovered1.wav", FILE_WRITE);
  audio2File = SD.open("/recovered2.wav", FILE_WRITE);
  for (int i = 0; i < 44; i++) { audio1File.write((uint8_t)0); audio2File.write((uint8_t)0); }
  uint32_t count1 = 0, count2 = 0; bool toggle = true;
  uint32_t processedSamples = 0;
  uint32_t totalSamples = inputFile.size() - 44;

  while (inputFile.available()) {
    uint8_t sample = inputFile.read();
    if (toggle) { audio1File.write(sample); count1++; }
    else { audio2File.write(sample); count2++; }
    toggle = !toggle;

    processedSamples++;

    if (processedSamples % 1000 == 0) {
          float percent = (processedSamples * 100.0) / totalSamples;
          Serial.printf("🔄 Processed %lu of %lu samples (%.1f%%)\n", processedSamples, totalSamples, percent);
          if ((processedSamples / 1000) % 10 == 0) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextSize(1);
            display.print("Demultiplexing, Completed:");
            display.setCursor(0, 15);
            display.print(percent);
            display.display();
          }
          
        }

  }

  writeWavHeader(audio1File, count1);
  writeWavHeader(audio2File, count2);
  audio1File.close(); audio2File.close(); inputFile.close();
}


void playing_audio(){
  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(PREV_BUTTON, INPUT_PULLUP);

  // Build playlist and start playback
  buildPlaylist();
  playCurrentFile();
  while(true){
    if (playing && currentFile.available()) {
    uint8_t sample = currentFile.read();
    dacWrite(DAC_PIN, sample);
  } else if (playing && !currentFile.available()) {
    currentFile.close();
    nextSong();
  }
  // (Optional) button check for song navigation here

  if (digitalRead(NEXT_BUTTON) == LOW) {
    delay(200);  // Debounce
    nextSong();
  }
  if (digitalRead(PREV_BUTTON) == LOW) {
    delay(200);  // Debounce
    prevSong();
  }

  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD Card Mount Failed!");
  } else {
    Serial.println("✅ SD Card Initialized");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) while (true);
  display.clearDisplay();
  display.setTextColor(WHITE);

  WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(" connected");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("WiFi Connected");
    display.display();

    // MQTT setup
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    client.setKeepAlive(60);
  
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Check for rising edge (button just pressed)
  if (receivedChar == 'P' || receivedChar == 'Q') {
    strcpy(displayText, "MULTIPLEX");
    multiplex();
    receivedChar = '\0';  // avoid running again
  }
  else if (receivedChar == 'R' || receivedChar == 'S') {
    strcpy(displayText, "DEMULTIPLEX");
    demultiplex();
    receivedChar = '\0';
  }
  else if (receivedChar == 'T' || receivedChar == 'U') {
    strcpy(displayText, "PLAYING AUDIO");
    playing_audio(); // blocks
    receivedChar = '\0';
  }

  // Serial.print("Reached before display");
  // Display
  // display.clearDisplay();
  // display.setCursor(0, 10);
  // display.setTextSize(1);
  // display.print(receivedChar);
  // display.setCursor(0, 20);
  // display.print(displayText);
  // display.display();
  // Serial.print(displayText);
}
