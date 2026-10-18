#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- KREDENSIAL WIFI ---
char ssid[] = "SUDJADI";
char pass[] = "27937005";

// --- KREDENSIAL TELEGRAM ---
#define BOTtoken "8947692422:AAEOvZyzIMjKI5PZYqR7lIt2A7qUVqRqTB8"
#define CHAT_ID "5345518559"

// --- PIN HARDWARE ---
#define DHTPIN D4
#define DHTTYPE DHT22
#define MQ_PIN A0
#define LED_PIN D5

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan = 0;
const long botRequestDelay = 5000; // Jeda 5 detik agar RAM tidak jebol / crash

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Akses ditolak!", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.println("Pesan masuk: " + text);

    if (text == "/start" || text == "/help") {
      String welcome = "🤖 QUARANTINE-X BOT\n\n";
      welcome += "/cek - Cek Sensor\n";
      welcome += "/led_on - Nyalakan LED\n";
      welcome += "/led_off - Matikan LED\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }

    if (text == "/cek") {
      float suhu = dht.readTemperature();
      float kelembapan = dht.readHumidity();
      int nilaiMQ = analogRead(MQ_PIN);

      String status = "📊 DATA SENSOR\n";
      status += "Suhu: " + String(suhu, 1) + " C\n";
      status += "Kelembapan: " + String(kelembapan, 1) + " %\n";
      status += "Gas: " + String(nilaiMQ) + "\n";
      bot.sendMessage(chat_id, status, "");
    }

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "LED Dinyalakan!", "");
    }

    if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "LED Dimatikan!", "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  Wire.begin(D2, D1);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Gagal!");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  client.setInsecure();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("QUARANTINE-X ONLINE");
  display.display();
}

void loop() {
  // Update Layar OLED & Sensor setiap putaran
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  int nilaiMQ = analogRead(MQ_PIN);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("--- QUARANTINE-X ---");
  display.setCursor(0, 16);
  display.print("Suhu  : "); display.print(suhu, 1); display.println(" C");
  display.setCursor(0, 32);
  display.print("Kelem : "); display.print(kelembapan, 1); display.println(" %");
  display.setCursor(0, 48);
  display.print("Gas   : "); display.println(nilaiMQ);
  display.display();

  // Cek pesan Telegram dengan interval aman (5 detik sekali)
  if (millis() - lastTimeBotRan > botRequestDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      while (numNewMessages > 0) {
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
    }
    lastTimeBotRan = millis();
  }
  
  delay(1000);
}