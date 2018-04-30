/*
   --------------------------------------------------------------------------------------------------------------------
   Example sketch/program showing how to read new NUID from a PICC to serial.
   --------------------------------------------------------------------------------------------------------------------
   This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid

   Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
   Reader on the Arduino SPI interface.

   When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
   then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
   you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
   will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
   when removing the PICC from reading distance too early.

   @license Released into the public domain.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include "Button.h"

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

int led = 3;
bool correct_key = false;
int state = 0;
Button btn(2);

/* Bunch of function declarations */

/**
   Return the next state
*/
int handleIntermediateState() {
  long t = btn.getSec();
  long m = btn.getMilli();
  if ( t == -1 ) {
    return 3; // If button has been released: state -> shortPress (3)
  }
  if ( (t < 5) ) {
    return 1;  // if still being presses, remain in same state
  }
  if ( t >= 5 ) {
    return 2; // If t > 10, state -> longPress (2)
  }
}

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.println(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println("");

  pinMode(LED_BUILTIN, OUTPUT);
  deleteKeys();
  byte key1[] = {0x5A, 0xA4, 0xDB, 0xD9};
  addKey(key1);
  byte key2[] = {0x80, 0x48, 0xFD, 0xA3};
  addKey(key2);
}

void handleEEPROM() {
  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been read
  if ( ! rfid.PICC_ReadCardSerial())
    return;


  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  //  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  //    Serial.println(F("The NUID tag is:"));
  //    Serial.print(F("In hex: "));
  //    printHex(rfid.uid.uidByte, rfid.uid.size);
  //    Serial.println();
  //    Serial.print(F("In dec: "));
  //    printDec(rfid.uid.uidByte, rfid.uid.size);
  //    Serial.println();

  Serial.print("Res: ");
  bool res = containsKey(rfid.uid.uidByte);
  Serial.println(res);

  if (res) {
    digitalWrite(led, HIGH);
    delay(10000);
    digitalWrite(led, LOW);

  }
  else {
    digitalWrite(led, LOW);
  }  
}

void handleAddKeyState() {
  if (btn.read() == HIGH) {
    state = 4;
  }
  else {
    // Look for new cards
    if ( ! rfid.PICC_IsNewCardPresent())
      return;

    // Verify if the NUID has been read
    if ( ! rfid.PICC_ReadCardSerial())
      return;

    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.println(rfid.PICC_GetTypeName(piccType));

    //  // Check is the PICC of Classic MIFARE type
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
      return;
    }
        
    addKey(rfid.uid.uidByte);
  }
}

void loop() {
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
      Serial.print(F("In state 0: Normal state"));
      Serial.print("\n");
      if (btn.read() == HIGH) {
        state = 1;
      }
      else {
        // handle EEPROM
        handleEEPROM();
      }
      break;
    case 1:
      Serial.print(F("In state 1: holding button down"));
      Serial.print("\n");
      state = handleIntermediateState();
      break;
    case 2:
      // Handle Long Press -> delete all
      Serial.print(F("In state 2: deleting all keys"));
      Serial.print("\n");
      deleteKeys();
      state = 0;
      break;
    case 3:
      // Handle Short press -> add keys state
      Serial.print(F("In state 3: adding keys"));
      Serial.print("\n");
      handleAddKeyState();
      break;
    case 4: // Extra state after done adding keys, need to press button again to go back to normal state
      if (btn.read() == LOW) {
        state = 0;
      }
      break;
  }
}


//  else Serial.println(F("Card read previously."));
//
//  // Halt PICC
//  rfid.PICC_HaltA();
//
//  // Stop encryption on PCD
//  rfid.PCD_StopCrypto1();
//}

void lightLED(void) {
  digitalWrite(led, HIGH);
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


void addKey(byte tag[]) {
  if (containsKey(tag)) {
    Serial.println("Key already added!");
    return;
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
