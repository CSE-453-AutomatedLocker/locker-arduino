#ifndef __ACCESSCTRL_H__
#define __ACCESSCTRL_H__

#include <MFRC522.h>

/**
 * Initialize the access control module
 *
 * @param ss_pin        SS Pin for RFID
 * @param rst_pin       RST Pin for RFID
 * @param solenoid_pin  Pin that controls the solenoid
 */
void init_access_ctrl(int ss_pin, int rst_pin, int solenoid_pin);

/**
 * Will check if:
 * 1. There is a new tag present; and
 * 2. The tag is valid (present in EEPROM)
 *
 * @returns Tag validity
 */
bool checkNewTag();

/**
 * Will do the following:
 * 1. Check if there is a new key
 * 2. Add the new key to EEPROM
 *
 * @returns
 *  -> -1 if no new tag was detected
 *  -> 0 if tag was successfully added
 *  -> 1 if tag was already present
 */
int addNewKey();

/* Control the solenoid states */
void unlock();
void lock();

/**
 * @returns Number of milliseconds the locker is unlocked or -1 if locked
 */
long unlocked_ms();

#endif // __ACCESSCTRL_H__
