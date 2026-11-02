# AlashMPU: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **гироскопа и акселерометра** MPU6050, MPU6500 и MPU9250 (плата **GY-521**): ускорение, вращение, **углы наклона** с фильтром, калибровка. Только встроенная `Wire`, без Adafruit Unified Sensor и BusIO. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashMPU** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение (GY-521)
| Датчик | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 3.3 В или 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| AD0 | не подключать (0x68) или 3.3 В (0x69) | то же |

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin() / begin(sda, scl)` | Найти датчик (0x68 или 0x69) и определить модель |
| `calibrate()` | Калибровка: датчик лежит неподвижно |
| `accelX() / accelY() / accelZ()` | Ускорение, g |
| `gyroX() / gyroY() / gyroZ()` | Скорость вращения, градусов в секунду |
| `roll() / pitch() / yaw()` | Крен, тангаж, рыскание (фильтр гироскоп + акселерометр) |
| `tilt()` | Отклонение от вертикали |
| `temperature()` | Температура кристалла, °C |
| `isStill()` | Датчик почти не вращается |
| `setAccelRange(g) / setGyroRange(dps)` | Диапазоны измерения |
| `setFilter(alpha)` | Доля гироскопа в фильтре |
| `resetYaw()` | Обнулить рыскание |
| `chipName() / isConnected() / diagnose(Serial)` | Модель, связь, самопроверка |

### Если что-то не работает
**Датчик не найден**

Проверьте SDA, SCL, VCC, GND; пин AD0 на GND даёт адрес 0x68, на VCC 0x69. У Uno SDA это A4, SCL это A5. Запустите Diagnose.

**Углы уплывают, особенно yaw**

Рыскание считается по гироскопу и со временем уходит (у MPU6050 нет магнитометра). Для курса берите AlashCompass. Откалибруйте датчик: `calibrate()` на ровной поверхности.

**Ошибка калибровки (датчик двигали)**

При `calibrate()` датчик должен лежать неподвижно около секунды, осью Z вверх.

### Чем отличается от других библиотек
Мы посмотрели MPU6050 (Electronic Cats / I2Cdevlib), MPU6050_light (rfetick), Adafruit MPU6050 и MPU9250 (bolderflight):
- Adafruit требует установить ещё Adafruit Unified Sensor и BusIO; I2Cdevlib большая и построена вокруг DMP с прошивкой; у нас одна короткая библиотека.
- Углы почти везде надо считать самому; у нас `roll()`, `pitch()`, `tilt()`.
- Ни в одной нет подсказки про конфликт адреса 0x68 с часами и что делать, если датчик не нашёлся: у нас `diagnose()`.

Не поддерживается: магнитометр MPU9250 (компас), прерывания, DMP. Рыскание `yaw()` считается по гироскопу, поэтому со временем уплывает: это свойство гироскопа без компаса.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Tilt`, `Calibrate`, `Diagnose`, `ESP32_CustomPins`.

---

## Қазақша

**Гироскоп пен акселерометрге** MPU6050, MPU6500 және MPU9250 (**GY-521** тақтасы) арналған қарапайым кітапхана: үдеу, айналу, сүзгісі бар **еңкею бұрыштары**, калибрлеу. Тек кірістірілген `Wire`. Alash Electronics сабақтары мен жинақтарына арналған.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashMPU** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу (GY-521)
| Сенсор | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 3.3 В немесе 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin() / begin(sda, scl)` | Датчикті табу (0x68 не 0x69) және моделін анықтау |
| `calibrate()` | Калибрлеу: датчик қозғалмай жатады |
| `accelX() / accelY() / accelZ()` | Үдеу, g |
| `gyroX() / gyroY() / gyroZ()` | Айналу жылдамдығы, секундына градус |
| `roll() / pitch() / yaw()` | Бүйір, бойлық, бұрылу (гироскоп + акселерометр сүзгісі) |
| `tilt()` | Тігінен ауытқу |
| `temperature()` | Кристалл температурасы, °C |
| `isStill()` | Датчик дерлік айналмайды |
| `setAccelRange(g) / setGyroRange(dps)` | Өлшеу диапазондары |
| `setFilter(alpha)` | Сүзгідегі гироскоп үлесі |
| `resetYaw()` | Бұрылуды нөлдеу |
| `chipName() / isConnected() / diagnose(Serial)` | Моделі, байланыс, өзін тексеру |

### Бірдеңе жұмыс істемесе
**Датчик табылмады**

SDA, SCL, VCC, GND тексеріңіз; AD0 GND-ге қосылса 0x68, VCC-ге қосылса 0x69. Uno-да SDA A4, SCL A5. Diagnose іске қосыңыз.

**Бұрыштар жылжиды, әсіресе yaw**

Бұрылу гироскоппен есептеледі де, уақыт өте ауытқиды. Бағыт үшін AlashCompass алыңыз. `calibrate()` жасаңыз.

**Калибрлеу қатесі (датчик қозғалды)**

`calibrate()` кезінде датчик шамамен бір секунд қозғалмай, Z осі жоғары жатуы керек.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Tilt`, `Calibrate`, `Diagnose`, `ESP32_CustomPins`.

---

## English

A simple library for the **gyroscope and accelerometer** MPU6050, MPU6500 and MPU9250 (**GY-521** board): acceleration, rotation, filtered **tilt angles**, calibration. Only the built-in `Wire`, no Adafruit Unified Sensor or BusIO. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashMPU** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring (GY-521)
| Sensor | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 3.3 V or 5 V | 3.3 V |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |

Not supported: the MPU9250 magnetometer (compass), interrupts, DMP. `yaw()` is integrated from the gyro, so it drifts over time.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `begin() / begin(sda, scl)` | Find the sensor (0x68 or 0x69) and detect the model |
| `calibrate()` | Calibration: the sensor lies still |
| `accelX() / accelY() / accelZ()` | Acceleration, g |
| `gyroX() / gyroY() / gyroZ()` | Rotation speed, degrees per second |
| `roll() / pitch() / yaw()` | Roll, pitch, yaw (gyro + accelerometer filter) |
| `tilt()` | Tilt from vertical |
| `temperature()` | Chip temperature, °C |
| `isStill()` | The sensor hardly rotates |
| `setAccelRange(g) / setGyroRange(dps)` | Measurement ranges |
| `setFilter(alpha)` | Gyro share in the filter |
| `resetYaw()` | Reset yaw |
| `chipName() / isConnected() / diagnose(Serial)` | Model, link, self-check |

### If something does not work
**The sensor is not found**

Check SDA, SCL, VCC, GND; the AD0 pin to GND gives address 0x68, to VCC 0x69. On Uno SDA is A4 and SCL is A5. Run Diagnose.

**Angles drift, especially yaw**

Yaw is integrated from the gyro and drifts (the MPU6050 has no magnetometer). For heading use AlashCompass. Calibrate with `calibrate()` on a flat surface.

**Calibration error (the sensor moved)**

During `calibrate()` the sensor must lie still for about a second with the Z axis up.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Tilt`, `Calibrate`, `Diagnose`, `ESP32_CustomPins`.

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashMPU
