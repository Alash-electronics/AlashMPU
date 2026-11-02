// [ru] AlashMPU: углы наклона. Крен (roll) и тангаж (pitch) в градусах, как в "электронном уровне".
//      Фильтр сам объединяет гироскоп (быстрый, но уплывает) и акселерометр (медленный, но точный).
//      Вызывать update() не нужно.
// [kk] AlashMPU: еңкею бұрыштары. Крен (roll) және тангаж (pitch) градуспен, «электрондық деңгей» сияқты.
//      Сүзгі гироскопты (жылдам, бірақ ығысады) және акселерометрді (баяу, бірақ дәл) өзі біріктіреді.
//      update() шақырудың қажеті жоқ.
// [en] AlashMPU: tilt angles. Roll and pitch in degrees, like an "electronic level".
//      The filter combines the gyroscope (fast but drifts) and the accelerometer (slow but accurate) itself.
//      There is no need to call update().
#include <AlashMPU.h>

AlashMPU mpu;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println(mpu.errorText());
    while (true) delay(1000);
  }
  mpu.calibrate();
}

void loop() {
  Serial.print("Крен: ");
  Serial.print(mpu.roll(), 1);
  Serial.print("   Тангаж: ");
  Serial.print(mpu.pitch(), 1);
  Serial.print("   Наклон от вертикали: ");
  Serial.print(mpu.tilt(), 1);
  Serial.println(mpu.tilt() < 2.0 ? "   ровно" : "");
  delay(100);
}
