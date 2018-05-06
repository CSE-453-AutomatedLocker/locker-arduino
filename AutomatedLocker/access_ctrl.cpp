
#include <SPI.h>

#include "access_ctrl.h"
#include "keys.h"

static bool init_p = false;
static MFRC522 *rfid = NULL;

static int solenoid = -1;
static bool locked = true;
static long unlocked_time = -1;

void init_access_ctrl(int ss_pin, int rst_pin, int solenoid_pin)
{
  if (init_p) return;
  solenoid = solenoid_pin;
  locked = true;
  unlocked_time = -1;
  rfid = new MFRC522(ss_pin, rst_pin);
  rfid->PCD_Init();

  init_p = true;
}

static bool detectKey() {
  // Check if there is a new card being shown
  if(!rfid->PICC_IsNewCardPresent()) {
    return false;
  }

  // Read the card
  bool ok = rfid->PICC_ReadCardSerial();
  if(!ok) return false;

  // Get PICC type (should be MIFARE)
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid->PICC_GetType(rfid->uid.sak);
  Serial.println(rfid->PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return false;
  }
  return true;
}

bool checkNewTag() {
  if (!init_p) {
    Serial.println("Uninitialized Access Control");
    return false;
  }

  if (detectKey()) {
    // Return whether read key is present in EEPROM
    return containsKey(rfid->uid.uidByte);
  }
  return false;
}

int addNewKey() {
  if (!init_p) {
    Serial.println("Uninitialized Access Control");
    return -1;
  }

  if (detectKey()) {
    return (addKey(rfid->uid.uidByte)) ? 0:1;
  }
  return -1;
}

void unlock() {
  if (!locked) return;
  locked = false;
  unlocked_time = millis();
  digitalWrite(solenoid, HIGH);
}

void lock() {
  if (locked) return;
  locked = true;
  unlocked_time = -1;
  digitalWrite(solenoid, LOW);
}

long unlocked_ms() {
  if (locked) return -1;
  return millis() - unlocked_time;
}
