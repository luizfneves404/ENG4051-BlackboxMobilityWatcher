#include <Arduino.h>
#include "IMUSpeed_local.h" // Your local IMU header
#include "WifiLocation.h"   // Our new WiFi Location header

IMUSpeed imu;

// Timing variable for speed printing
unsigned long previousSpeedTime = 0;
const unsigned long SPEED_INTERVAL = 1000; // Print speed every 1 second

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for serial to connect
  delay(1500);

  // Initialize IMU
  if (!imu.begin(8, 9, 100))
  {
    Serial.println("Erro ao iniciar MPU6050");
    while (1)
      ;
  }
  Serial.println("Sensor IMU iniciado");

  // Initialize WiFi Location services
  wifiLocationSetup(); // This is the setup function from WifiLocation.cpp

  Serial.println("Sistema completo iniciado - IMU + WiFi Location");
}

void loop()
{
  unsigned long currentTime = millis();

  // --- IMU Update Task ---
  // This part can run continuously without being blocked.
    if (imu.update()) {
      if (currentTime - previousSpeedTime >= SPEED_INTERVAL) {
        float speed = imu.getSpeed();
        Serial.print("Velocidade: ");
        Serial.print(speed, 3);
        Serial.println(" m/s");
        previousSpeedTime = currentTime;
      }
    }

  // --- Non-Blocking WiFi Location Task ---
  // This single function handles the entire non-blocking process.
  wifiLocationLoop();

  // Optional: Reset speed manually
  // if (digitalRead(0) == LOW) imu.resetSpeed();
}