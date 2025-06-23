/* #include "IMUSpeed.h"

IMUSpeed imu;

void setup() {
  Serial.begin(9600);
  if (!imu.begin(21, 22, 100)) {  // SDA, SCL e sample rate 100 Hz
    Serial.println("Falha ao iniciar MPU6050");
    while (1);
  }
  Serial.println("MPU6050 iniciado com sucesso");
}

void loop() {
  if (imu.SpeedUpdate()) {
    float speed = imu.getSpeed();
    Serial.print("Velocidade: ");
    Serial.print(speed, 3);
    Serial.println(" m/s");
  }
  // Aqui você pode fazer outras coisas sem bloquear o loop
}
 */