#ifndef PETRI_NET_H

#define PETRI_NET_H

#include <Arduino.h>

enum class FireError
{
	NO_ERROR = 0,
	CONDITIONS_ARE_NOT_APPLIED = 1, // One or more of the conditions necessary for the transition aren't available
	PRODUCE_EXCEPTION = 2,			// The result of the fire is an exception
	NOT_KNOWN = 3
};

/*
PlatformInterface class helps to implement the PetriNet library across platforms.
In most of the cases, the only modifications you will have to make is to extend this class
to match your plaform and replace it on the Platforminterface of the PetriNet class.
*/
class PlatformInterface
{
public:
	virtual void print(char character) = 0;
	virtual uint32_t getTimeStamp() = 0;
};

// Implementation of Arduino-based functions
class Arduino : public PlatformInterface
{

public:
	void print(char character)
	{
		Serial.print(character);
	};

	uint32_t getTimeStamp()
	{
		return millis();
	}
};

class PetriNet
{
protected:
	uint8_t *AdjacencyList;
	uint16_t *TokenVector;
	uint16_t *FireVector;
	uint8_t *Conditions;
	uint16_t *GoalToken;
	bool hasConditions = false;

	bool hasGoalToken = false;
	uint8_t NumberOfPlaces;
	uint8_t NumberOfTransitions;
	uint8_t NumberMaxOfInputs;
	uint8_t NumberMaxOfOutputs;

	uint8_t GoalTokenSize;

	PlatformInterface *platformInterface = new Arduino();

	uint8_t *OutputListAuxiliarPointer;

	void print(char toPrint);
	void print(uint8_t number);
	void print(uint16_t toPrint);
	void print(uint32_t toPrint);
	void print(char *toPrint);

private:
	void prepareMemoryStack();

public:
	PetriNet(uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, bool hasConditions, uint8_t goal_token_size);
	PetriNet(uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, bool hasConditions);
	PetriNet(uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, uint8_t goal_token_size);
	PetriNet(uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs);
	PetriNet(uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasGoalToken);
	PetriNet(uint8_t num_places, uint8_t num_transitions, bool hasConditions);
	PetriNet(uint8_t num_places, uint8_t num_transitions, uint8_t goal_token_size);
	PetriNet(uint8_t num_places, uint8_t num_transitions);
	~PetriNet();
	uint8_t getNumberOfPlaces();
	uint8_t getNumberOfTransitions();
	uint8_t getNumberMaxOfInputs();
	uint8_t getNumberMaxOfOutputs();

	uint8_t getGoalTokenSize();

	void releaseMemory();
	void reconfigure(uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasGoalToken);

	void setFireVector(uint16_t *vector);
	void getFireVector(uint16_t *vector);
	uint16_t *getFireVectorPointer();
	void printFireVector();

	void setTokenVector(uint16_t *vector);
	void getTokenVector(uint16_t *vector);
	uint16_t *getTokenVectorPointer();
	void printTokenVector();

	void setGoalToken(uint16_t *vector);
	void getGoalToken(uint16_t *vector);
	uint16_t *getGoalTokenPointer();
	void printGoalToken();

	bool setIncidenceMatrix(int8_t *matrix);
	void getIncidenceMatrix(int8_t *matrix);
	int8_t getMatrixElement(int8_t place, int8_t transition);
	void printIncidenceMatrix();

	bool addInput(uint8_t place, uint8_t transition);
	bool deleteInput(uint8_t place, uint8_t transition);
	bool addOutput(uint8_t place, uint8_t transition);
	bool deleteOutput(uint8_t place, uint8_t transition);

	uint8_t getInputs(uint8_t transition, uint8_t *inputs);
	bool setInputs(uint8_t transition, uint8_t *inputs, uint8_t inputsCount);
	uint8_t getOutputs(uint8_t transition, uint8_t *outputs);
	bool setOutputs(uint8_t transition, uint8_t *outputs, uint8_t outputsCount);

	uint8_t *getInputsPointer(uint8_t transition);
	uint8_t *getOutputsPointer(uint8_t transition);

	uint8_t *getAdjacencyListPointer();

	void conditionIsSatisfied(uint8_t transition);
	void conditionIsNotSatisfied(uint8_t transition);
	void conditionUpdate(uint8_t transition, bool isSatisfied);

	bool isConditionSatisfied(uint8_t transition);
	uint8_t *getConditionsPointer();
	void printConditions();

	FireError fire();
	FireError fire(uint8_t transition);
	FireError isTriggerable(uint8_t transition);
	FireError isTriggerable();
};

#endif
