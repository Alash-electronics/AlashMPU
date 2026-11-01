// AlashMPU: зачем нужна калибровка. Скетч показывает значения в покое до и после calibrate().
// Положите датчик ровно (ось Z вверх) и не двигайте. До калибровки гироскоп в покое показывает
// не ноль, а ускорение по Z не ровно 1 g. После калибровки значения близки к 0, 0, 0 и 0, 0, 1.
#include <AlashMPU.h>

AlashMPU mpu;

void show(const char *title) {
  Serial.println(title);
  Serial.print("  Ускорение g: ");
  Serial.print(mpu.accelX(), 3); Serial.print("  ");
  Serial.print(mpu.accelY(), 3); Serial.print("  ");
  Serial.println(mpu.accelZ(), 3);
  Serial.print("  Вращение °/с: ");
  Serial.print(mpu.gyroX(), 2); Serial.print("  ");
  Serial.print(mpu.gyroY(), 2); Serial.print("  ");
  Serial.println(mpu.gyroZ(), 2);
}

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println(mpu.errorText());
    while (true) delay(1000);
  }
  delay(500);
  show("До калибровки:");
  Serial.println("Калибрую, не двигайте датчик...");
  if (mpu.calibrate(1500)) show("После калибровки:");
  else Serial.println(mpu.errorText());   // например, датчик двигали
}

void loop() {}
