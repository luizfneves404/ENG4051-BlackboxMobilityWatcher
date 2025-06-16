#include <Arduino.h>
#include "IMUSpeed_local.h"
#include "WifiLocation.h"

IMUSpeed imu;

// Timing variables
unsigned long previousSpeedTime = 0;
unsigned long previousLocationTime = 0;
const unsigned long SPEED_INTERVAL = 1000;     // Print speed every 1 second
const unsigned long LOCATION_INTERVAL = 30000; // Get location every 30 seconds

void setup() {
    neopixelWrite(10, 0, 0, 0);
    delay(1000);
    neopixelWrite(10, 0, 0, 100);
    delay(1000);
    neopixelWrite(10, 0, 0, 0);
    delay(1000);
    neopixelWrite(10, 0, 0, 100);
    delay(1000);
    neopixelWrite(10, 0, 0, 0);
    delay(1000);
    neopixelWrite(10, 0, 0, 100);
  Serial.begin(9600); 
  delay(1500);
  
//   // Initialize IMU
//   if (!imu.begin(8, 9, 100)) {
//     Serial.println("Erro ao iniciar MPU6050");
//     while (1);
//   }
//   Serial.println("Sensor IMU iniciado");
  
  // Initialize WiFi Location
  wifiLocationSetup();
  Serial.println("Sistema de localização WiFi iniciado");
  
  Serial.println("Sistema completo iniciado - IMU + WiFi Location");
}

void loop() {
  unsigned long currentTime = millis();
  
//   // Update IMU data
//   if (imu.update()) {
//     // Print speed every SPEED_INTERVAL milliseconds
//     if (currentTime - previousSpeedTime >= SPEED_INTERVAL) {
//       float speed = imu.getSpeed();
//       Serial.print("Velocidade: ");
//       Serial.print(speed, 3);
//       Serial.println(" m/s");
//       previousSpeedTime = currentTime;
//     }
//   }
  
  // Get location every LOCATION_INTERVAL milliseconds
  if (currentTime - previousLocationTime >= LOCATION_INTERVAL) {
    Serial.println("--- Obtendo localização ---");
    wifiLocationLoop();
    previousLocationTime = currentTime;
  }
  
  // Optional: Reset speed manually (you can connect a button here)
  // if (digitalRead(0) == LOW) imu.resetSpeed();
}
