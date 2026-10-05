#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define MESH_PREFIX   "Lab5Medan"
#define MESH_PASSWORD "Medan1234"
#define MESH_PORT     8080

#define DHTPIN  D4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();

Task taskSendMessage(
  TASK_SECOND * 3,
  TASK_FOREVER,
  &sendMessage
);

void sendMessage() {

  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("ERROR DHT");
    return;
  }

  StaticJsonDocument<200> doc;

  doc["tipe"] = "suhu_node";
  doc["suhu"] = suhu;
  doc["kelembapan"] = kelembapan;

  String msg;
  serializeJson(doc, msg);

  mesh.sendBroadcast(msg);

  Serial.println("### NODE 1 MESH ###");
  Serial.println("DATA JSON:");
  Serial.println(msg);
}

void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("############################");
  Serial.println("PROGRAM BARU NODE 1");
  Serial.println("ESP8266 + DHT22 + MESH");
  Serial.println("############################");

  dht.begin();

  delay(2000);

  mesh.setDebugMsgTypes(
    ERROR |
    STARTUP |
    CONNECTION
  );

  mesh.init(
    MESH_PREFIX,
    MESH_PASSWORD,
    &userScheduler,
    MESH_PORT
  );

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  Serial.println("NODE 1 SUDAH AKTIF");
}

void loop() {
  mesh.update();
}