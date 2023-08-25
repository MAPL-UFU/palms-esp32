#include <Pn532NfcReader.h>
#include <PN532_HSU.h>

//Rotines related with the configuration of the RFID reader PN532       
PN532_HSU pn532hsu(Serial1);
NfcAdapter nfc = NfcAdapter(pn532hsu);

//Creation of the reader and PNRD objects
Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
Pnrd pnrd = Pnrd(reader,5,5, false, true); //The pnrd object created has a tag history (5th argument)

uint32_t tagId = 0xFF;

void setup() {    
  //Initialization of the communication with the reader and with the computer Serial bus
  Serial.begin(9600);   
  reader->initialize();    

  //Setting of the classic iPNRD approach 
  pnrd.setAsTagInformation(PetriNetInformation::FIRE_VECTOR);
  pnrd.setAsTagInformation(PetriNetInformation::TAG_HISTORY);
  
  Serial.println("\nTag history viewer.");
}

void loop() {   
  delay(1000);
  
  //Tag reading
  ReadError readError = pnrd.getData();

  //In case of a successiful reading  
  if(readError == ReadError::NO_ERROR){
      
    if(tagId != pnrd.getTagId()){
        tagId = pnrd.getTagId();
        Serial.print("\nNew tag. ID code: ");        
        Serial.println(tagId, HEX);
        pnrd.printTagHistory();
           
    }
  }
  
}
