#ifndef PN532_NFC_READER_CPP

#define PN532_NFC_READER_CPP

#include "Pn532NfcReader.h"

static const byte comparisonByteVector[] = {'P', 'N', 'R', 'D'};
String comparisonString = "PNRD";

Pn532NfcReader::Pn532NfcReader(NfcAdapter *adapter) : Reader()
{
	nfcAdapter = adapter;
}

void Pn532NfcReader::initialize()
{
	nfcAdapter->begin();
};

void Pn532NfcReader::readTag(Pnrd *pnrd)
{
	if (nfcAdapter->tagPresent())
	{
		NfcTag tag = nfcAdapter->read();
		if (tag.hasNdefMessage())
		{
			NdefMessage message = tag.getNdefMessage();
			int recordCount = message.getRecordCount();
			for (int recordNumber = 0; recordNumber < recordCount; recordNumber++)
			{

				int uidLength = tag.getUidLength();
				byte uid[uidLength];
				tag.getUid(uid, uidLength);

				// Create uint32_t for store concatenated HEX
				uint32_t bignum = 0;
				// create charTagId for Concate Hex byte
				String charTagId;
				for (uint8_t index = 0; index < uidLength; index++)
				{
					charTagId += String(uid[index], HEX);
				}
				// Convert String to Uint32_t HEX
				bignum = strtoul(charTagId.c_str(), NULL, 16);
				// Stores only the two last hex values of the uid as the tag id;
				pnrd->setTagId(bignum);
			}
		}
	}
};

ReadError Pn532NfcReader::read(Pnrd *pnrd)
{
	if (nfcAdapter->tagPresent())
	{
		NfcTag tag = nfcAdapter->read();
		if (tag.hasNdefMessage())
		{
			NdefMessage message = tag.getNdefMessage();
			int recordCount = message.getRecordCount();
			NdefRecord record;
			for (int recordNumber = 0; recordNumber < recordCount; recordNumber++)
			{
				record = message.getRecord(recordNumber);

				if (record.getType().equals(comparisonString))
				{
					int payloadLength = record.getPayloadLength();
					byte payload[payloadLength];
					record.getPayload(payload);

					int uidLength = tag.getUidLength();
					byte uid[uidLength];
					tag.getUid(uid, uidLength);

					// Create uint32_t for store concatenated HEX
					uint32_t bignum = 0;
					// create charTagId for Concate Hex byte
					String charTagId;
					for (uint8_t index = 0; index < uidLength; index++)
					{
						charTagId += String(uid[index], HEX);
					}
					// Convert String to Uint32_t HEX
					bignum = strtoul(charTagId.c_str(), NULL, 16);
					// Stores only the two last hex values of the uid as the tag id;
					pnrd->setTagId(bignum);

					return getInformation(payload, pnrd);
				}
			}

			return ReadError::INFORMATION_NOT_PRESENT;
		}
		else
		{
			return ReadError::INFORMATION_NOT_PRESENT;
		}
	}
	else
	{
		return ReadError::TAG_NOT_PRESENT;
	}

	return ReadError::ERROR_UNKNOWN;
};

WriteError Pn532NfcReader::write(Pnrd *pnrd)
{
	if (nfcAdapter->tagPresent())
	{
		NdefRecord record = NdefRecord();
		int typeSize = 4;
		NdefMessage writeMessage = NdefMessage();
		record.setType(comparisonByteVector, typeSize);
		uint16_t size = 4;

		if (pnrd->isTagInformation(PetriNetInformation::TOKEN_VECTOR))
		{
			size += sizeof(uint16_t) * pnrd->getNumberOfPlaces();
		}

		if (pnrd->isTagInformation(PetriNetInformation::ADJACENCY_LIST))
		{
			size += 2;
			size += (pnrd->getNumberMaxOfInputs() + pnrd->getNumberMaxOfOutputs()) * pnrd->getNumberOfTransitions();
		}

		if (pnrd->isTagInformation(PetriNetInformation::FIRE_VECTOR))
		{
			size += sizeof(uint16_t) * (pnrd->getNumberOfTransitions());
		}

		if (pnrd->isTagInformation(PetriNetInformation::CONDITIONS))
		{
			size += (pnrd->getNumberOfTransitions() + 7) / 8;
		}

		if (pnrd->isTagInformation(PetriNetInformation::TAG_HISTORY))
		{
			size += 2;
			size += sizeof(TagHistoryEntry) * MAX_NUM_OF_TAG_HISTORY_ENTRIES;
		}

		if (pnrd->isTagInformation(PetriNetInformation::GOAL_TOKEN))
		{
			size += sizeof(uint16_t) * pnrd->getNumberOfPlaces();
		}

		if (size > 1024)
		{
			return WriteError::NOT_ENOUGH_SPACE;
		}

		byte pointer[size];

		setInformation(pointer, pnrd);

		record.setPayload(pointer, size);
		writeMessage.addRecord(record);

		if (nfcAdapter->write(writeMessage))
		{
			return WriteError::NO_ERROR;
		}
		else
		{
			free(pointer);
			return WriteError::INFORMATION_NOT_SAVED;
		}
	}
	else
	{
		return WriteError::TAG_NOT_PRESENT;
	}

	return WriteError::ERROR_UNKNOWN;
};

ReadError Pn532NfcReader::getInformation(byte *payload, Pnrd *pnrd)
{
	uint16_t index = 0;

	byte version = payload[index];
	index++;

	if (version != (byte)1)
	{
		return ReadError::VERSION_NOT_SUPPORTED;
	}

	if (pnrd->setup == PnrdPolicy::TAG_SETUP)
	{
		pnrd->getDataInTag()[0] = payload[index];
		index++;
	}
	else
	{
		if (payload[index] != pnrd->getDataInTag()[0])
		{
			return ReadError::INFORMATION_NOT_PRESENT;
		}
		index++;
	}

	uint8_t size = (uint8_t)payload[index];
	index++;
	if (size != pnrd->getNumberOfPlaces())
	{
		Serial.println("Places");
		Serial.println(size);
		return ReadError::DATA_SIZE_NOT_COMPATIBLE;
	}

	size = (uint8_t)payload[index];
	index++;
	if (size != pnrd->getNumberOfTransitions())
	{
		Serial.println("Transitions");
		Serial.println(size);

		return ReadError::DATA_SIZE_NOT_COMPATIBLE;
	}

	if (pnrd->isTagInformation(PetriNetInformation::TOKEN_VECTOR))
	{
		size = sizeof(uint16_t) * pnrd->getNumberOfPlaces();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			((byte *)pnrd->getTokenVectorPointer())[counter] = payload[index];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::ADJACENCY_LIST))
	{
		size = (uint8_t)payload[index];
		index++;
		if (size != pnrd->getNumberMaxOfInputs())
		{
			return ReadError::DATA_SIZE_NOT_COMPATIBLE;
		}

		size = (uint8_t)payload[index];
		index++;
		if (size != pnrd->getNumberMaxOfOutputs())
		{
			return ReadError::DATA_SIZE_NOT_COMPATIBLE;
		}

		size = sizeof(uint8_t) * (pnrd->getNumberMaxOfInputs() + pnrd->getNumberMaxOfOutputs()) * pnrd->getNumberOfTransitions();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			((byte *)pnrd->getAdjacencyListPointer())[counter] = payload[index];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::FIRE_VECTOR))
	{
		size = sizeof(uint16_t) * pnrd->getNumberOfTransitions();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			((byte *)pnrd->getFireVectorPointer())[counter] = payload[index];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::CONDITIONS))
	{
		size = sizeof(uint8_t) * (pnrd->getNumberOfTransitions() + 7) / 8;
		for (uint16_t counter = 0; counter < size; counter++)
		{
			((byte *)pnrd->getConditionsPointer())[counter] = payload[index];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::TAG_HISTORY))
	{
		uint8_t *tagIndex = pnrd->getTagHistoryIndexPointer();

		size = sizeof(uint8_t);
		for (uint16_t counter = 0; counter < size; counter++)
		{
			((byte *)tagIndex)[counter] = payload[index];
			index++;
		}

		size = sizeof(TagHistoryEntry) * MAX_NUM_OF_TAG_HISTORY_ENTRIES;
		for (uint16_t counter = 0; counter < size; counter++)
		{
			((byte *)pnrd->getTagHistoryPointer())[counter] = payload[index];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::GOAL_TOKEN))
	{
		size = sizeof(uint16_t) * pnrd->getNumberOfPlaces();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			((byte *)pnrd->getGoalTokenPointer())[counter] = payload[index];
			index++;
		}
	}

	return ReadError::NO_ERROR;
}

WriteError Pn532NfcReader::setInformation(byte *payload, Pnrd *pnrd)
{
	uint8_t version = 1;
	uint16_t index = 0;
	uint16_t size = 0;

	payload[index] = version;
	index++;
	payload[index] = pnrd->getDataInTag()[0];
	index++;
	payload[index] = pnrd->getNumberOfPlaces();
	index++;
	payload[index] = pnrd->getNumberOfTransitions();
	index++;

	if (pnrd->isTagInformation(PetriNetInformation::TOKEN_VECTOR))
	{
		size = sizeof(uint16_t) * pnrd->getNumberOfPlaces();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			payload[index] = ((byte *)pnrd->getTokenVectorPointer())[counter];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::ADJACENCY_LIST))
	{
		payload[index] = pnrd->getNumberMaxOfInputs();
		index++;
		payload[index] = pnrd->getNumberMaxOfOutputs();
		index++;
		size = sizeof(uint8_t) * (pnrd->getNumberMaxOfInputs() + pnrd->getNumberMaxOfOutputs()) * pnrd->getNumberOfTransitions();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			payload[index] = ((byte *)pnrd->getAdjacencyListPointer())[counter];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::FIRE_VECTOR))
	{
		size = sizeof(uint16_t *) * pnrd->getNumberOfTransitions();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			payload[index] = ((byte *)pnrd->getFireVectorPointer())[counter];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::CONDITIONS))
	{
		size = (pnrd->getNumberOfTransitions() + 7) / 8;
		for (uint16_t counter = 0; counter < size; counter++)
		{
			payload[index] = ((byte *)pnrd->getConditionsPointer())[counter];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::TAG_HISTORY))
	{
		uint8_t *tagIndex = pnrd->getTagHistoryIndexPointer();
		byte *pointer = (byte *)pnrd->getTagHistoryPointer();

		payload[index] = tagIndex[0];
		index++;
		size = sizeof(TagHistoryEntry) * MAX_NUM_OF_TAG_HISTORY_ENTRIES;
		for (uint16_t counter = 0; counter < size; counter++)
		{
			payload[index] = pointer[counter];
			index++;
		}
	}

	if (pnrd->isTagInformation(PetriNetInformation::GOAL_TOKEN))
	{
		size = sizeof(uint16_t) * pnrd->getNumberOfPlaces();
		for (uint16_t counter = 0; counter < size; counter++)
		{
			payload[index] = ((byte *)pnrd->getGoalTokenPointer())[counter];
			index++;
		}
	}

	return WriteError::NO_ERROR;
}
#endif