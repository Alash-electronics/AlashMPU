// AlashMPU: самый простой пример. Ускорение, вращение и температура с датчика MPU6050 (плата GY-521).
//
// Подключение: VCC к 3.3 В (на плате GY-521 можно и 5 В), GND к GND,
// SDA к A4 (ESP32: GPIO21), SCL к A5 (ESP32: GPIO22). Вывод AD0 не подключайте (адрес 0x68).
#include <AlashMPU.h>

AlashMPU mpu;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {                     // сам находит датчик и его модель
    Serial.println(mpu.errorText());      // объяснит, что проверить
    while (true) delay(1000);
  }
  Serial.println("Положите датчик ровно и не трогайте секунду...");
  mpu.calibrate();                        // убирает смещение нуля
}

void loop() {
  Serial.print("Ускорение g: ");
  Serial.print(mpu.accelX()); Serial.print("  ");
  Serial.print(mpu.accelY()); Serial.print("  ");
  Serial.print(mpu.accelZ());
  Serial.print("   Вращение °/с: ");
  Serial.print(mpu.gyroX()); Serial.print("  ");
  Serial.print(mpu.gyroY()); Serial.print("  ");
  Serial.print(mpu.gyroZ());
  Serial.print("   Температура: ");
  Serial.println(mpu.temperature());
  delay(200);
}
