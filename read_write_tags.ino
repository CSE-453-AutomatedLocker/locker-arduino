#include <EEPROM.h>

void setup() {
  /**
   * TODO(anand-bala): Write state machine in pseudocode
   */
  deleteKeys();
  Serial.begin(9600);
  uint32_t test = 0xDEADBEEF;
  byte buf[4];
  memcpy(buf, &test, 4);
  addKey(buf);
  test = 0xAAAAAAAA;
  buf[4];
  memcpy(buf, &test, 4);
  addKey(buf);
  test = 0xEEEAAFF1;
  buf[4];
  memcpy(buf, &test, 4);
  addKey(buf);
  test = 0x12345678;
  buf[4];
  memcpy(buf, &test, 4);
  addKey(buf);

}

void loop() {
//   put your main code here, to run repeatedly:
  uint32_t test = 0x12345678;
  byte buf[4];
  memcpy(buf, &test, 4);
  containsKey(buf);
  delay(10000000);
  return;
}

void addKey(byte tag[]) {
  uint8_t offset = EEPROM.read(0);
  int addr = (offset * 4) + 1;
  for (int i = 0; i < 4; i++) {
    EEPROM.write(addr + i, tag[i]);
  }
  uint32_t id;
  memcpy(&id, tag, 4);
  Serial.print("Added tag ");
  Serial.println(id, HEX);
  offset += 1;
  EEPROM.write(0, offset);
}

void deleteKeys() {
  EEPROM.write(0, 0);
}

bool containsKey(byte tag[]) {
  bool res = false;
  uint32_t id;
  memcpy(&id, tag, 4);
  Serial.print("Looking for: ");
  Serial.println(id, HEX);
  uint8_t offset = EEPROM.read(0);
  int end = (offset * 4) + 1;
  byte cur[4];
  for (int i = 1; i < end; i += 4) {
    for (int j = 0; j < 4; j++) {
      cur[j] = EEPROM.read(j + i);
    }
    uint32_t id2;
    memcpy(&id2, cur, 4);
    Serial.print("Found: ");
    Serial.println(id2, HEX);
    if (id == id2) {
      res = true;
    }
  }
  return res;
}


