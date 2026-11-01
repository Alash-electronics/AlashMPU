// Тесты AlashMPU без железа (имитация регистров MPU6050 и времени):
//   g++ -std=c++17 -Wall -I mock -I ../../src test_mpu.cpp ../../src/AlashMPU.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "Wire.h"
#include "AlashMPU.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static bool near(float a, float b, float eps) { return fabsf(a - b) <= eps; }
static void reset() { g_mpu = FakeMpu(); g_reads = 0; g_onRead = nullptr; g_us = 1000000; }
// Несколько обновлений с шагом 10 мс (100 Гц)
static void run(AlashMPU &m, int steps) { for (int i = 0; i < steps; i++) { g_us += 10000; m.update(); } }

static int toggle = 0;
static void shake() { toggle ^= 1; g_mpu.gx = toggle ? 1000 : -1000; }   // ±15 °/с при 500 °/с

int main() {
  // 1) определение чипа и адреса
  reset(); AlashMPU a; CHECK(a.begin()); CHECK(a.chip() == AlashMPU::CHIP_MPU6050 && a.address() == 0x68);
  reset(); g_mpu.addr = 0x69; AlashMPU b; CHECK(b.begin()); CHECK(b.address() == 0x69);
  reset(); g_mpu.who = 0x71; AlashMPU c; CHECK(c.begin() && c.chip() == AlashMPU::CHIP_MPU9250);
  reset(); g_mpu.who = 0x70; AlashMPU c2; CHECK(c2.begin() && c2.chip() == AlashMPU::CHIP_MPU6500);
  reset(); g_mpu.who = 0x98; AlashMPU c3; CHECK(c3.begin() && c3.chip() == AlashMPU::CHIP_MPU6050);
  reset(); g_mpu.who = 0x55; AlashMPU w; CHECK(!w.begin() && w.error() == AlashMPU::ERR_WRONG_CHIP);
  reset(); g_mpu.present = false; AlashMPU n; CHECK(!n.begin() && n.error() == AlashMPU::ERR_NO_DEVICE);
  CHECK(isnan(n.tilt()) || n.error() == AlashMPU::ERR_NO_DEVICE);

  // 2) единицы: ±4 g по умолчанию (8192 на g), ±500 °/с (65.5 на °/с)
  reset(); AlashMPU m; m.begin();
  g_mpu.ax = 4096; g_mpu.ay = -8192; g_mpu.az = 8192; g_mpu.gx = 655; g_mpu.gy = -131; g_mpu.gz = 0; g_mpu.temp = 340;
  g_us += 10000;
  CHECK(near(m.accelX(), 0.5f, 0.001f) && near(m.accelY(), -1.0f, 0.001f) && near(m.accelZ(), 1.0f, 0.001f));
  CHECK(near(m.gyroX(), 10.0f, 0.01f) && near(m.gyroY(), -2.0f, 0.01f));
  CHECK(near(m.temperature(), 37.53f, 0.01f));
  CHECK(m.setAccelRange(2) && m.setGyroRange(250));
  g_us += 10000;
  CHECK(near(m.accelX(), 0.25f, 0.001f) && near(m.gyroX(), 5.0f, 0.01f));     // тот же сырой код при другом диапазоне
  CHECK(!m.setAccelRange(3) && !m.setGyroRange(300));
  CHECK(g_mpu.reg[0x1C] == (0 << 3) && g_mpu.reg[0x1B] == (0 << 3));           // диапазон записан в датчик
  m.setAccelRange(16); CHECK(g_mpu.reg[0x1C] == (3 << 3));
  m.setGyroRange(2000); CHECK(g_mpu.reg[0x1B] == (3 << 3));
  reset(); g_mpu.who = 0x71; AlashMPU t; t.begin(); g_mpu.temp = 0; g_us += 10000;
  CHECK(near(t.temperature(), 21.0f, 0.01f));                                  // у MPU6500/9250 другая формула

  // 3) углы по ускорению: тангаж 30°, крен 45°, наклон от вертикали
  reset(); AlashMPU p; p.begin();
  g_mpu.ax = -4096; g_mpu.ay = 0; g_mpu.az = (int16_t)(8192 * 0.8660254f);    // тангаж +30°
  g_us += 10000;
  CHECK(near(p.pitch(), 30.0f, 0.1f) && near(p.roll(), 0.0f, 0.1f));
  CHECK(near(p.tilt(), 30.0f, 0.1f) && near(p.accelMagnitude(), 1.0f, 0.01f));
  reset(); AlashMPU r; r.begin();
  g_mpu.ax = 0; g_mpu.ay = 5793; g_mpu.az = 5793;                              // крен +45°
  g_us += 10000;
  CHECK(near(r.roll(), 45.0f, 0.1f) && near(r.pitch(), 0.0f, 0.1f));
  g_mpu.ay = 5793; g_mpu.az = -5793; reset(); AlashMPU up; up.begin();         // вверх ногами
  g_mpu.ay = 0; g_mpu.az = -8192; g_us += 10000;
  CHECK(near(fabsf(up.roll()), 180.0f, 0.5f) && near(up.tilt(), 180.0f, 0.5f));

  // 4) фильтр: alpha=0 это чистый акселерометр, alpha=1 чистый гироскоп
  reset(); AlashMPU f0; f0.begin(); f0.setFilter(0.0f);
  g_mpu.az = 8192; f0.update(); g_mpu.ay = 5793; g_mpu.az = 5793; run(f0, 3);
  CHECK(near(f0.roll(), 45.0f, 0.1f));
  reset(); AlashMPU f1; f1.begin(); f1.setFilter(1.0f);
  g_mpu.az = 8192; f1.update(); g_mpu.gx = 655;                                // 10 °/с
  run(f1, 100);                                                                  // 1 секунда
  CHECK(near(f1.roll(), 10.0f, 0.3f));

  // 5) сходимость: после резкого поворота стенда по ускорению фильтр приходит к реальному углу
  reset(); AlashMPU f2; f2.begin();
  g_mpu.az = 8192; f2.update();
  g_mpu.ay = 5793; g_mpu.az = 5793;
  run(f2, 400);                                                                  // 4 секунды
  CHECK(near(f2.roll(), 45.0f, 1.0f));

  // 6) рыскание: интегрирование гироскопа, сброс, переход через 180
  reset(); AlashMPU y; y.begin(); y.update();
  g_mpu.gz = 655; run(y, 100);
  CHECK(near(y.yaw(), 10.0f, 0.3f));
  y.resetYaw(); CHECK(y.yaw() == 0.0f);
  g_mpu.gz = 6550; run(y, 100);                                                  // 100 °/с ещё секунда: 100
  g_mpu.gz = 6550; run(y, 100);                                                  // 200 °/с всего -> -160
  CHECK(near(y.yaw(), -160.0f, 1.5f));

  // 7) длинная пауза не даёт скачок гироскопа
  reset(); AlashMPU g; g.begin(); g.update();
  g_mpu.gz = 655; g_us += 2000000; g.update();                                   // 2 секунды без опроса
  CHECK(near(g.yaw(), 0.0f, 0.01f));

  // 8) калибровка: смещения гироскопа и акселерометра исчезают
  reset(); AlashMPU k; k.begin();
  g_mpu.gx = 131; g_mpu.gy = -66; g_mpu.gz = 33;                                // +2, -1, +0.5 °/с в покое
  g_mpu.ax = 400; g_mpu.ay = -200; g_mpu.az = 8192 + 300;
  g_us += 10000;
  CHECK(fabsf(k.gyroX()) > 1.0f);
  CHECK(k.calibrate(500));
  g_us += 10000;
  CHECK(near(k.gyroX(), 0.0f, 0.05f) && near(k.gyroY(), 0.0f, 0.05f) && near(k.gyroZ(), 0.0f, 0.05f));
  CHECK(near(k.accelX(), 0.0f, 0.002f) && near(k.accelY(), 0.0f, 0.002f) && near(k.accelZ(), 1.0f, 0.002f));
  CHECK(near(k.roll(), 0.0f, 0.2f) && near(k.pitch(), 0.0f, 0.2f));

  // 9) калибровка при движении отклоняется, прежние значения остаются
  reset(); AlashMPU mv; mv.begin(); g_onRead = shake;
  CHECK(!mv.calibrate(300) && mv.error() == AlashMPU::ERR_MOVING);
  g_onRead = nullptr;

  // 10) калибровка не лежащего ровно датчика: только гироскоп
  reset(); AlashMPU tl; tl.begin();
  g_mpu.ax = 4096; g_mpu.az = 7094; g_mpu.gx = 131;
  CHECK(tl.calibrate(300));
  g_us += 10000;
  CHECK(near(tl.accelX(), 0.5f, 0.002f) && near(tl.gyroX(), 0.0f, 0.05f));

  // 11) датчик опрашивается не чаще раза в 5 мс
  reset(); AlashMPU rate; rate.begin(); g_us += 10000;
  g_reads = 0;
  rate.accelX(); rate.accelY(); rate.gyroZ(); rate.pitch();
  CHECK(g_reads == 1);
  g_us += 6000; rate.accelX();
  CHECK(g_reads == 2);

  // 12) isStill и пропадание датчика
  reset(); AlashMPU st; st.begin(); g_us += 10000;
  CHECK(st.isStill());
  g_mpu.gx = 6550; g_us += 10000; CHECK(!st.isStill());
  g_mpu.present = false; g_us += 10000; st.update();
  CHECK(st.error() == AlashMPU::ERR_I2C && !st.isConnected());

  // 13) тексты на трёх языках и diagnose
  reset(); AlashMPU d; d.begin(); g_mpu.gx = 400;
  d.setLanguage(AlashMPU::LANG_KK); const char *kk = (const char *)d.errorText();
  d.setLanguage(AlashMPU::LANG_EN); d.diagnose(*(new Print));
  CHECK(kk != nullptr);

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
