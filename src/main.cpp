#include <Arduino.h>
#include "IMUSpeed.h" // Your local IMU header
#include "WifiLocation.h"   // Our new WiFi Location header
#include "einkDisplay.h"
#include <WiFi.h>
#include <MQTT.h>
#include <WiFiClientSecure.h>
#include "certificates.h"
#include <ArduinoJson.h>
#include <Preferences.h>

MQTTClient mqtt;

IMUSpeed imu;

WiFiClientSecure conexaoSegura;

Preferences preferencias;

// Timing variable for speed printing
unsigned long previousSpeedTime = 0;
const unsigned long SPEED_INTERVAL = 5000; // Print speed every 1 second

Location previousLocation;

float vX = 0;
float vY = 0;
float vZ = 0;
float speed = 0;

float aX = 0;
float aY = 0;
float aZ = 0;
float acc = 0;

// MQTT publishing wrapper function for dependency injection
void publishToMQTT(const char* topic, const char* payload) {
  if (mqtt.connected()) {
    mqtt.publish(topic, payload);
    Serial.println("MQTT Published - Topic: " + String(topic) + ", Payload: " + String(payload));
  } else {
    Serial.println("MQTT não conectado - mensagem não enviada");
  }
}

void reconectarMQTT()
{
  if (!mqtt.connected())
  {
    Serial.print("Conectando MQTT...");
    while (!mqtt.connected())
    {
      mqtt.connect("Grupo5BMW", "aula", "zowmad-tavQez");
      Serial.print(".");
      delay(1000);
    }
    Serial.println(" conectado!");
    mqtt.subscribe("grupo1/preferencesToESP");
  }

}

void recebeuMensagem(String topico, String conteudo) { 
  Serial.println("Recebendo mensagem do MQTT: " + topico + ": " + conteudo); 
  if (topico == "grupo1/preferencesToESP") {
    JsonDocument doc;
    deserializeJson(doc, conteudo);
    preferencias.putBool("sendAlerts", doc["sendAlerts"]);
    preferencias.putInt("speedMax", doc["speedMax"]);
    preferencias.putInt("accMax", doc["accMax"]);
  }
}

void enviaDadosLocMQTT() {
  Location currentLocation = getCurrentLocation();
  JsonDocument doc;
  doc["latitude"] = currentLocation.latitude;
  doc["longitude"] = currentLocation.longitude;
  doc["accuracy"] = currentLocation.accuracy;
  doc["velocidade"] = speed;
  doc["aceleracao"] = acc;
  String payload;
  serializeJson(doc, payload);
  publishToMQTT("grupo1/posicao", payload.c_str());
}

void setup()
{
  Serial.begin(9600);
  delay(1500);

  preferencias.begin("ajustesBMW");

  // Initialize IMU
  if (!imu.begin(8, 9, 100))
  {
    Serial.println("Erro ao iniciar MPU6050");
    while (1)
      ;
  }
  Serial.println("Sensor IMU iniciado");

  WiFi.mode(WIFI_AP_STA);

  // Setup MQTT callback before einkDisplay setup
  setMQTTPublishCallback(publishToMQTT);
  
  // einkDisplaySetup();

  // Initialize WiFi Location services
  wifiLocationSetup(); // This is the setup function from WifiLocation.cpp

  conexaoSegura.setCACert(get_certificado_mqtt());

  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.onMessage(recebeuMensagem);
  mqtt.setKeepAlive(10);
  mqtt.setWill("tópico da desconexão", "conteúdo");
  reconectarMQTT();

  Serial.println("Sistema completo iniciado");

}



void loop()
{
  unsigned long currentTime = millis();

  // --- IMU Update Task ---
  // This part can run continuously without being blocked.
  if (imu.update()) {
    if (currentTime - previousSpeedTime >= SPEED_INTERVAL) {

    vX = imu.getVX();
    vY = imu.getVY();
    vZ = imu.getVZ();
    speed = imu.getSpeed2D();

    aX = imu.getAccX();
    aY = imu.getAccY();
    aZ = imu.getAccZ();
    acc = imu.getAcc2D();

    // imu.printAll();
    // imu.printTotals();
    imu.printTotals2D();


    if (speed > preferencias.getInt("speedMax", 5.0)) {
      enviaDadosLocMQTT();
    }


    previousSpeedTime = currentTime;
    }
  }

  // --- Non-Blocking WiFi Location Task ---
  // This single function handles the entire non-blocking process.
  wifiLocationLoop();

  Location currentLocation = getCurrentLocation();

  if (previousLocation.latitude != currentLocation.latitude || previousLocation.longitude != currentLocation.longitude)
  {
    enviaDadosLocMQTT();
    // mostraTela();
    previousLocation = currentLocation;
  }

  // einkDisplayLoop();

  reconectarMQTT();
  mqtt.loop();
}