// MIT License. Alash Electronics.
#include "AlashMPU.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

static const uint8_t REG_SMPLRT_DIV = 0x19;
static const uint8_t REG_CONFIG = 0x1A;
static const uint8_t REG_GYRO_CONFIG = 0x1B;
static const uint8_t REG_ACCEL_CONFIG = 0x1C;
static const uint8_t REG_ACCEL_CONFIG2 = 0x1D;   // только MPU6500 / MPU9250
static const uint8_t REG_ACCEL_XOUT_H = 0x3B;
static const uint8_t REG_PWR_MGMT_1 = 0x6B;
static const uint8_t REG_WHO_AM_I = 0x75;
static const float RAD2DEG = 57.2957795f;

// ---- Запуск --------------------------------------------------------------------

bool AlashMPU::begin(TwoWire &wire, bool startWire) {
  _wire = &wire;
  if (startWire) _wire->begin();
  _chip = CHIP_NONE;
  Error first = ERR_NO_DEVICE;
  for (uint8_t a = 0x68; a <= 0x69; a++) {
    if (probe(a)) return true;
    if (_err == ERR_WRONG_CHIP) first = ERR_WRONG_CHIP;
  }
  _err = first;
  return false;
}

bool AlashMPU::begin(uint8_t address, TwoWire &wire, bool startWire) {
  _wire = &wire;
  if (startWire) _wire->begin();
  _chip = CHIP_NONE;
  return probe(address);
}

#if defined(ESP32) || defined(ESP8266)
bool AlashMPU::begin(int sdaPin, int sclPin) {
  Wire.begin(sdaPin, sclPin);
  return begin(Wire, false);
}
#endif

bool AlashMPU::probe(uint8_t address) {
  _addr = address;
  _wire->beginTransmission(address);
  if (_wire->endTransmission() != 0) {
    _err = ERR_NO_DEVICE;
    return false;
  }
  uint8_t id = 0;
  if (!read8(REG_WHO_AM_I, id)) {
    _err = ERR_I2C;
    return false;
  }
  // Код в регистре WHO_AM_I. 0x98 и 0x70 встречаются у плат, проданных как MPU6050.
  if (id == 0x68 || id == 0x98) _chip = CHIP_MPU6050;
  else if (id == 0x70) _chip = CHIP_MPU6500;
  else if (id == 0x71 || id == 0x73) _chip = CHIP_MPU9250;
  else {
    _chip = CHIP_NONE;
    _err = ERR_WRONG_CHIP;
    return false;
  }
  if (!write8(REG_PWR_MGMT_1, 0x80)) { _chip = CHIP_NONE; _err = ERR_I2C; return false; }   // сброс
  delay(100);
  write8(REG_PWR_MGMT_1, 0x01);                       // проснуться, тактирование от гироскопа (точнее встроенного)
  delay(10);
  write8(REG_CONFIG, 0x03);                           // фильтр гироскопа 41 Гц: меньше шума
  if (_chip != CHIP_MPU6050) write8(REG_ACCEL_CONFIG2, 0x03);
  write8(REG_SMPLRT_DIV, 4);
  setAccelRange(4);
  setGyroRange(500);
  _hasData = false;
  _anglesInit = false;
  _err = ERR_OK;
  return true;
}

bool AlashMPU::isConnected() {
  if (_chip == CHIP_NONE) return false;
  uint8_t id = 0;
  return read8(REG_WHO_AM_I, id);
}

const __FlashStringHelper *AlashMPU::chipName() const {
  switch (_chip) {
    case CHIP_MPU6050: return F("MPU6050");
    case CHIP_MPU6500: return F("MPU6500");
    case CHIP_MPU9250: return F("MPU9250");
    default: return F("-");
  }
}

// ---- Настройки ----------------------------------------------------------------------

bool AlashMPU::setAccelRange(uint8_t g) {
  uint8_t code = g == 2 ? 0 : g == 4 ? 1 : g == 8 ? 2 : g == 16 ? 3 : 255;
  if (code == 255) return false;
  _accelScale = 16384.0f / (1 << code);
  return _chip == CHIP_NONE ? true : write8(REG_ACCEL_CONFIG, code << 3);
}

bool AlashMPU::setGyroRange(uint16_t dps) {
  uint8_t code = dps == 250 ? 0 : dps == 500 ? 1 : dps == 1000 ? 2 : dps == 2000 ? 3 : 255;
  if (code == 255) return false;
  _gyroScale = 131.0f / (1 << code);
  return _chip == CHIP_NONE ? true : write8(REG_GYRO_CONFIG, code << 3);
}

// ---- Чтение -------------------------------------------------------------------------

bool AlashMPU::readSample() {
  uint8_t b[14];
  if (!readBytes(REG_ACCEL_XOUT_H, b, 14)) {
    _err = ERR_I2C;
    return false;
  }
  for (uint8_t i = 0; i < 7; i++) _raw[i] = (int16_t)((b[2 * i] << 8) | b[2 * i + 1]);
  return true;
}

void AlashMPU::convert() {
  _ax = _raw[0] / _accelScale - _offAx;
  _ay = _raw[1] / _accelScale - _offAy;
  _az = _raw[2] / _accelScale - _offAz;
  _tempC = _chip == CHIP_MPU6050 ? _raw[3] / 340.0f + 36.53f : _raw[3] / 333.87f + 21.0f;
  _gx = _raw[4] / _gyroScale - _offGx;
  _gy = _raw[5] / _gyroScale - _offGy;
  _gz = _raw[6] / _gyroScale - _offGz;
}

// Разница двух углов, приведённая к -180...180 (чтобы фильтр не ломался на переходе через 180)
static float wrap180(float a) {
  while (a > 180.0f) a -= 360.0f;
  while (a < -180.0f) a += 360.0f;
  return a;
}

void AlashMPU::update() {
  if (_chip == CHIP_NONE) {
    _err = ERR_NO_DEVICE;
    return;
  }
  if (!readSample()) return;
  uint32_t now = micros();
  float dt = _hasData ? (uint32_t)(now - _lastUs) / 1e6f : 0.0f;
  if (dt > 0.5f) dt = 0.0f;                           // долгая пауза: гироскоп не интегрируем, берём углы по ускорению
  _lastUs = now;
  _lastReadMs = millis();
  _hasData = true;
  _err = ERR_OK;
  convert();

  float accRoll = atan2f(_ay, _az) * RAD2DEG;
  float accPitch = atan2f(-_ax, sqrtf(_ay * _ay + _az * _az)) * RAD2DEG;
  if (!_anglesInit || dt == 0.0f) {
    if (!_anglesInit) { _roll = accRoll; _pitch = accPitch; }
    _anglesInit = true;
  } else {
    _roll += _gx * dt;
    _roll += (1.0f - _alpha) * wrap180(accRoll - _roll);
    _roll = wrap180(_roll);
    _pitch += _gy * dt;
    _pitch += (1.0f - _alpha) * (accPitch - _pitch);
  }
  _yaw = wrap180(_yaw + _gz * dt);
}

void AlashMPU::ensureFresh() {
  if (_hasData && (uint32_t)(micros() - _lastUs) < 5000) return;   // не чаще раза в 5 мс
  update();
}

float AlashMPU::accelX() { ensureFresh(); return _ax; }
float AlashMPU::accelY() { ensureFresh(); return _ay; }
float AlashMPU::accelZ() { ensureFresh(); return _az; }
float AlashMPU::gyroX() { ensureFresh(); return _gx; }
float AlashMPU::gyroY() { ensureFresh(); return _gy; }
float AlashMPU::gyroZ() { ensureFresh(); return _gz; }
float AlashMPU::temperature() { ensureFresh(); return _tempC; }
float AlashMPU::roll() { ensureFresh(); return _roll; }
float AlashMPU::pitch() { ensureFresh(); return _pitch; }
float AlashMPU::yaw() { ensureFresh(); return _yaw; }
float AlashMPU::accelMagnitude() { ensureFresh(); return sqrtf(_ax * _ax + _ay * _ay + _az * _az); }
bool AlashMPU::isStill(float limit) {
  ensureFresh();
  return fabsf(_gx) < limit && fabsf(_gy) < limit && fabsf(_gz) < limit;
}
float AlashMPU::tilt() {
  ensureFresh();
  float m = sqrtf(_ax * _ax + _ay * _ay + _az * _az);
  if (m < 0.01f) return NAN;
  float c = _az / m;
  return acosf(c > 1.0f ? 1.0f : c < -1.0f ? -1.0f : c) * RAD2DEG;
}

// ---- Калибровка ---------------------------------------------------------------------------

bool AlashMPU::calibrate(uint16_t durationMs) {
  if (_chip == CHIP_NONE) {
    _err = ERR_NO_DEVICE;
    return false;
  }
  _offAx = _offAy = _offAz = _offGx = _offGy = _offGz = 0;   // считаем по «сырым» значениям
  double sum[6] = {0, 0, 0, 0, 0, 0};
  float gMin[3] = {1e9f, 1e9f, 1e9f}, gMax[3] = {-1e9f, -1e9f, -1e9f};
  uint16_t n = 0;
  uint32_t t0 = millis();
  while ((uint32_t)(millis() - t0) < durationMs || n < 10) {
    if (!readSample()) return false;
    convert();
    float v[6] = {_ax, _ay, _az, _gx, _gy, _gz};
    for (uint8_t i = 0; i < 6; i++) sum[i] += v[i];
    for (uint8_t i = 0; i < 3; i++) {
      if (v[3 + i] < gMin[i]) gMin[i] = v[3 + i];
      if (v[3 + i] > gMax[i]) gMax[i] = v[3 + i];
    }
    n++;
    delay(3);
  }
  for (uint8_t i = 0; i < 3; i++) {
    if (gMax[i] - gMin[i] > 8.0f) {                   // за секунду гироскоп гулял больше 8 °/с: датчик двигали
      _err = ERR_MOVING;
      return false;
    }
  }
  float mean[6];
  for (uint8_t i = 0; i < 6; i++) mean[i] = (float)(sum[i] / n);
  _offGx = mean[3]; _offGy = mean[4]; _offGz = mean[5];
  // Ускорение: если датчик лежит ровно (Z вверх), то в покое должно быть 0, 0, 1 g. Иначе нулевое смещение не определить.
  if (fabsf(mean[0]) < 0.3f && fabsf(mean[1]) < 0.3f && mean[2] > 0.8f && mean[2] < 1.2f) {
    _offAx = mean[0]; _offAy = mean[1]; _offAz = mean[2] - 1.0f;
  }
  _anglesInit = false;
  _hasData = false;
  _yaw = 0;
  _err = ERR_OK;
  return true;
}

// ---- I2C -------------------------------------------------------------------------------------

bool AlashMPU::read8(uint8_t reg, uint8_t &v) { return readBytes(reg, &v, 1); }

bool AlashMPU::readBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) return false;
  if (_wire->requestFrom(_addr, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = (uint8_t)_wire->read();
  return true;
}

bool AlashMPU::write8(uint8_t reg, uint8_t v) {
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  _wire->write(v);
  return _wire->endTransmission() == 0;
}

// ---- Диагностика -----------------------------------------------------------------------------

const __FlashStringHelper *AlashMPU::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_DEVICE:
      return T("Датчик не найден: проверьте провода, питание 3.3 В и пины SDA / SCL",
               "Сенсор табылмады: сымдарды, қуатты (3.3 В) және SDA / SCL пиндерін тексеріңіз",
               "Sensor not found: check wiring, 3.3 V power and the SDA / SCL pins");
    case ERR_WRONG_CHIP:
      return T("По адресу 0x68 / 0x69 отвечает другое устройство (например, часы DS3231 тоже 0x68). Подключите вывод AD0 датчика к 3.3 В: адрес станет 0x69",
               "0x68 / 0x69 мекенжайында басқа құрылғы жауап береді (мысалы, DS3231 сағаты да 0x68). Сенсордың AD0 шығысын 3.3 В-қа қосыңыз: мекенжай 0x69 болады",
               "Another device answers at 0x68 / 0x69 (for example a DS3231 clock is also 0x68). Connect the sensor AD0 pin to 3.3 V: the address becomes 0x69");
    case ERR_I2C:
      return T("Ошибка I2C: проверьте контакты и подтяжку линий", "I2C қатесі: байланысты және тартқыш резисторларды тексеріңіз",
               "I2C error: check connections and pull-up resistors");
    case ERR_MOVING:
      return T("Калибровка не удалась: датчик двигали. Положите его ровно на стол и не трогайте секунду",
               "Калибрлеу сәтсіз: сенсорды қозғалттыңыз. Оны үстелге тегіс қойып, бір секунд қозғамаңыз",
               "Calibration failed: the sensor was moved. Put it flat on the table and do not touch it for a second");
  }
  return F("");
}

void AlashMPU::diagnose(Print &out) {
  out.println(T("AlashMPU: диагностика", "AlashMPU: диагностика", "AlashMPU: diagnostics"));
  out.println(T("Найденные устройства I2C:", "Табылған I2C құрылғылары:", "I2C devices found:"));
  for (uint8_t a = 1; a < 127; a++) {
    _wire->beginTransmission(a);
    if (_wire->endTransmission() == 0) {
      out.print(F("  0x"));
      if (a < 16) out.print('0');
      out.print(a, HEX);
      if (a == 0x68 || a == 0x69) out.print(F("  MPU6050 / MPU9250 / DS3231"));
      else if (a == 0x76 || a == 0x77) out.print(F("  BME280 / BMP280"));
      else if (a == 0x3C || a == 0x3D) out.print(F("  OLED SSD1306"));
      else if (a == 0x27 || a == 0x3F) out.print(F("  LCD I2C"));
      else if (a == 0x40) out.print(F("  PCA9685 / INA219"));
      out.println();
    }
  }
  if (_chip == CHIP_NONE) {
    out.println(errorText());
    return;
  }
  out.print(T("Датчик: ", "Сенсор: ", "Sensor: "));
  out.print(chipName());
  out.print(F(", 0x"));
  out.println(_addr, HEX);
  update();
  if (_err != ERR_OK) {
    out.println(errorText());
    return;
  }
  out.print(T("Ускорение X Y Z (g): ", "Үдеу X Y Z (g): ", "Acceleration X Y Z (g): "));
  out.print(_ax); out.print(F(" ")); out.print(_ay); out.print(F(" ")); out.println(_az);
  out.print(T("Вращение X Y Z (°/с): ", "Айналу X Y Z (°/с): ", "Rotation X Y Z (deg/s): "));
  out.print(_gx); out.print(F(" ")); out.print(_gy); out.print(F(" ")); out.println(_gz);
  out.print(T("Крен / тангаж: ", "Бүйір / бойлық ауытқу: ", "Roll / pitch: "));
  out.print(_roll); out.print(F(" / ")); out.println(_pitch);
  float m = sqrtf(_ax * _ax + _ay * _ay + _az * _az);
  if (fabsf(_gx) < 3 && fabsf(_gy) < 3 && fabsf(_gz) < 3 && (m < 0.85f || m > 1.15f))
    out.println(T("В покое длина ускорения должна быть около 1 g, а здесь нет: проверьте питание 3.3 В и не трясите датчик",
                  "Тыныштықта үдеу ұзындығы шамамен 1 g болуы керек, мұнда олай емес: 3.3 В қуатын тексеріңіз, сенсорды шайқамаңыз",
                  "At rest the acceleration length should be about 1 g, but it is not: check the 3.3 V power and keep the sensor still"));
  if (fabsf(_gx) > 1.5f || fabsf(_gy) > 1.5f || fabsf(_gz) > 1.5f)
    out.println(T("Гироскоп в покое показывает вращение: если датчик лежит неподвижно, вызовите calibrate()",
                  "Гироскоп тыныштықта айналуды көрсетеді: сенсор қозғалмай жатса, calibrate() шақырыңыз",
                  "The gyro shows rotation at rest: if the sensor lies still, call calibrate()"));
}
