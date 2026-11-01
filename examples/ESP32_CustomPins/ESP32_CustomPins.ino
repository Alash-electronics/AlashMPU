// AlashMPU на ESP32 со своими пинами I2C: begin(SDA, SCL).
#include <AlashMPU.h>

AlashMPU mpu;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin(21, 22)) {               // SDA = GPIO21, SCL = GPIO22 (можно любые другие)
    Serial.println(mpu.errorText());
    while (true) delay(1000);
  }
  mpu.calibrate();
}

void loop() {
  Serial.print(mpu.roll(), 1);
  Serial.print("  ");
  Serial.println(mpu.pitch(), 1);
  delay(100);
}
