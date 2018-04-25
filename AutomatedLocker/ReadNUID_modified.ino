/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

int led = 3;
bool correct_key = false;

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  pinMode(LED_BUILTIN, OUTPUT);
}
 
void loop() {
  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;


  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

//  // Check is the PICC of Classic MIFARE type
//  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
//    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
//    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
//    Serial.println(F("Your tag is not of type MIFARE Classic."));
//    return;
//  }
//
//  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
//    rfid.uid.uidByte[1] != nuidPICC[1] || 
//    rfid.uid.uidByte[2] != nuidPICC[2] || 
//    rfid.uid.uidByte[3] != nuidPICC[3] ) {
//    Serial.println(F("A new card has been detected."));
//
//    // Store NUID into nuidPICC array
//    for (byte i = 0; i < 4; i++) {
//      nuidPICC[i] = rfid.uid.uidByte[i];
//    }
//   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
//    Serial.println();
//    Serial.print(F("In dec: "));
//    printDec(rfid.uid.uidByte, rfid.uid.size);
//    Serial.println();

    

    int byte0 = 0x5A;
    int byte1 = 0xA4;
    int byte2 = 0xDB;
    int byte3 = 0xD9;


    int byteA = 0x80;
    int byteB = 0x48;
    int byteC = 0xFD;
    int byteD = 0xA3;

    

    if (rfid.uid.uidByte[0] == byte0 && 
    rfid.uid.uidByte[1] == byte1 && 
    rfid.uid.uidByte[2] == byte2 && 
    rfid.uid.uidByte[3] == byte3 ) {
      
      Serial.println(F("The keys match!"));
      correct_key = true;

    }
      

      
//      while(1){
////         Serial.println(F("We're in the while"));
////        // put your main code here, to run repeatedly:
//////        digitalWrite(LED_BUILTIN, HIGH);
//////        delay(1000);
//////        digitalWrite(LED_BUILTIN,LOW);
//////        delay(1000);
//       
////          
////
//      }

    if (rfid.uid.uidByte[0] == byteA && 
    rfid.uid.uidByte[1] == byteB && 
    rfid.uid.uidByte[2] == byteC && 
    rfid.uid.uidByte[3] == byteD ) {
        Serial.println(F("The card matches!"));
       correct_key = false;
    }

    if(correct_key){
      digitalWrite(led, HIGH);
      delay(10000);
      digitalWrite(led,LOW);
      
    }
    else{
      digitalWrite(led,LOW);
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

void lightLED(void){
   digitalWrite(led, HIGH);
}
/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
