#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <MadgwickAHRS.h>

// #define DEG_TO_RAD 0.017453292519943295f

class IMUSpeed {
  public:
    IMUSpeed() : vX(0), vY(0), vZ(0), lastTime(0), sampleIntervalMs(10) {}

    bool begin(uint8_t sdaPin = 8, uint8_t sclPin = 9, uint16_t sampleRateHz = 100) {
      Wire.begin(sdaPin, sclPin);
      mpu.initialize();
      if (!mpu.testConnection()) return false;
      filter.begin(sampleRateHz);
      lastTime = millis();
      sampleIntervalMs = 1000 / sampleRateHz;
      return true;
    }

    bool update() {
      unsigned long now = millis();
      if (now - lastTime < sampleIntervalMs) return false;

      float dt = (now - lastTime) / 1000.0f;
      lastTime = now;

      int16_t axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw;
      mpu.getMotion6(&axRaw, &ayRaw, &azRaw, &gxRaw, &gyRaw, &gzRaw);

      float ax = axRaw / 16384.0f;
      float ay = ayRaw / 16384.0f;
      float az = azRaw / 16384.0f;

      float gx = gxRaw * DEG_TO_RAD / 131.0f;
      float gy = gyRaw * DEG_TO_RAD / 131.0f;
      float gz = gzRaw * DEG_TO_RAD / 131.0f;

      filter.updateIMU(gx, gy, gz, ax, ay, az);

      float q0 = filter.getQ0();
      float q1 = filter.getQ1();
      float q2 = filter.getQ2();
      float q3 = filter.getQ3();

      float gxg = 2 * (q1 * q3 - q0 * q2);
      float gyg = 2 * (q0 * q1 + q2 * q3);
      float gzg = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

      float accX = (ax - gxg) * 9.81f;
      float accY = (ay - gyg) * 9.81f;
      float accZ = (az - gzg) * 9.81f;

      // Elimina ruído com limiar
      if (abs(accX) < 0.2f) accX = 0;
      if (abs(accY) < 0.2f) accY = 0;
      if (abs(accZ) < 0.2f) accZ = 0;

      vX += accX * dt;
      vY += accY * dt;
      vZ += accZ * dt;

      return true;
    }

    float getSpeed() {
      return sqrt(vX * vX + vY * vY + vZ * vZ);
    }

    void resetSpeed() {
      vX = vY = vZ = 0;
    }

    void setSpeed(float vx, float vy, float vz) {
      vX = vx;
      vY = vy;
      vZ = vz;
    }

  private:
    MPU6050 mpu;
    Madgwick filter;

    float vX, vY, vZ;
    unsigned long lastTime;
    unsigned long sampleIntervalMs;
};

// === Exemplo de uso da classe ===

IMUSpeed imu;

void setup() {
  Serial.begin(115200); delay(1500);
  if (!imu.begin(8, 9, 100)) {
    Serial.println("Erro ao iniciar MPU6050");
    while (1);
  }
  Serial.println("Sensor iniciado");
}

unsigned long instanteAnterior = 0;
unsigned long instanteAtual = 0;

void loop() {
  
  

  if (imu.update()) {
    instanteAtual = millis();
    if (instanteAtual > instanteAnterior + 1000) {
      float speed = imu.getSpeed();
      Serial.print("Velocidade: ");
      Serial.print(speed, 3);
      Serial.println(" m/s");
      instanteAnterior = instanteAtual;
    }
  }

  // Exemplo: zerar velocidade manualmente (você pode usar botão aqui)
  // if (digitalRead(0) == LOW) imu.setSpeed(0, 0, 0);
}
