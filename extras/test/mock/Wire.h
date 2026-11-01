// Имитация I2C и MPU6050 (карта регистров) для тестов.
#pragma once
#include "Arduino.h"
struct FakeMpu {
  bool present = true; uint8_t addr = 0x68; uint8_t who = 0x68;
  uint8_t reg[256] = {0};
  int16_t ax = 0, ay = 0, az = 16384, temp = 0, gx = 0, gy = 0, gz = 0;   // «сырые» значения
  void pack() {
    int16_t v[7] = {ax, ay, az, temp, gx, gy, gz};
    for (int i = 0; i < 7; i++) { reg[0x3B + 2 * i] = (uint16_t)v[i] >> 8; reg[0x3C + 2 * i] = v[i] & 0xFF; }
  }
};
inline FakeMpu g_mpu;
inline int g_reads = 0;                       // сколько раз читали блок данных
inline void (*g_onRead)() = nullptr;          // вызывается при каждом чтении данных (имитация движения)
class TwoWire {
 public:
  void begin() {}
  void begin(int, int) {}
  void beginTransmission(uint8_t a) { _a = a; _n = 0; }
  size_t write(uint8_t v) { if (_n == 0) _reg = v; else g_mpu.reg[_reg + _n - 1] = v; _n++; return 1; }
  uint8_t endTransmission(bool = true) { return (g_mpu.present && _a == g_mpu.addr) ? 0 : 2; }
  uint8_t requestFrom(uint8_t a, uint8_t len) {
    if (!(g_mpu.present && a == g_mpu.addr)) return 0;
    if (_reg == 0x3B) { g_reads++; if (g_onRead) g_onRead(); }
    g_mpu.pack(); g_mpu.reg[0x75] = g_mpu.who;
    for (uint8_t i = 0; i < len; i++) _buf[i] = g_mpu.reg[_reg + i];
    _len = len; _pos = 0; return len;
  }
  int read() { return _pos < _len ? _buf[_pos++] : -1; }
 private:
  uint8_t _a = 0, _reg = 0, _n = 0, _buf[32], _len = 0, _pos = 0;
};
inline TwoWire Wire;
