#ifndef PN532_NFC_READER_H

#define PN532_NFC_READER_H

#include <Arduino.h>
#include "Pnrd.h"

#include <PN532.h>
#include <NfcAdapter.h>

class Pn532NfcReader : public Reader
{
private:
	NfcAdapter *nfcAdapter;
	ReadError getInformation(byte *payload, Pnrd *pnrd);
	WriteError setInformation(byte *payload, Pnrd *pnrd);

public:
	void initialize();

	Pn532NfcReader(NfcAdapter *adapter);

	ReadError read(Pnrd *pnrd);
	WriteError write(Pnrd *pnrd);
	void readTag(Pnrd *pnrd);
};
#endif