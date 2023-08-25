#include <Pn532NfcReader.h>
#include <PN532_HSU.h>
      
uint16_t mFireVector[] = {1,0,0,0,0};

//Rotines related with the configuration of the RFID reader PN532      
PN532_HSU pn532hsu(Serial1);
NfcAdapter nfc = NfcAdapter(pn532hsu);

//Creation of the reader and PNRD objects
Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
Pnrd pnrd = Pnrd(reader,5,5, false, true);

void setup() {  
  //Initialization of the communication with the reader and with the computer Serial bus
  Serial.begin(9600); 
  reader->initialize();      

  //Defining the fire vector to be recorded
  pnrd.setFireVector(mFireVector);

  //Setting of the classic iPNRD approach 
  pnrd.setAsTagInformation(PetriNetInformation::FIRE_VECTOR);
  pnrd.setAsTagInformation(PetriNetInformation::TAG_HISTORY);
  Serial.print("\nInitial recording of iPNRD tags.");
}

void loop() {
  
  Serial.println("Place a tag near the reader.");
  
  if(pnrd.saveData() == WriteError::NO_ERROR){
        Serial.println("Tag configurated successfully.");
  };

  Serial.print('\n');
  delay(1000); 
   
}
