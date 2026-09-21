#include <ESP8266WiFi.h>  
#include <ESP8266WebServer.h>  
#include <DHT.h>

// --- PENGATURAN WI-FI ---
const char* ssid = "Ramadhani";  
const char* password = "rama3311";

ESP8266WebServer server(80);

// --- PENGATURAN PIN ---
const byte dhtPin = 2;       // Pin D4 di NodeMCU
const byte relayPin = 12;    // Pin D6 di NodeMCU
DHT dht(dhtPin, DHT11);

// --- KERANGKA HTML TAMPILAN WEB ---
const char index_html[] PROGMEM = R"rawliteral(  
<!DOCTYPE html>  
<html>  
<head>  
  <!-- TUGAS 3.7: Auto-Refresh Web Setiap 5 Detik -->
  <meta http-equiv="refresh" content="5"> 
  
  <meta name="viewport" content="width=device-width, initial-scale=1">  
  <title>IoT Dashboard</title>  
  <style>  
    body { font-family: Arial; text-align: center; margin-top: 50px; background-color: #f4f4f9;}  
    button { padding: 15px 30px; font-size: 20px; border-radius: 8px; margin: 10px; cursor: pointer; border: none; font-weight: bold;}  
    .btn-on { background-color: #4CAF50; color: white; }  
    .btn-off { background-color: #f44336; color: white; }  
    .sensor-box { font-size: 24px; font-weight: bold; background-color: #ffffff; padding: 20px; border-radius: 10px; display: inline-block; box-shadow: 0 4px 8px rgba(0,0,0,0.1);}  
  </style>  
</head>  
<body>  
  <h1>ESP8266 Web Server</h1>  
  <div class="sensor-box">  
    <p>Suhu Saat Ini: <span style="color:#f44336;">%TEMPERATURE% &deg;C</span></p>  
    <p>Kelembapan: <span style="color:#2196F3;">%HUMIDITY% %</span></p>
  </div>  
  
  <h2>Kendali Relay</h2>  
  <p>Status Relay: <strong>%RELAY_STATUS%</strong></p>
  <!-- Tombol akan berubah otomatis sesuai status Relay -->
  %RELAY_BUTTON%  
</body>  
</html>  
)rawliteral";

void handleRoot() {  
  String html = index_html; 
  float t = dht.readTemperature();  
  float h = dht.readHumidity(); 
  
  if (isnan(t) || isnan(h)) {
    html.replace("%TEMPERATURE%", "--"); 
    html.replace("%HUMIDITY%", "--"); 
  } else {
    html.replace("%TEMPERATURE%", String(t)); 
    html.replace("%HUMIDITY%", String(h)); 
  }
  
  // LOGIKA ACTIVE-LOW
  if (digitalRead(relayPin) == LOW) { 
    html.replace("%RELAY_STATUS%", "MENYALA");
    html.replace("%RELAY_BUTTON%", "<a href=\"/relay/off\"><button class=\"btn-off\">MATIKAN (OFF)</button></a>");
  } else {
    html.replace("%RELAY_STATUS%", "MATI");
    html.replace("%RELAY_BUTTON%", "<a href=\"/relay/on\"><button class=\"btn-on\">NYALAKAN (ON)</button></a>");
  }
  
  server.send(200, "text/html", html);  
}

void handleRelayOn() {  
  // Mengirim sinyal LOW untuk menyalakan Relay
  digitalWrite(relayPin, LOW); 
  server.sendHeader("Location", "/");   
  server.send(303);  
}

void handleRelayOff() {  
  // Mengirim sinyal HIGH untuk mematikan Relay
  digitalWrite(relayPin, HIGH); 
  server.sendHeader("Location", "/");   
  server.send(303);  
}

void setup() {  
  Serial.begin(115200);  
  pinMode(relayPin, OUTPUT);  
  
  // Memastikan Relay mati saat alat pertama kali dihidupkan
  digitalWrite(relayPin, HIGH); 
  
  dht.begin();  
    
  WiFi.mode(WIFI_STA);   
  WiFi.begin(ssid, password);  
  
  Serial.println("\nMenghubungkan ke Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }  
  
  Serial.println("\nBerhasil Terhubung!");
  Serial.print("IP Address Server Anda: ");  
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);  
  server.on("/relay/on", handleRelayOn);  
  server.on("/relay/off", handleRelayOff);  
  server.begin();  
}

void loop() {  
  server.handleClient();  
}