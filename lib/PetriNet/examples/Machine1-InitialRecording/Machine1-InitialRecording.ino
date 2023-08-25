#include <Pn532NfcReader.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
String  anttag;

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);
int8_t mIncidenceMatrix[] = {-1,  0,  1,  0,
                              1, -1, -1,  0,
                              0,  1,  0, -1,
                              0,  0,  1,  0,
                              0,  0,  0,  1};
      
uint16_t mStartingTokenVector[] = {1,0,0,0,0};


//Creation of the reader and PNRD objects
Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
Pnrd pnrd = Pnrd(reader,5,4);

void setup() {  
  //Initialization of the communication with the reader and with the computer Serial bus 
  Serial.begin(9600); 
  reader->initialize();     

  pnrd.setIncidenceMatrix(mIncidenceMatrix);
  pnrd.setTokenVector(mStartingTokenVector);

  //Setting of the classic PNRD approach  
  pnrd.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);
  Serial.print("\nMachine 1: Initial tag recording.");
}

void loop() {
  
  Serial.println("Place an tag to be configurated.");
  
  if(pnrd.saveData() == WriteError::NO_ERROR){
        Serial.println("Tag configurated successfully.");
  };

  Serial.print('\n');
  delay(1000); 
   
}
