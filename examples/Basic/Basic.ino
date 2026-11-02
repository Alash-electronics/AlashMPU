// [ru] AlashMPU: самый простой пример. Ускорение, вращение и температура с датчика MPU6050 (плата GY-521).
//
//      Подключение: VCC к 3.3 В (на плате GY-521 можно и 5 В), GND к GND,
//      SDA к A4 (ESP32: GPIO21), SCL к A5 (ESP32: GPIO22). Вывод AD0 не подключайте (адрес 0x68).
// [kk] AlashMPU: ең қарапайым мысал. MPU6050 датчигінен (GY-521 тақшасы) үдеу, айналу және температура.
//
//      Қосу: VCC 3.3 В-қа (GY-521 тақшасында 5 В да болады), GND GND-ға,
//      SDA A4-ке (ESP32: GPIO21), SCL A5-ке (ESP32: GPIO22). AD0 шықпасын қоспаңыз (мекенжай 0x68).
// [en] AlashMPU: the simplest example. Acceleration, rotation and temperature from an MPU6050 sensor (GY-521 board).
//
//      Wiring: VCC to 3.3 V (the GY-521 board also accepts 5 V), GND to GND,
//      SDA to A4 (ESP32: GPIO21), SCL to A5 (ESP32: GPIO22). Leave the AD0 pin unconnected (address 0x68).
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
