#include <EEPROM.h>
#include "Arduino.h"
#include "keys.h"


bool addKey(byte tag[]) {
  if (containsKey(tag)) {
    Serial.println("Key already added!");
    return false;
  }

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
  return true;
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
