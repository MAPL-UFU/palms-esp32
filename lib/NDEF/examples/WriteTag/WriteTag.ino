#if 0
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);
#else

#include <Wire.h>
#include <PN532_HSU.h>
#include <PN532.h>
#include <NfcAdapter.h>


PN532_HSU pn532hsu1(Serial2);

NfcAdapter nfc = NfcAdapter(pn532hsu1);
#endif

void setup() {
      Serial.begin(9600);
      Serial.println("NDEF Writer");
      nfc.begin();
}

void loop() {
    Serial.println("\nPlace a formatted Mifare Classic NFC tag on the reader.");
    if (nfc.tagPresent()) {
        NdefMessage message = NdefMessage();
        message.addUriRecord("http://arduino.cc");

        bool success = nfc.write(message);
        if (success) {
          Serial.println("Success. Try reading this tag with your phone.");        
        } else {
          Serial.println("Write failed.");
        }
    }
    delay(5000);
}