// AlashMPU: гироскоп и акселерометр MPU6050 / MPU6500 / MPU9250 (GY-521): ускорение, вращение, углы наклона с фильтром, калибровка.
// Без зависимостей, только встроенная Wire. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashMPU
#ifndef ALASH_MPU_H
#define ALASH_MPU_H

#include <Arduino.h>
#include <Wire.h>

class AlashMPU {
 public:
  enum Chip : uint8_t { CHIP_NONE = 0, CHIP_MPU6050, CHIP_MPU6500, CHIP_MPU9250 };
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_DEVICE,    // на шине I2C нет ответа по адресам 0x68 и 0x69
    ERR_WRONG_CHIP,   // отвечает другое устройство
    ERR_I2C,          // ошибка чтения или записи по I2C
    ERR_MOVING        // при калибровке датчик двигали
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  // ---- Запуск --------------------------------------------------------------
  // begin() сам находит датчик (адрес 0x68 или 0x69) и определяет модель. После него вызовите calibrate().
  bool begin(TwoWire &wire = Wire, bool startWire = true);
  bool begin(uint8_t address, TwoWire &wire = Wire, bool startWire = true);
#if defined(ESP32) || defined(ESP8266)
  bool begin(int sdaPin, int sclPin);       // свои пины I2C
#endif

  // Калибровка: датчик лежит неподвижно ровно (ось Z вверх) около секунды. Вычитает смещение нуля гироскопа и акселерометра.
  bool calibrate(uint16_t durationMs = 1000);

  // ---- Измерения (датчик опрашивается сам, не чаще раза в 5 мс) ---------------------
  float accelX();                           // ускорение по оси X, g (1 g = 9,81 м/с²)
  float accelY();
  float accelZ();
  float gyroX();                            // скорость вращения вокруг оси X, градусов в секунду
  float gyroY();
  float gyroZ();
  float temperature();                      // температура кристалла, °C

  // ---- Углы (фильтр объединяет гироскоп и акселерометр) -------------------------------
  float roll();                             // крен, градусы: поворот вокруг оси X (-180...180)
  float pitch();                            // тангаж, градусы: поворот вокруг оси Y (-90...90)
  float yaw();                              // рыскание, градусы: считается по гироскопу, со временем уплывает
  void resetYaw() { _yaw = 0; }
  float tilt();                             // угол отклонения оси Z от вертикали, градусы (0 = лежит ровно)
  float accelMagnitude();                   // длина вектора ускорения, g (около 1 в покое)
  bool isStill(float gyroLimit = 3.0f);     // датчик почти не вращается

  void update();                            // прочитать датчик и обновить углы (вызывается сама, вручную не нужно)

  // ---- Настройки -----------------------------------------------------------------
  bool setAccelRange(uint8_t g);            // 2, 4, 8 или 16 g (по умолчанию 4)
  bool setGyroRange(uint16_t dps);          // 250, 500, 1000 или 2000 °/с (по умолчанию 500)
  void setFilter(float alpha) { _alpha = alpha < 0 ? 0 : alpha > 1 ? 1 : alpha; }   // доля гироскопа в фильтре, по умолчанию 0.98
  void setLanguage(Language lang) { _lang = lang; }

  // ---- Состояние и диагностика --------------------------------------------------------
  bool isConnected();
  Chip chip() const { return _chip; }
  const __FlashStringHelper *chipName() const;
  uint8_t address() const { return _addr; }
  Error error() const { return _err; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);

 private:
  TwoWire *_wire = &Wire;
  uint8_t _addr = 0;
  Chip _chip = CHIP_NONE;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  float _accelScale = 8192.0f;              // LSB на 1 g (диапазон 4 g)
  float _gyroScale = 65.5f;                 // LSB на °/с (диапазон 500)
  float _alpha = 0.98f;

  int16_t _raw[7] = {0};                    // ax ay az temp gx gy gz
  float _ax = 0, _ay = 0, _az = 0, _gx = 0, _gy = 0, _gz = 0, _tempC = 0;
  float _offAx = 0, _offAy = 0, _offAz = 0, _offGx = 0, _offGy = 0, _offGz = 0;
  float _roll = 0, _pitch = 0, _yaw = 0;
  bool _anglesInit = false;
  uint32_t _lastUs = 0;
  uint32_t _lastReadMs = 0;
  bool _hasData = false;

  bool probe(uint8_t address);
  bool readSample();
  bool read8(uint8_t reg, uint8_t &v);
  bool readBytes(uint8_t reg, uint8_t *buf, uint8_t len);
  bool write8(uint8_t reg, uint8_t v);
  void ensureFresh();
  void convert();
};

#endif  // ALASH_MPU_H
