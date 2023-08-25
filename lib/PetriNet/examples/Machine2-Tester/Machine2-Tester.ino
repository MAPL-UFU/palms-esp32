#include <Pn532NfcReader.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
String  anttag;

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);

//Creation of the reader and PNRD objects
Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
Pnrd pnrd = Pnrd(reader,5,4);

uint32_t tagId = 0xFF;

const int buttonAccept = 6;
const int buttonReject = 5;

bool tagReadyToContinue = false;

void setup() {  
  //Initialization of the communication with the reader and with the computer Serial bus 
  Serial.begin(9600);   
  reader->initialize(); 
  
  //Iniciação dos botões
  pinMode(buttonAccept, INPUT);
  pinMode(buttonReject, INPUT);

  //Setting of the classic PNRD approach  
  pnrd.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);

  Serial.println("\nMachine 2: Part tester."); 
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
        Serial.print("\nNew part. ID: ");
        Serial.println(tagId, HEX);
        
        tagReadyToContinue = false;

         //Fire of transition 0
        FireError fireError = pnrd.fire(0);
     
    switch(fireError){
        case FireError::NO_ERROR :   
        
          //Save new information in tag          
          if(pnrd.saveData() == WriteError::NO_ERROR){
               Serial.println("Part ready to be tested.");
               return;
          }else{
               Serial.println("Error in the tag update.");
               return;
          };
          return;

        case FireError::PRODUCE_EXCEPTION :
          Serial.println("Error: exception. Part has followed a wrong workflow.");
          break;

        case FireError::CONDITIONS_ARE_NOT_APPLIED :   
              
          break;
    } 
    

        
    }else{
      //Able the transitions in case of approval or rejection of the part
      pnrd.conditionUpdate(1, digitalRead(buttonAccept) && !tagReadyToContinue);     
      pnrd.conditionUpdate(2, digitalRead(buttonReject) && !tagReadyToContinue);      
      
        //Fire of transition 1
        fireError = pnrd.fire(1);
         switch(fireError){
        case FireError::NO_ERROR :   
          Serial.println("Part approved.");
          
          //Save new information in tag          
          if(pnrd.saveData() == WriteError::NO_ERROR){
               Serial.println("Ready to be taken to stock.");
               tagReadyToContinue = true;
          }else{
               Serial.println("Error in the tag update.");
          };
          return;

        case FireError::PRODUCE_EXCEPTION :
          Serial.println("Error: exception. Part has followed a wrong workflow.");
          return;

        case FireError::CONDITIONS_ARE_NOT_APPLIED : 
        break;      
          
        }  
        
         //Fire of transition 2
        fireError = pnrd.fire(2);  
        switch(fireError){
        case FireError::NO_ERROR :   
          Serial.println("Part rejected.");
          //Save new information in tag         
          if(pnrd.saveData() == WriteError::NO_ERROR){
               Serial.println("Ready to be reworked.");
               tagReadyToContinue = true;
          }else{
                Serial.println("Error in the tag update.");
          };
          return;

        case FireError::PRODUCE_EXCEPTION :
          Serial.println("Error: exception. Part has followed a wrong workflow.");
          return;

        case FireError::CONDITIONS_ARE_NOT_APPLIED : 
        break;   
        }         
    }
  }  
}
