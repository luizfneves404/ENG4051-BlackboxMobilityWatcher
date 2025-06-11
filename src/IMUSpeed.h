#ifndef IMUSPEED_H
#define IMUSPEED_H

#include <Wire.h>
#include <MPU6050.h>
#include <MadgwickAHRS.h>

class IMUSpeed {
  public:
    IMUSpeed();

    // Inicializa sensor com pinos SDA, SCL e sample rate (Hz)
    bool begin(uint8_t sdaPin = 21, uint8_t sclPin = 22, uint16_t sampleRateHz = 100);

    // Atualiza dados (retorna true se atualizou)
    bool SpeedUpdate();

    // Retorna módulo da velocidade (m/s)
    float getSpeed();

    // Reseta velocidade (zera integração)
    void resetSpeed();

    // Define as velocidades manualmente (m/s)
    void setSpeed(float vx, float vy, float vz);

  private:
    MPU6050 mpu;
    Madgwick filter;

    float vX, vY, vZ;
    unsigned long lastTime;
    unsigned long sampleIntervalMs;
};

#endif
