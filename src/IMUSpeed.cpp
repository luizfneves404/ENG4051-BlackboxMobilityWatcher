#include "IMUSpeed.h"

IMUSpeed::IMUSpeed() : vX(0), vY(0), vZ(0), lastTime(0), sampleIntervalMs(10) {}

bool IMUSpeed::begin(uint8_t sdaPin, uint8_t sclPin, uint16_t sampleRateHz) {
  Wire.begin(sdaPin, sclPin);
  mpu.initialize();
  if (!mpu.testConnection()) return false;
  filter.begin(sampleRateHz);
  lastTime = millis();
  sampleIntervalMs = 1000 / sampleRateHz;
  return true;
}

bool IMUSpeed::SpeedUpdate() {
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

  float q0 = filter.q0;
  float q1 = filter.q1;
  float q2 = filter.q2;
  float q3 = filter.q3;

  float gxg = 2 * (q1 * q3 - q0 * q2);
  float gyg = 2 * (q0 * q1 + q2 * q3);
  float gzg = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

  float accX = (ax - gxg) * 9.81f;
  float accY = (ay - gyg) * 9.81f;
  float accZ = (az - gzg) * 9.81f;

  if (abs(accX) < 0.2f) accX = 0;
  if (abs(accY) < 0.2f) accY = 0;
  if (abs(accZ) < 0.2f) accZ = 0;

  vX += accX * dt;
  vY += accY * dt;
  vZ += accZ * dt;

  return true;
}

float IMUSpeed::getSpeed() {
  return sqrt(vX * vX + vY * vY + vZ * vZ);
}

void IMUSpeed::resetSpeed() {
  vX = vY = vZ = 0;
}

void IMUSpeed::setSpeed(float vx, float vy, float vz) {
  vX = vx;
  vY = vy;
  vZ = vz;
}