#include "IMUSpeed.h"

IMUSpeed::IMUSpeed() : vX(0), vY(0), vZ(0), lastTime(0), sampleIntervalMs(10) {}

#ifndef DEG_TO_RAD 
#define DEG_TO_RAD 0.017453292519943295769236907684886
#endif

#define CALIB_OFFSET_NB_MES 1000
void IMUSpeed::calibrateSensorOffsets() {
  float accSumX = 0, accSumY = 0, accSumZ = 0;
  float gyroSumX = 0, gyroSumY = 0, gyroSumZ = 0;
  
  for (int i = 0; i < CALIB_OFFSET_NB_MES; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    accSumX += ax;
    accSumY += ay;
    accSumZ += az;  // 1g em repouso = 16384 LSB
    gyroSumX += gx;
    gyroSumY += gy;
    gyroSumZ += gz;

    delay(1);
  }

  int16_t accOffsetX = accSumX / CALIB_OFFSET_NB_MES;
  int16_t accOffsetY = accSumY / CALIB_OFFSET_NB_MES;
  int16_t accOffsetZ = accSumZ / CALIB_OFFSET_NB_MES;

  int16_t gyroOffsetX = gyroSumX / CALIB_OFFSET_NB_MES;
  int16_t gyroOffsetY = gyroSumY / CALIB_OFFSET_NB_MES;
  int16_t gyroOffsetZ = gyroSumZ / CALIB_OFFSET_NB_MES;

  mpu.setXAccelOffset(-accOffsetX);
  mpu.setYAccelOffset(-accOffsetY);
  mpu.setZAccelOffset(-accOffsetZ);

  mpu.setXGyroOffset(-gyroOffsetX);
  mpu.setYGyroOffset(-gyroOffsetY);
  mpu.setZGyroOffset(-gyroOffsetZ);
}

void IMUSpeed::calibrateGyroOffsetsOnly() {
  float gxSum = 0, gySum = 0, gzSum = 0;

  for (int i = 0; i < CALIB_OFFSET_NB_MES; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    gxSum += gx;
    gySum += gy;
    gzSum += gz;
    delay(1);
  }

  int16_t offsetX = gxSum / CALIB_OFFSET_NB_MES;
  int16_t offsetY = gySum / CALIB_OFFSET_NB_MES;
  int16_t offsetZ = gzSum / CALIB_OFFSET_NB_MES;

  mpu.setXGyroOffset(-offsetX);
  mpu.setYGyroOffset(-offsetY);
  mpu.setZGyroOffset(-offsetZ);
}
#undef CALIB_OFFSET_NB_MES

bool IMUSpeed::begin(uint8_t sdaPin, uint8_t sclPin, uint16_t sampleRateHz) {
  Serial.println("Iniciando sensor IMU...");
  Wire.begin(sdaPin, sclPin);
  mpu.initialize();
  if (!mpu.testConnection()) return false;
  filter.begin(sampleRateHz);
  lastTime = millis();
  sampleIntervalMs = 1000 / sampleRateHz;

  Serial.println("Calibrando sensores, mantenha o dispositivo imóvel...");
  calibrateGyroOffsetsOnly();
  Serial.println("Calibração concluída.");

  return true;
}

bool IMUSpeed::update() {
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
  if (abs(accX) < 0.5f) accX = 0;
  if (abs(accY) < 0.5f) accY = 0;
  if (abs(accZ) < 0.5f) accZ = 0;

  if (!accX && !accY) {
      resetSpeed(); // ou vX = vY = vZ = 0;
  }

  filteredAccX = alpha * accX + (1 - alpha) * filteredAccX;
  filteredAccY = alpha * accY + (1 - alpha) * filteredAccY;
  filteredAccZ = alpha * accZ + (1 - alpha) * filteredAccZ;

  vX += filteredAccX * dt;
  vY += filteredAccY * dt;
  vZ += filteredAccZ * dt;

  this->accX = filteredAccX;
  this->accY = filteredAccY;
  this->accZ = filteredAccZ;

  return true;
}

float IMUSpeed::getSpeed() {
  return sqrt(vX * vX + vY * vY + vZ * vZ);
}

float IMUSpeed::getSpeed2D() {
  return sqrt(vX * vX + vY * vY);
}

void IMUSpeed::resetSpeed() {
  vX = vY = vZ = 0;
}

void IMUSpeed::setSpeed(float vx, float vy, float vz) {
  vX = vx;
  vY = vy;
  vZ = vz;
}

// Velocidade
float IMUSpeed::getVX() { return vX; }
float IMUSpeed::getVY() { return vY; }
float IMUSpeed::getVZ() { return vZ; }

// Aceleração
float IMUSpeed::getAccX() { return accX; }
float IMUSpeed::getAccY() { return accY; }
float IMUSpeed::getAccZ() { return accZ; }

// Módulo da aceleração vetorial
float IMUSpeed::getAcc() {
  return sqrt(accX * accX + accY * accY + accZ * accZ);
}

float IMUSpeed::getAcc2D() {
  return sqrt(accX * accX + accY * accY);
}

float ms_to_kmh(float speed_ms) {
  return speed_ms * MS_TO_KMH;
}

void IMUSpeed::printAll() {
  Serial.println("---------");

  Serial.print("Velocidade X: ");
  Serial.print(vX, 3);
  Serial.print(" m/s | ");

  Serial.print("Velocidade Y: ");
  Serial.print(vY, 3);
  Serial.print(" m/s | ");

  Serial.print("Velocidade Z: ");
  Serial.print(vZ, 3);
  Serial.println(" m/s");

  Serial.print("Velocidade total (3D): ");
  Serial.print(getSpeed(), 3);
  Serial.println(" m/s");

  Serial.print("Velocidade total (2D): ");
  Serial.print(getSpeed2D(), 3);
  Serial.println(" m/s");

  Serial.print("Aceleracao X: ");
  Serial.print(accX, 3);
  Serial.print(" m/s² | ");

  Serial.print("Aceleracao Y: ");
  Serial.print(accY, 3);
  Serial.print(" m/s² | ");

  Serial.print("Aceleracao Z: ");
  Serial.print(accZ, 3);
  Serial.println(" m/s²");

  Serial.print("Modulo da aceleracao (3D): ");
  Serial.print(getAcc(), 3);
  Serial.println(" m/s²");

  Serial.print("Modulo da aceleracao (2D): ");
  Serial.print(getAcc2D(), 3);
  Serial.println(" m/s²");

  Serial.println("---------");
}

void IMUSpeed::printTotals() {
  Serial.println("---------");

  Serial.print("Velocidade total (3D): ");
  Serial.print(getSpeed(), 3);
  Serial.println(" m/s");

  Serial.print("Modulo da aceleracao (3D): ");
  Serial.print(getAcc(), 3);
  Serial.println(" m/s²");

  Serial.println("---------");
}

void IMUSpeed::printTotals2D() {
  Serial.println("---------");

  Serial.print("Velocidade total (2D): ");
  Serial.print(getSpeed2D(), 3);
  Serial.println(" m/s");

  Serial.print("Modulo da aceleracao (2D): ");
  Serial.print(getAcc2D(), 3);
  Serial.println(" m/s²");

  Serial.println("---------");
}
