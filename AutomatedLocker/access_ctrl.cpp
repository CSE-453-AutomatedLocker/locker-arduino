/**
 * @file access_ctrl.cpp
 * @brief Control access to the locker (lock, unlock, detect, add)
 *
 * This contains the functions that will control the locker, including
 *  - Lock/Unlock
 *  - Add/Detect Keys
 *  - Get the state of the locker (Amount of time the locker has been unlocked)
 *
 * @author Simran Singh (simranlall)
 * @author Ian Wilson (singularity471)
 * @author Jacob Belewich (jacobbel)
 * @author Anand Balakrishnan (anand-bala)

 * @date 2018-05-06
 */


#include <SPI.h>
#include "Arduino.h"
#include "access_ctrl.h"
#include "keys.h"

/* -- State Variables -- */
/**
 *  Keeps track of whether the `Access Control` system was initialized.
 */
static bool init_p = false;
/**
 * Pointer RFID module driver.
 */
static MFRC522 *rfid = NULL;

/**
 * The pin to control the solenoid.
 */
static int solenoid = -1;

/**
 * State of the locker (Locked/Unlocked).
 */
static bool locked = true;
/**
 * Amount of time the locker has been unlocked.
 */
static long unlocked_time = -1;

/**
 * Initialize the`Access Control` module.
 *
 * @param ss_pin        SS Pin for MFRC522 (SPI protocol)
 * @param rst_pin       RST Pin for MFRC522 
 * @param solenoid_pin  Pin to solenoid
 */
void init_access_ctrl(int ss_pin, int rst_pin, int solenoid_pin)
{
  if (init_p) return;
  solenoid = solenoid_pin;
  pinMode(solenoid, OUTPUT);
  locked = true;
  unlocked_time = -1;
  rfid = new MFRC522(ss_pin, rst_pin);
  rfid->PCD_Init();

  init_p = true;
}

/**
 * Internal function to detect if a new MIFARE tag is present and can be read.
 *
 * @return if a valid tag has been detected
 */
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

/**
 * Check if a new tag has been detected and is present in EEPROM
 *
 * @return Nothing happened => -1; Key accepted => 1; Key rejected => 0.
 */
int checkNewTag() {
  if (!init_p) {
    Serial.println("Uninitialized Access Control");
    return -1;
  }

  if (detectKey()) {
    // Return whether read key is present in EEPROM
    return (containsKey(rfid->uid.uidByte)) ? 1:0;
  }
  return -1;
}

/**
 * Detec and add key to EEPROM
 *
 * @return Nothing happened => -1; Key added = 0; Key already exists = 1;
 */
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

/**
 * Unlock the locker
 */
void unlock() {
  if (!locked) return;
  locked = false;
  unlocked_time = millis();
  digitalWrite(solenoid, HIGH);
}

/**
 * Lock the locker
 */
void lock() {
  if (locked) return;
  locked = true;
  unlocked_time = -1;
  digitalWrite(solenoid, LOW);
}

/**
 * Get number of milliseconds the locker has been unlocked for
 *
 * @return ms the locker has been unlocked (-1 if locked)
 */
long unlocked_ms() {
  if (locked) return -1;
  return millis() - unlocked_time;
}
