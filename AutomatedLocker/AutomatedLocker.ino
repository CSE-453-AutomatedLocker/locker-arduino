#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#include "RGB_LED.h"
#include "Button.h"
#include "keys.h"
#include "access_ctrl.h"

// RFID Pins
#define SS_PIN  10
#define RST_PIN 9

// Solenoid Pin
#define SOLENOID  8

// RGB LED Pins
#define RGB_RED   6
#define RGB_GREEN 4
#define RGB_BLUE  7

// Initialize a bunch of stuff
Button btn(2);
RGB_LED rgb(RGB_RED, RGB_GREEN, RGB_BLUE);

int state = 0;
long system_timeout = 0;

/* Bunch of function declarations */
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);


int state_normal() {
  rgb.writeState(0,0,0); 
  if (btn.read() == HIGH) {
    return 1; // return next state
  }

  if (checkNewTag()) {
    Serial.println("UNLOCK!!");
    system_timeout = millis(); // resets the timer
    unlock();
  }
  return state;
}

/**
  Return the next state
 */
int state_intermediate() {
  long t = btn.getSec();
  long m = btn.getMilli();
  if ( t == -1 ) {
    system_timeout = millis(); // reset the timer
    return 3; // If button has been released: state -> shortPress (3)
  }
  if ( (t < 5) ) {
    return 1;  // if still being presses, remain in same state
  }
  if ( t >= 5 ) {
    return 2; // If t > 10, state -> longPress (2)
  }
}

int state_add() {
  if (btn.read() == HIGH) {
    return 4; // Next state
  }
  switch (addNewKey()) {
    case -1:
      break;
    case 0:
      rgb.writeState(1,0,1);
      delay(250); // Delay to make flash visible
      break;
    case 1:
      rgb.writeState(0,1,1);
      delay(250);
      break;
    }
    return state; // Else return same state
}

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
 
  init_access_ctrl(SS_PIN, RST_PIN, SOLENOID);
  system_timeout = millis(); // Start up the system timeout
}



void loop() {
  /**
   * If the locker is locked and system has has been unlocked for longer than 10s, lock
   */
  if (unlocked_ms() > 10000) {
    Serial.println("LOCK!");
    system_timeout = millis(); // resets the timer
    lock();
  }
  /**
   * If not in add state or button press and system has timed out (10s)
   * --> shutdown
   */
  if ((state == 0 || state == 3) && millis() - system_timeout > 10000) {
    digitalWrite(3, HIGH);  // Turn off system
    Serial.println(F("System timed out!"));
  }

  /**
    State Machine:
    0 -> Normal Mode (Read Tag and compare with EEPROM)
    1 -> 0 <= t < TIMEOUT (Intermediate State)
    2 -> Long Press (If t >= TIMEOUT)
    3 -> Short Press (If released and previous state was 1)
    4 -> State after the short press which waits for a button press to return to the Normal Mode
   */
  switch (state) {
    case 0:
      state = state_normal();
      break;
    case 1:
      state = state_intermediate();
      rgb.writeState(0,0,1);
      break;
    case 2:
      // Handle Long Press -> delete all
      deleteKeys();
      state = 0;
      system_timeout = millis(); // reset the timer
      rgb.writeState(0,1,1);
      // NOTE: A delay is needed to prevent the state from going to 3 immediately
      delay(1000);
      break;
    case 3:
      // Handle Short press -> add keys state
      state = state_add();
      rgb.writeState(0, 1, 0);
      break;
    case 4: 
      // Extra state after done adding keys
      // - press button again to go back to normal state
      if (btn.read() == LOW) {
        state = 0;
        system_timeout = millis(); // reset the timer
      }
      break;
  }
}

/**
  Helper routine to dump a byte array as hex values to Serial.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
  Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
