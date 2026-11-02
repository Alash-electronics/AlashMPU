// [ru] AlashMPU: зачем нужна калибровка. Скетч показывает значения в покое до и после calibrate().
//      Положите датчик ровно (ось Z вверх) и не двигайте. До калибровки гироскоп в покое показывает
//      не ноль, а ускорение по Z не ровно 1 g. После калибровки значения близки к 0, 0, 0 и 0, 0, 1.
// [kk] AlashMPU: калибрлеу не үшін керек. Скетч тыныштықтағы мәндерді calibrate()-ке дейін және кейін көрсетеді.
//      Датчикті тегіс қойыңыз (Z осі жоғары) және қозғамаңыз. Калибрлеуге дейін тыныштықтағы гироскоп
//      нөлді емес көрсетеді, ал Z бойынша үдеу дәл 1 g емес. Калибрлеуден кейін мәндер 0, 0, 0 және 0, 0, 1-ге жақын.
// [en] AlashMPU: why calibration is needed. The sketch shows the values at rest before and after calibrate().
//      Put the sensor level (Z axis up) and do not move it. Before calibration the gyroscope at rest shows
//      not zero, and the Z acceleration is not exactly 1 g. After calibration the values are close to 0, 0, 0 and 0, 0, 1.
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
