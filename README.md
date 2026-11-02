# AlashMPU

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **гироскопа и акселерометра** MPU6050, MPU6500 и MPU9250 (плата **GY-521**): ускорение, вращение, **углы наклона** с фильтром, калибровка. Только встроенная `Wire`, без Adafruit Unified Sensor и BusIO. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <AlashMPU.h>
AlashMPU mpu;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) { Serial.println(mpu.errorText()); while (true); }
  mpu.calibrate();                  // датчик лежит ровно и не двигается
}
void loop() {
  Serial.println(mpu.roll());       // крен, градусы
  Serial.println(mpu.pitch());      // тангаж, градусы
  delay(100);
}
```

### Что умеет
- **Сам находит датчик:** адрес 0x68 или 0x69 и модель (MPU6050, MPU6500, MPU9250, в том числе платы-клоны).
- **Значения с понятными единицами:** `accelX/Y/Z()` в g, `gyroX/Y/Z()` в градусах в секунду, `temperature()` в °C.
- **Углы:** `roll()`, `pitch()`, `yaw()`, `tilt()` (отклонение от вертикали). Комплементарный фильтр объединяет гироскоп и акселерометр, работает на реальном времени между опросами. Вызывать `update()` не нужно.
- **Калибровка:** `calibrate()` убирает смещение нуля, проверяет, что датчик не двигали, и сообщает, если двигали.
- **Диапазоны:** `setAccelRange(2|4|8|16)` g, `setGyroRange(250|500|1000|2000)`.
- **ESP32 / ESP8266:** `mpu.begin(SDA, SCL)` для своих пинов.
- **Понятные подсказки** на русском, казахском и английском: `errorText()` и `diagnose(Serial)` (список устройств на шине I2C, значения и что делать; например, подсказывает, что часы DS3231 тоже имеют адрес 0x68 и как поменять адрес датчика выводом AD0).
- Работает на **Arduino AVR, ESP32, ESP8266** и других платах.

### Подключение (GY-521)
| Датчик | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 3.3 В или 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| AD0 | не подключать (0x68) или 3.3 В (0x69) | то же |

### Чем отличается от других библиотек
Мы посмотрели MPU6050 (Electronic Cats / I2Cdevlib), MPU6050_light (rfetick), Adafruit MPU6050 и MPU9250 (bolderflight):
- Adafruit требует установить ещё Adafruit Unified Sensor и BusIO; I2Cdevlib большая и построена вокруг DMP с прошивкой; у нас одна короткая библиотека.
- Углы почти везде надо считать самому; у нас `roll()`, `pitch()`, `tilt()`.
- Ни в одной нет подсказки про конфликт адреса 0x68 с часами и что делать, если датчик не нашёлся: у нас `diagnose()`.

Не поддерживается: магнитометр MPU9250 (компас), прерывания, DMP. Рыскание `yaw()` считается по гироскопу, поэтому со временем уплывает: это свойство гироскопа без компаса.

### Примеры
`Basic`, `Tilt`, `Calibrate`, `Diagnose`, `ESP32_CustomPins`.

### Тесты
Единицы, диапазоны, формулы углов, фильтр, интегрирование, калибровка и отклонение при движении, определение чипа проверены без железа на имитации регистров: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**Гироскоп пен акселерометрге** MPU6050, MPU6500 және MPU9250 (**GY-521** тақтасы) арналған қарапайым кітапхана: үдеу, айналу, сүзгісі бар **еңкею бұрыштары**, калибрлеу. Тек кірістірілген `Wire`. Alash Electronics сабақтары мен жинақтарына арналған.

```cpp
#include <AlashMPU.h>
AlashMPU mpu;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) { Serial.println(mpu.errorText()); while (true); }
  mpu.calibrate();                  // сенсор тегіс жатыр, қозғалмайды
}
void loop() {
  Serial.println(mpu.roll());
  Serial.println(mpu.pitch());
  delay(100);
}
```

### Мүмкіндіктері
- **Сенсорды өзі табады:** 0x68 немесе 0x69 мекенжайы және моделі.
- **Мәндер:** `accelX/Y/Z()` g-мен, `gyroX/Y/Z()` градус/секундпен, `temperature()` °C.
- **Бұрыштар:** `roll()`, `pitch()`, `yaw()`, `tilt()`. `update()` шақырудың қажеті жоқ.
- **Калибрлеу:** `calibrate()` нөлдік ығысуды жояды, сенсор қозғалса хабарлайды.
- **Ауқымдар:** `setAccelRange()`, `setGyroRange()`.
- **ESP32 / ESP8266:** `mpu.begin(SDA, SCL)`.
- **Түсінікті кеңестер** орысша, қазақша, ағылшынша: `errorText()`, `diagnose(Serial)`.

### Қосу (GY-521)
| Сенсор | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 3.3 В немесе 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |

### Мысалдар
`Basic`, `Tilt`, `Calibrate`, `Diagnose`, `ESP32_CustomPins`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for the **gyroscope and accelerometer** MPU6050, MPU6500 and MPU9250 (**GY-521** board): acceleration, rotation, filtered **tilt angles**, calibration. Only the built-in `Wire`, no Adafruit Unified Sensor or BusIO. Made for classes and Alash Electronics kits.

```cpp
#include <AlashMPU.h>
AlashMPU mpu;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) { Serial.println(mpu.errorText()); while (true); }
  mpu.calibrate();                  // the sensor lies flat and still
}
void loop() {
  Serial.println(mpu.roll());       // degrees
  Serial.println(mpu.pitch());      // degrees
  delay(100);
}
```

### Features
- **Finds the sensor itself:** address 0x68 or 0x69 and the model (MPU6050, MPU6500, MPU9250, clone boards included).
- **Values in clear units:** `accelX/Y/Z()` in g, `gyroX/Y/Z()` in degrees per second, `temperature()` in °C.
- **Angles:** `roll()`, `pitch()`, `yaw()`, `tilt()`. A complementary filter merges gyro and accelerometer using the real time between reads. No `update()` needed.
- **Calibration:** `calibrate()` removes the zero offsets and tells you if the sensor was moved.
- **Ranges:** `setAccelRange(2|4|8|16)` g, `setGyroRange(250|500|1000|2000)`.
- **ESP32 / ESP8266:** `mpu.begin(SDA, SCL)` for custom pins.
- **Clear tips** in Russian, Kazakh and English: `errorText()` and `diagnose(Serial)`.
- Works on **Arduino AVR, ESP32, ESP8266** and others.

### Wiring (GY-521)
| Sensor | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 3.3 V or 5 V | 3.3 V |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |

Not supported: the MPU9250 magnetometer (compass), interrupts, DMP. `yaw()` is integrated from the gyro, so it drifts over time.

### Examples
`Basic`, `Tilt`, `Calibrate`, `Diagnose`, `ESP32_CustomPins`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

### License
MIT, Alash Electronics.
