// [ru] AlashMPU: датчик не работает? Скетч покажет список устройств на шине I2C, модель датчика
//      и текущие значения, а также подскажет, что проверить.
// [kk] AlashMPU: датчик жұмыс істемей ме? Скетч I2C шинасындағы құрылғылар тізімін, датчик моделін
//      және ағымдағы мәндерді көрсетеді, сондай-ақ нені тексеру керегін айтады.
// [en] AlashMPU: the sensor does not work? The sketch shows the list of devices on the I2C bus, the sensor model
//      and the current values, and tells what to check.
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
