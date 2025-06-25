#ifndef IMUSPEED_LOCAL_H
#define IMUSPEED_LOCAL_H

#ifndef MS_TO_KMH
#define MS_TO_KMH 3.6f
#endif


#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <MadgwickAHRS.h>

class IMUSpeed {
  public:
    IMUSpeed();

    // Inicializa sensor com pinos SDA, SCL e sample rate (Hz)
    bool begin(uint8_t sdaPin = 8, uint8_t sclPin = 9, uint16_t sampleRateHz = 100);

    // Atualiza dados (retorna true se atualizou)
    bool update();

    // Retorna módulo da velocidade (m/s)
    float getSpeed();

    float getSpeed2D(); // velocidade no plano XY

    // Reseta velocidade (zera integração)
    void resetSpeed();

    // Define as velocidades manualmente (m/s)
    void setSpeed(float vx, float vy, float vz);

    float getVX();
    float getVY();
    float getVZ();
    float getAccX();
    float getAccY();
    float getAccZ();
    float getAcc();
    float getAcc2D();

    void printAll();
    void printTotals();
    void printTotals2D();

  private:
    MPU6050 mpu;
    Madgwick filter;

    float vX, vY, vZ;
    float accX, accY, accZ;
    unsigned long lastTime;
    unsigned long sampleIntervalMs;

    float filteredAccX = 0, filteredAccY = 0, filteredAccZ = 0;
    float alpha = 0.1f; // entre 0.05 e 0.2 geralmente funciona bem

    // Métodos de calibração
    void calibrateSensorOffsets();
    void calibrateGyroOffsetsOnly();
};

float ms_to_kmh(float speed_ms);


#endif 