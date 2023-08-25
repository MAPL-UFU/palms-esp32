#ifndef PNRD_H

#define PNRD_H

#include "PetriNet.h"

#ifndef MAX_NUM_OF_TAG_HISTORY_ENTRIES
#define MAX_NUM_OF_TAG_HISTORY_ENTRIES 10
#endif

enum class PetriNetInformation
{
	TOKEN_VECTOR = 0,
	ADJACENCY_LIST = 1,
	FIRE_VECTOR = 2,
	CONDITIONS = 3,
	GOAL_TOKEN = 4,
	TAG_HISTORY = 5,
};

enum class ReadError
{
	NO_ERROR = 0,
	TAG_NOT_PRESENT = 1,
	INFORMATION_NOT_PRESENT = 2,
	DATA_SIZE_NOT_COMPATIBLE = 3,
	NOT_AUTORIZED = 4,
	VERSION_NOT_SUPPORTED = 5,
	ERROR_UNKNOWN = 6,
	ERROR_GET_GOAL_TOKEN = 7
};

enum class WriteError
{
	NO_ERROR = 0,
	TAG_NOT_PRESENT = 1,
	INFORMATION_NOT_SAVED = 2,
	NOT_ENOUGH_SPACE = 3,
	NOT_AUTORIZED = 4,
	VERSION_NOT_SUPPORTED = 5,
	ERROR_UNKNOWN = 6,
	ERROR_GET_GOAL_TOKEN = 7
};

enum class PnrdPolicy
{
	DEFAULT_SETUP = 0,
	TAG_SETUP = 1
};

struct TagHistoryEntry
{
	uint16_t DeviceId = 0;
	uint8_t Place = 255;
	uint16_t Tokens = 0;
	uint32_t TimeStamp = 0;
};

class Reader;

class Pnrd : public PetriNet
{

private:
	Reader *reader;
	uint8_t dataInTag;

	uint32_t tagId;
	uint32_t deviceId;

	TagHistoryEntry *tagHistory;
	uint8_t tagHistoryIndex = 0;

	bool hasTagHistory;

private:
	void preparePnrdMemoryStack();

	void printTagHistoryEntry(TagHistoryEntry entry);
	void saveTagHistory();

public:
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, bool hasConditions, bool hasTagHistory, uint8_t goal_token_size);
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, bool hasConditions, bool hasTagHistory);
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, uint8_t goal_token_size);
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs);
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasTagHistory, bool hasGoalToken);
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasTagHistory);
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions, uint8_t goal_token_size);
	Pnrd(Reader *readerPointer, uint8_t num_places, uint8_t num_transitions);
	~Pnrd();

	void setAsTagInformation(PetriNetInformation info);
	void setAsDeviceInformation(PetriNetInformation info);
	void resetAsTagInformation();
	bool isTagInformation(PetriNetInformation info);
	uint8_t *getDataInTag();

	ReadError getData();
	WriteError saveData();

	void setTagId(uint32_t tagId);
	uint32_t getTagId();

	void setDeviceId(uint32_t deviceId);
	uint32_t getDeviceId();

	bool setTagHistory(TagHistoryEntry *vector, uint8_t numberOfEntries);
	uint8_t getTagHistory(TagHistoryEntry *vector);
	TagHistoryEntry *getTagHistoryPointer();
	uint8_t *getTagHistoryIndexPointer();
	void printTagHistory();

	void addTagHistoryEntry(TagHistoryEntry entry);
	void removeLastTagHistoryEntry();

	FireError fire();
	FireError fire(uint8_t transition);

	PnrdPolicy setup = PnrdPolicy::DEFAULT_SETUP;
};

class Reader
{
public:
	virtual ReadError read(Pnrd *pnrd) = 0;
	virtual WriteError write(Pnrd *pnrd) = 0;
	virtual void readTag(Pnrd *pnrd);
};

#endif