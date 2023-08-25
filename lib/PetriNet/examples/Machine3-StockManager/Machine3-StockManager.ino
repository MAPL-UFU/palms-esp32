#include <Pn532NfcReader.h>
#include <PN532_HSU.h>

//Rotines related with the configuration of the RFID reader PN532      
PN532_HSU pn532hsu(Serial1);
NfcAdapter nfc = NfcAdapter(pn532hsu);

//Creation of the reader and PNRD objects
Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
Pnrd pnrd = Pnrd(reader,5,4);

uint32_t tagId = 0xFF;

void setup() {  
  //Initialization of the communication with the reader and with the computer Serial bus
  Serial.begin(9600);   
  reader->initialize();     
  
  //Setting of the classic PNRD approach 
  pnrd.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);

   Serial.println("\nMachine 3: Stock management.");
}

void loop() {
  delay(1000);
  
  //Tag reading
  ReadError readError = pnrd.getData();

  //In case of a successful reading  
  if(readError == ReadError::NO_ERROR){
    
    FireError fireError;
    
    //Checks if it's a new tag
    if(tagId != pnrd.getTagId()){
        tagId = pnrd.getTagId();
        Serial.print("\nPart with the ID: ");
        Serial.println(tagId, HEX);        
       
        //Fire of transition 3
        FireError fireError = pnrd.fire(3);
     
    switch(fireError){
        case FireError::NO_ERROR :   
        
          //Update the information in tag         
          if(pnrd.saveData() == WriteError::NO_ERROR){
               Serial.println("Part ready to be stored.");
               return;
          }else{
               Serial.println("Error in the tag update.");
               return;
          };
          return;

        case FireError::PRODUCE_EXCEPTION :
          Serial.println("Error: exception. Part has followed a wrong workflow.");
          pnrd.printTokenVector();
          break;

        case FireError::CONDITIONS_ARE_NOT_APPLIED :   
              
          break;
    }      
   }
  }       
    
  
     
}
