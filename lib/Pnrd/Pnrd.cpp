#ifndef PNRD_CPP

#define PNRD_CPP

#include "Pnrd.h"

// Constructors

// Goal Token size Included
Pnrd::Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, bool hasConditions, bool hasTagHistory, uint8_t goal_token_size) : PetriNet(num_places, num_transitions, num_max_of_inputs, num_max_of_outputs, hasConditions, goal_token_size)
{
	this->hasTagHistory = hasTagHistory;
	reader = readerPointer;
	preparePnrdMemoryStack();
}

Pnrd::Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, bool hasConditions, bool hasTagHistory) : PetriNet(num_places, num_transitions, num_max_of_inputs, num_max_of_outputs, hasConditions)
{
	this->hasTagHistory = hasTagHistory;
	reader = readerPointer;
	preparePnrdMemoryStack();
}
// Goal Token size Included
Pnrd::Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, uint8_t goal_token_size) : PetriNet(num_places, num_transitions, num_max_of_inputs, num_max_of_outputs, goal_token_size)
{
	reader = readerPointer;
	preparePnrdMemoryStack();
}

Pnrd::Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs) : PetriNet(num_places, num_transitions, num_max_of_inputs, num_max_of_outputs)
{
	reader = readerPointer;
	preparePnrdMemoryStack();
}
// Goal Token size Included
Pnrd::Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasTagHistory) : PetriNet(num_places, num_transitions, hasConditions)
{
	this->hasTagHistory = hasTagHistory;
	reader = readerPointer;
	preparePnrdMemoryStack();
}

Pnrd::Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasTagHistory, bool hasGoalToken) : PetriNet(num_places, num_transitions, hasConditions, hasGoalToken)
{
	this->hasTagHistory = hasTagHistory;
	reader = readerPointer;
	preparePnrdMemoryStack();
}

Pnrd::Pnrd(Reader *readerPointer, uint8_t num_of_places, uint8_t num_of_transitions) : PetriNet(num_of_places, num_of_transitions)
{
	reader = readerPointer;
	preparePnrdMemoryStack();
}

// Destructor
Pnrd::~Pnrd()
{
	if (hasTagHistory)
	{
		free(tagHistory);
	}
}

void Pnrd::setAsTagInformation(PetriNetInformation info)
{
	dataInTag |= 0b1 << ((int)info);
}

void Pnrd::resetAsTagInformation()
{
	dataInTag = 0b0;
}
void Pnrd::setAsDeviceInformation(PetriNetInformation info)
{
	dataInTag &= ~(0b1 << ((int)info));
}

bool Pnrd::isTagInformation(PetriNetInformation info)
{
	return dataInTag & (0b1 << ((int)info));
}

uint8_t *Pnrd::getDataInTag()
{
	return &dataInTag;
}

ReadError Pnrd::getData()
{
	return reader->read(this);
}

WriteError Pnrd::saveData()
{
	reader->readTag(this);

	return reader->write(this);
}

void Pnrd::setTagId(uint32_t tagId)
{
	this->tagId = tagId;
}

uint32_t Pnrd::getTagId()
{
	return tagId;
}

void Pnrd::setDeviceId(uint32_t deviceId)
{
	this->deviceId = deviceId;
}

uint32_t Pnrd::getDeviceId()
{
	return deviceId;
}

bool Pnrd::setTagHistory(TagHistoryEntry *vector, uint8_t numberOfEntries)
{
	bool noError = true;
	if (numberOfEntries > MAX_NUM_OF_TAG_HISTORY_ENTRIES)
	{
		numberOfEntries = MAX_NUM_OF_TAG_HISTORY_ENTRIES;
		noError = false;
	}

	for (uint8_t index = 0; index < numberOfEntries; index++)
	{
		tagHistory[index] = vector[index];
	}

	return noError;
}

uint8_t Pnrd::getTagHistory(TagHistoryEntry *vector)
{
	uint8_t index = 0;

	for (uint8_t counter = tagHistoryIndex + 1; counter < MAX_NUM_OF_TAG_HISTORY_ENTRIES; counter++)
	{
		vector[index] = tagHistory[index];
		if (!(tagHistory[index].Place == 0xFF))
		{
			vector[index] = tagHistory[index];
			index++;
		}
	}

	for (uint8_t counter = 0; counter < tagHistoryIndex; index++, counter++)
	{
		if (!(tagHistory[index].Place == 0xFF))
		{
			vector[index] = tagHistory[index];
			index++;
		}
	}

	return index;
}

TagHistoryEntry *Pnrd::getTagHistoryPointer()
{
	return tagHistory;
}

uint8_t *Pnrd::getTagHistoryIndexPointer()
{
	return &tagHistoryIndex;
}

void Pnrd::printTagHistory()
{
	for (uint8_t counter = tagHistoryIndex; counter > 0; counter--)
	{
		printTagHistoryEntry(tagHistory[counter]);
	}

	if (tagHistory[0].Place != 0xFF)
	{
		printTagHistoryEntry(tagHistory[0]);
	}

	for (uint8_t counter = MAX_NUM_OF_TAG_HISTORY_ENTRIES - 1; counter > tagHistoryIndex; counter--)
	{
		if (tagHistory[counter].Place == 0xFF)
		{
			return;
		}
		printTagHistoryEntry(tagHistory[counter]);
	}
}

void Pnrd::addTagHistoryEntry(TagHistoryEntry entry)
{
	if (tagHistoryIndex == MAX_NUM_OF_TAG_HISTORY_ENTRIES)
	{
		tagHistoryIndex = 0;
	}
	else
	{
		tagHistoryIndex++;
	}

	tagHistory[tagHistoryIndex] = entry;
}

void Pnrd::removeLastTagHistoryEntry()
{
	tagHistory[tagHistoryIndex].Place = 0xFF;

	if (tagHistoryIndex == 0)
	{
		tagHistoryIndex = MAX_NUM_OF_TAG_HISTORY_ENTRIES;
	}
	else
	{
		tagHistoryIndex--;
	}
}

FireError Pnrd::fire()
{
	FireError fireError = this->PetriNet::fire();
	saveTagHistory();
	return fireError;
}

FireError Pnrd::fire(uint8_t transition)
{
	FireError fireError = this->PetriNet::fire(transition);
	saveTagHistory();
	return fireError;
}

// Private Methods
void Pnrd::preparePnrdMemoryStack()
{
	if (hasTagHistory)
	{
		tagHistory = (TagHistoryEntry *)malloc(sizeof(TagHistoryEntry) * MAX_NUM_OF_TAG_HISTORY_ENTRIES);
		for (uint8_t count = 0; count < MAX_NUM_OF_TAG_HISTORY_ENTRIES; count++)
		{
			TagHistoryEntry entry;
			entry.Place = 0xFF;
			tagHistory[count] = entry;
		}
		tagHistoryIndex = 0;
	}
}

void Pnrd::printTagHistoryEntry(TagHistoryEntry entry)
{
	print("Device Id: ");
	print(entry.DeviceId);
	print(". ");

	print("Place: ");
	print(entry.Place);
	print(". ");

	print("Tokens: ");
	print(entry.Tokens);
	print(". ");

	print("Timestamp: ");
	print(entry.TimeStamp);
	print(".\n");
}

void Pnrd::saveTagHistory()
{
	if (hasTagHistory)
	{
		for (uint8_t place = 0; place < getNumberOfPlaces(); place++)
		{
			if (TokenVector[place] > 0)
			{
				TagHistoryEntry entry;
				entry.Place = place;
				entry.DeviceId = getDeviceId();
				entry.Tokens = TokenVector[place];
				entry.TimeStamp = platformInterface->getTimeStamp();
				addTagHistoryEntry(entry);
			}
		}
	}
}

#endif