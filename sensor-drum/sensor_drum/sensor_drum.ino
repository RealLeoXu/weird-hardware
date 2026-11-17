// Sensor Drum - ESP32-C3 Super Mini + Adafruit MMA8451
// Sends "x y z" over USB serial at 800 Hz. 1024 = 1 g.
// SDA = GPIO3, SCL = GPIO4. Needs "USB CDC On Boot: Enabled".

#include <Wire.h>
#include <Adafruit_MMA8451.h>

#define PIN_SDA     3
#define PIN_SCL     4
#define MMA_ADDR    0x1D
#define REG_STATUS  0x00
#define REG_OUT_X   0x01
#define REG_CTRL1   0x2A

Adafruit_MMA8451 mma;

static bool readRegs(uint8_t reg, uint8_t *buf, int n) {
  Wire.beginTransmission(MMA_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)MMA_ADDR, n) != n) return false;
  for (int i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

static void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MMA_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000) {}

  Wire.setPins(PIN_SDA, PIN_SCL);
  if (!mma.begin(MMA_ADDR)) {
    while (1) {
      Serial.println("# MMA8451 not found");
      delay(1000);
    }
  }
  mma.setRange(MMA8451_RANGE_8_G);   // hits go way past 2 g
  mma.setDataRate(MMA8451_DATARATE_800_HZ);
  Wire.setClock(400000);

  // the library turns on low-noise mode, which caps the range at 4 g.
  // turn it off (has to be done in standby).
  uint8_t ctrl1;
  readRegs(REG_CTRL1, &ctrl1, 1);
  writeReg(REG_CTRL1, 0x00);
  writeReg(REG_CTRL1, (ctrl1 & ~0x04) | 0x01);

  // lines starting with # get filtered out in Max
  Serial.println("# ready");
}

void loop() {
  uint8_t status;
  if (!readRegs(REG_STATUS, &status, 1)) return;
  if (!(status & 0x08)) return;   // wait for a new xyz sample

  uint8_t b[6];
  if (!readRegs(REG_OUT_X, b, 6)) return;
  // 14-bit values, left-aligned
  int16_t x = (int16_t)((b[0] << 8) | b[1]) >> 2;
  int16_t y = (int16_t)((b[2] << 8) | b[3]) >> 2;
  int16_t z = (int16_t)((b[4] << 8) | b[5]) >> 2;

  char line[24];
  int len = snprintf(line, sizeof(line), "%d %d %d\n", x, y, z);
  Serial.write((const uint8_t *)line, len);
}
