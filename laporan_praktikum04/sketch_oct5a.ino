#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

// =====================================================
// WIFI
// =====================================================

const char* ssid = "cihuyyy";
const char* password = "12345678";

// =====================================================
// PIN
// =====================================================

const byte dhtPin = 2;       // D4 / GPIO2
const byte buttonPin = 4;   // D2 / GPIO4

// LED ON/OFF
const byte ledPin = 12;     // D6 / GPIO12

// LED PWM
const byte pinLED = 5;      // D1 / GPIO5


// =====================================================
// DHT22
// =====================================================

#define DHTTYPE DHT22

DHT dht(dhtPin, DHTTYPE);


// =====================================================
// VARIABEL
// =====================================================

bool ledState = false;

String currentTemp = "--";
String currentHum = "--";

int pwmValue = 0;


// =====================================================
// BUTTON
// =====================================================

int buttonState = LOW;
int lastButtonState = LOW;

unsigned long lastDebounceTime = 0;

const unsigned long debounceDelay = 50;


// =====================================================
// TIMER DHT
// =====================================================

unsigned long lastDHTTime = 0;

const unsigned long dhtInterval = 3000;


// =====================================================
// WEB SERVER & WEBSOCKET
// =====================================================

AsyncWebServer server(80);

AsyncWebSocket ws("/ws");


// =====================================================
// HTML
// =====================================================

const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>Smart Lighting IoT</title>


<style>

body {

  font-family: Arial;

  text-align: center;

  background: #eeeeee;

  margin: 0;

  padding: 20px;

}


.card {

  background: white;

  margin: 20px auto;

  padding: 20px;

  max-width: 350px;

  border-radius: 10px;

  box-shadow: 0 2px 8px rgba(0,0,0,0.15);

}


button {

  padding: 15px 30px;

  font-size: 20px;

  border-radius: 5px;

  cursor: pointer;

  color: white;

  border: none;

}


.btn-on {

  background-color: #4CAF50;

}


.btn-off {

  background-color: #f44336;

}


input[type=range] {

  width: 90%;

}


.value {

  font-size: 25px;

  font-weight: bold;

}


</style>

</head>


<body>


<h1>Smart Room</h1>


<!-- =================================================
     SUHU
================================================= -->

<div class="card">

<h2>Suhu</h2>

<p class="value">

<span id="tempValue">--</span>

&deg;C

</p>

</div>


<!-- =================================================
     KELEMBAPAN
================================================= -->

<div class="card">

<h2>Kelembapan</h2>

<p class="value">

<span id="humValue">--</span>

%

</p>

</div>


<!-- =================================================
     LED ON OFF
================================================= -->

<div class="card">

<h2>LED ON/OFF</h2>

<h3>

Status:

<span id="ledStatus">OFF</span>

</h3>


<button
id="toggleBtn"
class="btn-off"
onclick="toggleLed()">

Turn ON

</button>

</div>


<!-- =================================================
     LED PWM
================================================= -->

<div class="card">

<h2>LED Dimmer</h2>


<p>

Intensitas:

<span id="pwmValue">0</span>

</p>


<input

type="range"

min="0"

max="1023"

value="0"

id="pwmSlider"

oninput="sendPWM(this.value)"

>


<p>0 = Mati</p>

<p>1023 = Terang Maksimal</p>


</div>


<script>


// =====================================================
// WEBSOCKET
// =====================================================

var gateway =
  `ws://${window.location.hostname}/ws`;

var websocket;


// =====================================================
// LOAD
// =====================================================

window.addEventListener(
  'load',
  onLoad
);


function onLoad(event) {

  initWebSocket();

}


// =====================================================
// INIT WEBSOCKET
// =====================================================

function initWebSocket() {

  console.log(
    "Menghubungkan WebSocket..."
  );


  websocket =
    new WebSocket(gateway);


  websocket.onopen =
    onOpen;


  websocket.onclose =
    onClose;


  websocket.onmessage =
    onMessage;

}


// =====================================================
// WEBSOCKET OPEN
// =====================================================

function onOpen(event) {

  console.log(
    "WebSocket Terkoneksi"
  );

}


// =====================================================
// WEBSOCKET CLOSE
// =====================================================

function onClose(event) {

  console.log(
    "WebSocket Terputus"
  );


  setTimeout(
    initWebSocket,
    2000
  );

}


// =====================================================
// LED ON / OFF
// =====================================================

function toggleLed() {

  if (
    websocket.readyState ===
    WebSocket.OPEN
  ) {

    websocket.send(
      "toggle"
    );

  }

}


// =====================================================
// PWM SLIDER
// =====================================================

function sendPWM(value) {


  // Tampilkan nilai PWM

  document.getElementById(
    "pwmValue"
  ).innerHTML = value;


  // Kirim ke NodeMCU

  if (
    websocket.readyState ===
    WebSocket.OPEN
  ) {

    websocket.send(
      "pwm," + value
    );

  }

}


// =====================================================
// TERIMA DATA DARI NODEMCU
// =====================================================

function onMessage(event) {


  console.log(
    "Data diterima:",
    event.data
  );


  var dataObj;


  try {

    dataObj =
      JSON.parse(event.data);

  }

  catch (error) {

    console.log(
      "JSON Error:",
      error
    );

    return;

  }


  // =================================================
  // SUHU
  // =================================================

  if (
    dataObj.suhu !== undefined
  ) {

    document.getElementById(
      "tempValue"
    ).innerHTML =
      dataObj.suhu;

  }


  // =================================================
  // KELEMBAPAN
  // =================================================

  if (
    dataObj.hum !== undefined
  ) {

    document.getElementById(
      "humValue"
    ).innerHTML =
      dataObj.hum;

  }


  // =================================================
  // LED ON OFF
  // =================================================

  if (
    dataObj.led !== undefined
  ) {


    var btn =
      document.getElementById(
        "toggleBtn"
      );


    var status =
      document.getElementById(
        "ledStatus"
      );


    if (
      dataObj.led == "1"
    ) {

      status.innerHTML =
        "ON";


      btn.innerHTML =
        "Turn OFF";


      btn.className =
        "btn-on";

    }


    else {

      status.innerHTML =
        "OFF";


      btn.innerHTML =
        "Turn ON";


      btn.className =
        "btn-off";

    }

  }


  // =================================================
  // PWM
  // =================================================

  if (
    dataObj.pwm !== undefined
  ) {


    document.getElementById(
      "pwmValue"
    ).innerHTML =
      dataObj.pwm;


    document.getElementById(
      "pwmSlider"
    ).value =
      dataObj.pwm;

  }

}


</script>


</body>

</html>

)rawliteral";


// =====================================================
// NOTIFY CLIENTS
// =====================================================

void notifyClients() {


  String jsonString = "{";


  // LED

  jsonString +=
    "\"led\":\"" +
    String(ledState ? 1 : 0) +
    "\",";


  // SUHU

  jsonString +=
    "\"suhu\":\"" +
    currentTemp +
    "\",";


  // KELEMBAPAN

  jsonString +=
    "\"hum\":\"" +
    currentHum +
    "\",";


  // PWM

  jsonString +=
    "\"pwm\":\"" +
    String(pwmValue) +
    "\"";


  jsonString += "}";


  Serial.print(
    "Kirim JSON: "
  );

  Serial.println(
    jsonString
  );


  ws.textAll(
    jsonString
  );

}


// =====================================================
// HANDLE WEBSOCKET MESSAGE
// =====================================================

void handleWebSocketMessage(

  void *arg,

  uint8_t *data,

  size_t len

) {


  AwsFrameInfo *info =
    (AwsFrameInfo*)arg;


  if (

    info->final &&

    info->index == 0 &&

    info->len == len &&

    info->opcode == WS_TEXT

  ) {


    data[len] = 0;


    String message =
      String((char*)data);


    Serial.print(
      "Pesan WebSocket: "
    );

    Serial.println(
      message
    );


    // =================================================
    // TOGGLE LED
    // =================================================

    if (
      message == "toggle"
    ) {


      ledState =
        !ledState;


      digitalWrite(

        ledPin,

        ledState
        ? HIGH
        : LOW

      );


      notifyClients();

    }


    // =================================================
    // PWM
    // Format: pwm,512
    // =================================================

    else if (

      message.startsWith(
        "pwm,"
      )

    ) {


      String nilai =
        message.substring(4);


      pwmValue =
        nilai.toInt();


      // Batasi 0 - 1023

      pwmValue =
        constrain(

          pwmValue,

          0,

          1023

        );


      // Atur kecerahan LED

      analogWrite(

        pinLED,

        pwmValue

      );


      Serial.print(
        "PWM LED: "
      );

      Serial.println(
        pwmValue
      );


      // Kirim kembali ke browser

      notifyClients();

    }

  }

}


// =====================================================
// WEBSOCKET EVENT
// =====================================================

void onEvent(

  AsyncWebSocket *server,

  AsyncWebSocketClient *client,

  AwsEventType type,

  void *arg,

  uint8_t *data,

  size_t len

) {


  switch (type) {


    // =================================================
    // CLIENT CONNECT
    // =================================================

    case WS_EVT_CONNECT:


      Serial.printf(

        "Client WebSocket #%u terhubung\n",

        client->id()

      );


      // Kirim status awal

      notifyClients();


      break;


    // =================================================
    // CLIENT DISCONNECT
    // =================================================

    case WS_EVT_DISCONNECT:


      Serial.printf(

        "Client WebSocket #%u terputus\n",

        client->id()

      );


      break;


    // =================================================
    // DATA
    // =================================================

    case WS_EVT_DATA:


      handleWebSocketMessage(

        arg,

        data,

        len

      );


      break;


    default:

      break;

  }

}


// =====================================================
// SETUP
// =====================================================

void setup() {


  Serial.begin(
    115200
  );


  delay(1000);


  Serial.println();

  Serial.println(
    "================================"
  );

  Serial.println(
    " SMART LIGHTING IoT"
  );

  Serial.println(
    "================================"
  );


  // =================================================
  // BUTTON
  // =================================================

  pinMode(

    buttonPin,

    INPUT

  );


  // =================================================
  // LED ON/OFF
  // =================================================

  pinMode(

    ledPin,

    OUTPUT

  );


  digitalWrite(

    ledPin,

    LOW

  );


  // =================================================
  // LED PWM
  // =================================================

  pinMode(

    pinLED,

    OUTPUT

  );


  analogWrite(

    pinLED,

    0

  );


  // =================================================
  // DHT22
  // =================================================

  dht.begin();


  delay(2000);


  // =================================================
  // WIFI
  // =================================================

  WiFi.mode(
    WIFI_STA
  );


  WiFi.begin(

    ssid,

    password

  );


  Serial.print(
    "Menghubungkan WiFi"
  );


  while (

    WiFi.status()
    != WL_CONNECTED

  ) {


    delay(500);


    Serial.print(
      "."
    );

  }


  Serial.println();


  Serial.println(
    "WiFi TERHUBUNG!"
  );


  Serial.print(
    "IP Address: "
  );


  Serial.println(

    WiFi.localIP()

  );


  // =================================================
  // WEB SERVER
  // =================================================

  server.on(

    "/",

    HTTP_GET,

    [](AsyncWebServerRequest *request) {


      request->send_P(

        200,

        "text/html",

        index_html

      );

    }

  );


  // =================================================
  // WEBSOCKET
  // =================================================

  ws.onEvent(
    onEvent
  );


  server.addHandler(
    &ws
  );


  // =================================================
  // START SERVER
  // =================================================

  server.begin();


  Serial.println(
    "Web Server Aktif"
  );


  Serial.println(
    "WebSocket Aktif"
  );


  Serial.println(
    "================================"
  );

}


// =====================================================
// LOOP
// =====================================================

void loop() {


  // Bersihkan client WebSocket

  ws.cleanupClients();


  // =================================================
  // LED ON/OFF
  // =================================================

  digitalWrite(

    ledPin,

    ledState
    ? HIGH
    : LOW

  );


  // =================================================
  // TOMBOL FISIK
  // =================================================

  int reading =
    digitalRead(
      buttonPin
    );


  if (

    reading !=
    lastButtonState

  ) {


    lastDebounceTime =
      millis();

  }


  if (

    (millis() -
     lastDebounceTime)
    > debounceDelay

  ) {


    if (

      reading !=
      buttonState

    ) {


      buttonState =
        reading;


      // Tombol ditekan

      if (

        buttonState ==
        HIGH

      ) {


        ledState =
          !ledState;


        digitalWrite(

          ledPin,

          ledState
          ? HIGH
          : LOW

        );


        notifyClients();

      }

    }

  }


  lastButtonState =
    reading;


  // =================================================
  // DHT SETIAP 3 DETIK
  // =================================================

  if (

    millis() -
    lastDHTTime
    >= dhtInterval

  ) {


    lastDHTTime =
      millis();


    // Baca suhu

    float t =
      dht.readTemperature();


    // Baca kelembapan

    float h =
      dht.readHumidity();


    // Pastikan valid

    if (

      !isnan(t) &&
      !isnan(h)

    ) {


      currentTemp =
        String(t, 2);


      currentHum =
        String(h, 2);


      Serial.print(
        "Suhu: "
      );

      Serial.print(
        currentTemp
      );

      Serial.print(
        " C | Kelembapan: "
      );

      Serial.print(
        currentHum
      );

      Serial.println(
        " %"
      );


      // Kirim ke browser

      notifyClients();

    }

    else {


      Serial.println(
        "Gagal membaca DHT22!"
      );

    }

  }

}