// AlashMPU: датчик не работает? Скетч покажет список устройств на шине I2C, модель датчика
// и текущие значения, а также подскажет, что проверить.
#include <AlashMPU.h>

AlashMPU mpu;

void setup() {
  Serial.begin(115200);
  delay(500);
  mpu.setLanguage(AlashMPU::LANG_RU);     // LANG_KK: қазақша, LANG_EN: English
  bool ok = mpu.begin();
  Serial.println(ok ? "Датчик найден" : "Датчик НЕ найден");
  mpu.diagnose(Serial);
}

void loop() {}
