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

int led = 8;
int redled = 4;
int blueled = 6;
int greenled = 7;
bool correct_key = false;
int state = 0;
Button btn(2);
bool locked = true;
long unlocked_time = 0;

/* Bunch of function declarations */
void printHex(byte *buffer, byte bufferSize);
void deleteKeys();
void addKey(byte tag[]);
bool containsKey(byte tag[]);
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

  pinMode(redled, OUTPUT);
  pinMode(blueled, OUTPUT);
  pinMode(greenled, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

bool handleEEPROM() {
  // Look for new cards
  //Serial.println("CHK New Card");
  if ( ! rfid.PICC_IsNewCardPresent()) {
    //    Serial.println("ERR Old Card");
    return false;
  }

  // Verify if the NUID has been read
  if ( ! rfid.PICC_ReadCardSerial()) {
    //    Serial.println("ERR NUID not read");
    return false;
  }


  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  //  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return false;
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
    unlocked_time = millis();
  }
  return res;
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
  if (!locked && (millis() - unlocked_time > 5000)) {
    Serial.println("LOCK!");
    locked = true;
    digitalWrite(led, LOW);
    digitalWrite(LED_BUILTIN, LOW);
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
      digitalWrite(redled, 0); // actually green
      digitalWrite(blueled, 0); // red
      digitalWrite(greenled, 0); // blue
      //      Serial.print(F("In state 0: Normal state"));
      //      Serial.print("\n");
      if (btn.read() == HIGH) {
        state = 1;
      }
      else {
        // handle EEPROM
        //        Serial.println(F("handle EEPROM"));
        if (handleEEPROM()) {
          Serial.println("UNLOCK!!");
          locked = false;
          unlocked_time = millis();
          digitalWrite(LED_BUILTIN, HIGH);
          digitalWrite(led, HIGH);
        }
      }
      break;
    case 1:
      //      Serial.print(F("In state 1: holding button down"));
      //      Serial.print("\n");
      state = handleIntermediateState();
      digitalWrite(redled, 1); // actually green
      digitalWrite(blueled, 1); // red
      digitalWrite(greenled, 0); // blue
      break;
    case 2:
      // Handle Long Press -> delete all
      //      Serial.print(F("In state 2: deleting all keys"));
      //      Serial.print("\n");
      deleteKeys();
      state = 0;
      digitalWrite(redled, 1); // actually green
      digitalWrite(blueled, 0); // red
      digitalWrite(greenled, 1); // blue
      delay(1000);  // NOTE: A delay is needed to prevent the state from going to 3 immediately
      break;
    case 3:
      // Handle Short press -> add keys state
      //      Serial.print(F("In state 3: adding keys"));
      //      Serial.print("\n");
      handleAddKeyState();
      digitalWrite(redled, 0); // actually green
      digitalWrite(blueled, 1); // red
      digitalWrite(greenled, 1); // blue
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
    digitalWrite(redled, 0); // actually green
    digitalWrite(blueled, 0); // red
    delay(250);
    return;
  }

  digitalWrite(redled, 1); // actually green
  digitalWrite(blueled, 0); // red

  delay(250);
  
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
