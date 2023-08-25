#include "PetriNet.h"

// Constructors

PetriNet::PetriNet(uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs, bool hasConditions)
{
	this->hasConditions = hasConditions;

	this->NumberOfPlaces = num_places;
	this->NumberOfTransitions = num_transitions;

	this->NumberMaxOfInputs = num_max_of_inputs;
	this->NumberMaxOfOutputs = num_max_of_outputs;

	prepareMemoryStack();
}

PetriNet::PetriNet(uint8_t num_places, uint8_t num_transitions, uint8_t num_max_of_inputs, uint8_t num_max_of_outputs)
{
	this->NumberOfPlaces = num_places;
	this->NumberOfTransitions = num_transitions;

	this->NumberMaxOfInputs = num_max_of_inputs;
	this->NumberMaxOfOutputs = num_max_of_outputs;

	prepareMemoryStack();
}

PetriNet::PetriNet(uint8_t num_places, uint8_t num_transitions, bool hasConditions)
{
	this->hasConditions = hasConditions;

	this->NumberOfPlaces = num_places;
	this->NumberOfTransitions = num_transitions;

	this->NumberMaxOfInputs = num_places;
	this->NumberMaxOfOutputs = num_places;

	prepareMemoryStack();
}
PetriNet::PetriNet(uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasGoalToken)
{

	this->hasConditions = hasConditions;
	this->hasGoalToken = hasGoalToken;
	this->NumberOfPlaces = num_places;
	this->NumberOfTransitions = num_transitions;

	this->NumberMaxOfInputs = num_places;
	this->NumberMaxOfOutputs = num_places;

	prepareMemoryStack();
}

PetriNet::PetriNet(uint8_t num_places, uint8_t num_transitions)
{
	this->NumberOfPlaces = num_places;
	this->NumberOfTransitions = num_transitions;

	this->NumberMaxOfInputs = num_places;
	this->NumberMaxOfOutputs = num_places;

	prepareMemoryStack();
}

// Destructor
PetriNet::~PetriNet()
{
	free(AdjacencyList);
	free(TokenVector);
	free(FireVector);
	free(Conditions);
	free(GoalToken);
}

void PetriNet::releaseMemory()
{
	delete[] TokenVector;
	delete[] FireVector;
	delete[] AdjacencyList;
	if (hasConditions)
		delete[] Conditions;
	if (hasGoalToken)
		delete[] GoalToken;
}

void PetriNet::reconfigure(uint8_t num_places, uint8_t num_transitions, bool hasConditions, bool hasGoalToken)
{
	releaseMemory();

	this->hasConditions = hasConditions;
	this->hasGoalToken = hasGoalToken;

	this->NumberOfPlaces = num_places;
	this->NumberOfTransitions = num_transitions;

	prepareMemoryStack();
}

// Public methods
uint8_t PetriNet::getNumberOfPlaces()
{
	return NumberOfPlaces;
}

uint8_t PetriNet::getNumberOfTransitions()
{
	return NumberOfTransitions;
}

uint8_t PetriNet::getNumberMaxOfInputs()
{
	return NumberMaxOfInputs;
}

uint8_t PetriNet::getNumberMaxOfOutputs()
{
	return NumberMaxOfOutputs;
}

// Public methods

void PetriNet::setFireVector(uint16_t *vector)
{
	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		FireVector[transition] = vector[transition];
	}
}

void PetriNet::getFireVector(uint16_t *vector)
{
	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		vector[transition] = FireVector[transition];
	}
}

uint16_t *PetriNet::getFireVectorPointer()
{
	return FireVector;
}

void PetriNet::printFireVector()
{
	print("Fire Vector:\n\n");

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		print(FireVector[transition]);
		print('\n');
	}

	print('\n');
}

void PetriNet::setTokenVector(uint16_t *vector)
{
	for (uint8_t place = 0; place < NumberOfPlaces; place++)
	{
		TokenVector[place] = vector[place];
	}
}

void PetriNet::getTokenVector(uint16_t *vector)
{
	if (vector == nullptr)
	{
		Serial.println("Error: Passed a null pointer to getTokenVector.");
		return;
	}
	for (uint8_t place = 0; place < NumberOfPlaces; place++)
	{
		vector[place] = TokenVector[place];
	}
}

uint16_t *PetriNet::getTokenVectorPointer()
{
	return TokenVector;
}

void PetriNet::printTokenVector()
{
	print("Token Vector:\n\n");

	for (int32_t place = 0; place < NumberOfPlaces; place++)
	{
		print(TokenVector[place]);
		print('\n');
	}

	print('\n');
}
//-------------------------------------New Include-----------------------------------------------------
void PetriNet::setGoalToken(uint16_t *vector)
{
	for (uint8_t count = 0; count < NumberOfPlaces; count++)
	{
		GoalToken[count] = vector[count];
	}
}

void PetriNet::getGoalToken(uint16_t *vector)
{
	for (uint8_t count = 0; count < NumberOfPlaces; count++)
	{
		vector[count] = GoalToken[count];
	}
}

uint16_t *PetriNet::getGoalTokenPointer()
{
	return GoalToken;
}

void PetriNet::printGoalToken()
{
	print("Goal Token:\n\n");
	for (int32_t count = 0; count < NumberOfPlaces; count++)
	{
		print(GoalToken[count]);
		print('\n');
	}
	print('\n');
}
//-------------------------------------------------------------------------------------------
bool PetriNet::setIncidenceMatrix(int8_t *matrix)
{
	bool noError = true;

	uint8_t sizeOfAdjacencyList = (NumberMaxOfInputs + NumberMaxOfOutputs) * NumberOfTransitions;
	for (uint8_t listIndex = 0; listIndex < sizeOfAdjacencyList; listIndex++)
	{
		AdjacencyList[listIndex] = 0xFF;
	}

	for (uint8_t place = 0; place < NumberOfPlaces; place++)
	{
		for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
		{
			if (matrix[place * NumberOfTransitions + transition] == 1)
			{
				noError &= addOutput(place, transition);
			}
			else if (matrix[place * NumberOfTransitions + transition] == -1)
			{
				noError &= addInput(place, transition);
			}
		}
	}

	return noError;
}

void PetriNet::getIncidenceMatrix(int8_t *matrix)
{
	uint16_t index;

	for (uint8_t place = 0; place < NumberOfPlaces; place++)
	{
		for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
		{
			index = place * NumberOfTransitions + transition;
			matrix[index] = 0;
		}
	}

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++)
		{
			index = transition * NumberMaxOfInputs + inputIndex;
			if (AdjacencyList[index] == 0xFF)
			{
				break;
			}
			else
			{
				matrix[AdjacencyList[index] * NumberOfTransitions + transition] = 1;
			}
		}
	}

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		for (uint8_t outputIndex = 0; outputIndex < NumberMaxOfInputs; outputIndex++)
		{
			index = transition * NumberMaxOfOutputs + outputIndex;
			if (OutputListAuxiliarPointer[index] == 0xFF)
			{
				break;
			}
			else
			{
				matrix[OutputListAuxiliarPointer[index] * NumberOfTransitions + transition] = -1;
			}
		}
	}
}

int8_t PetriNet::getMatrixElement(int8_t place, int8_t transition)
{
	uint16_t index;

	for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++)
	{
		index = transition * NumberMaxOfInputs + inputIndex;
		if (AdjacencyList[index] == 0xFF)
		{
			break;
		}
		else if (AdjacencyList[index] == place)
		{
			return -1;
		}
	}

	for (uint8_t outputIndex = 0; outputIndex < NumberMaxOfInputs; outputIndex++)
	{
		index = transition * NumberMaxOfOutputs + outputIndex;
		if (OutputListAuxiliarPointer[index] == 0xFF)
		{
			break;
		}
		else if (OutputListAuxiliarPointer[index] == place)
		{
			return 1;
		}
	}

	return 0;
}

void PetriNet::printIncidenceMatrix()
{
	print("Incidence Matrix: \n\n");

	for (uint8_t place = 0; place < NumberOfPlaces; place++)
	{
		for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
		{
			switch (getMatrixElement(place, transition))
			{
			case 1:
				print("1 ");
				break;
			case -1:
				print("-1 ");
				break;
			case 0:
				print("0 ");
				break;
			}
		}
		print('\n');
	}

	print('\n');
}

bool PetriNet::addInput(uint8_t place, uint8_t transition)
{
	uint16_t index = transition * NumberMaxOfInputs;

	for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
	{
		if (AdjacencyList[index] == 0xFF)
		{
			AdjacencyList[index] = place;
			return true;
		}
		else if (AdjacencyList[index] == place)
		{
			return false;
		}
	}
	return false;
}

bool PetriNet::deleteInput(uint8_t place, uint8_t transition)
{
	uint16_t index = transition * NumberMaxOfInputs;

	for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
	{
		if (AdjacencyList[index] == 0xFF)
		{
			return false;
		}
		else if (AdjacencyList[index] == place)
		{
			for (; inputIndex < NumberMaxOfInputs - 1; inputIndex++, index++)
			{
				AdjacencyList[index] = AdjacencyList[index] + 1;
			}
			AdjacencyList[index] = 0xFF;
			return true;
		}
	}
	return false;
}

bool PetriNet::addOutput(uint8_t place, uint8_t transition)
{
	uint16_t index = transition * NumberMaxOfOutputs;

	for (uint8_t outputIndex = 0; outputIndex < NumberMaxOfOutputs; outputIndex++, index++)
	{
		if (OutputListAuxiliarPointer[index] == 0xFF)
		{
			OutputListAuxiliarPointer[index] = place;
			return true;
		}
		else if (OutputListAuxiliarPointer[index] == place)
		{
			return false;
		}
	}

	return false;
}

bool PetriNet::deleteOutput(uint8_t place, uint8_t transition)
{
	uint16_t index = transition * NumberMaxOfInputs;

	for (uint8_t outputIndex = 0; outputIndex < NumberMaxOfInputs; outputIndex++, index++)
	{
		if (OutputListAuxiliarPointer[index] == 0xFF)
		{
			return false;
		}
		else if (OutputListAuxiliarPointer[index] == place)
		{
			for (; outputIndex < NumberMaxOfInputs - 1; outputIndex++, index++)
			{
				OutputListAuxiliarPointer[index] = OutputListAuxiliarPointer[index] + 1;
			}
			OutputListAuxiliarPointer[index] = 0xFF;
			return true;
		}
	}
	return false;
}

uint8_t PetriNet::getInputs(uint8_t transition, uint8_t *inputs)
{
	uint8_t counter = 0;
	uint16_t index = transition * NumberMaxOfInputs;

	for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
	{
		if (AdjacencyList[index] == 0xFF)
		{
			return counter;
		}
		else
		{
			inputs[counter] = AdjacencyList[index];
		}
	}

	return counter;
}

bool PetriNet::setInputs(uint8_t transition, uint8_t *inputs, uint8_t inputsCount)
{
	bool noError = true;
	uint16_t index = transition * NumberMaxOfInputs;

	if (inputsCount > NumberMaxOfInputs)
	{
		inputsCount = NumberMaxOfInputs;
		noError = false;
	}

	uint8_t inputIndex = 0;
	for (; inputIndex < inputsCount; inputIndex++, index++)
	{
		AdjacencyList[index] = inputs[inputIndex];
	}

	for (; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
	{
		AdjacencyList[index] = 0xFF;
	}

	return noError;
}

uint8_t PetriNet::getOutputs(uint8_t transition, uint8_t *outputs)
{
	uint8_t counter = 0;
	uint16_t index = transition * NumberMaxOfOutputs;

	for (uint8_t outputIndex = 0; outputIndex < NumberMaxOfOutputs; outputIndex++, index++)
	{
		if (OutputListAuxiliarPointer[index] == 0xFF)
		{
			return counter;
		}
		else
		{
			outputs[counter] = OutputListAuxiliarPointer[index];
		}
	}

	return counter;
}

bool PetriNet::setOutputs(uint8_t transition, uint8_t *outputs, uint8_t outputsCount)
{
	bool noError = true;
	uint16_t index = transition * NumberMaxOfOutputs;

	if (outputsCount > NumberMaxOfOutputs)
	{
		outputsCount = NumberMaxOfOutputs;
		noError = false;
	}

	uint8_t outputIndex = 0;
	for (; outputIndex < outputsCount; outputIndex++, index++)
	{
		OutputListAuxiliarPointer[index] = outputs[outputIndex];
	}

	for (; outputIndex < NumberMaxOfOutputs; outputIndex++, index++)
	{
		OutputListAuxiliarPointer[index] = 0xFF;
	}

	return noError;
}

uint8_t *PetriNet::getInputsPointer(uint8_t transition)
{
	return AdjacencyList + transition * NumberMaxOfInputs;
}

uint8_t *PetriNet::getOutputsPointer(uint8_t transition)
{
	return OutputListAuxiliarPointer + transition * NumberMaxOfOutputs;
}

uint8_t *PetriNet::getAdjacencyListPointer()
{
	return AdjacencyList;
}

void PetriNet::conditionIsSatisfied(uint8_t transition)
{
	Conditions[transition / 8] |= 0b00000001 << transition % 8;
}

void PetriNet::conditionIsNotSatisfied(uint8_t transition)
{
	Conditions[transition / 8] &= ~(0b00000001 << transition % 8);
}

void PetriNet::conditionUpdate(uint8_t transition, bool isSatisfied)
{
	if (isSatisfied)
	{
		conditionIsSatisfied(transition);
	}
	else
	{
		conditionIsNotSatisfied(transition);
	}
}

bool PetriNet::isConditionSatisfied(uint8_t transition)
{
	return Conditions[transition / 8] & (0b00000001 << transition % 8);
}

uint8_t *PetriNet::getConditionsPointer()
{
	return Conditions;
}

void PetriNet::printConditions()
{
	print("Conditions: \n\n");

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		print(transition);
		if (isConditionSatisfied(transition))
		{
			print(": Satisfied\n");
		}
		else
		{
			print(": Not satisfied\n");
		}
	}

	print('\n');
}

FireError PetriNet::fire()
{

	if (hasConditions)
	{
		for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
		{
			if (FireVector[transition] > 0)
			{
				if (!isConditionSatisfied(transition))
				{
					return FireError::CONDITIONS_ARE_NOT_APPLIED;
				}
			}
		}
	}

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		uint16_t fireQuantity = FireVector[transition];
		uint16_t index = transition * NumberMaxOfInputs;

		for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
		{
			if (AdjacencyList[index] == 0xFF)
			{
				break;
			}
			else
			{
				if (TokenVector[AdjacencyList[index]] < fireQuantity)
				{
					// Return the token vector to its initial value when an exception occurs
					for (inputIndex--; inputIndex < 0; inputIndex--, index--)
					{
						TokenVector[AdjacencyList[index]] += fireQuantity;
					}

					for (transition--; transition < 0; transition--)
					{
						fireQuantity = FireVector[transition];
						index = transition * NumberMaxOfInputs;

						for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
						{
							if (AdjacencyList[index] == 0xFF)
							{
								break;
							}
							else
							{
								TokenVector[AdjacencyList[index]] -= fireQuantity;
							}
						}
					}

					return FireError::PRODUCE_EXCEPTION;
				}
				else
				{
					TokenVector[AdjacencyList[index]] -= fireQuantity;
				}
			}
		}
	}

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		uint16_t fireQuantity = FireVector[transition];
		uint16_t index = transition * NumberMaxOfOutputs;

		for (uint8_t outputIndex = 0; outputIndex < NumberMaxOfOutputs; outputIndex++, index++)
		{
			if (OutputListAuxiliarPointer[index] == 0xFF)
			{
				break;
			}
			else
			{
				TokenVector[OutputListAuxiliarPointer[index]] += fireQuantity;
			}
		}
	}

	return FireError::NO_ERROR;
}

FireError PetriNet::fire(uint8_t transition)
{

	if (hasConditions)
	{
		if (!isConditionSatisfied(transition))
		{
			return FireError::CONDITIONS_ARE_NOT_APPLIED;
		}
	}

	uint16_t index = transition * NumberMaxOfInputs;
	for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
	{
		if (AdjacencyList[index] == 0xFF)
		{
			break;
		}
		else
		{
			if (TokenVector[AdjacencyList[index]] < 1)
			{
				// Return the token vector to its initial value when an exception occurs
				for (transition--; transition < 0; transition--)
				{
					index = transition * NumberMaxOfInputs;

					for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
					{
						if (AdjacencyList[index] == 0xFF)
						{
							break;
						}
						else
						{
							TokenVector[AdjacencyList[index]]++;
						}
					}
				}

				return FireError::PRODUCE_EXCEPTION;
			}
			else
			{
				TokenVector[AdjacencyList[index]]--;
			}
		}
	}

	index = transition * NumberMaxOfOutputs;
	for (uint8_t outputIndex = 0; outputIndex < NumberMaxOfOutputs; outputIndex++, index++)
	{
		if (OutputListAuxiliarPointer[index] == 0xFF)
		{
			break;
		}
		else
		{
			TokenVector[OutputListAuxiliarPointer[index]]++;
		}
	}

	return FireError::NO_ERROR;
}

FireError PetriNet::isTriggerable(uint8_t transition)
{

	if (hasConditions)
	{
		for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
		{
			if (FireVector[transition] > 0)
			{
				if (!isConditionSatisfied(transition))
				{
					return FireError::CONDITIONS_ARE_NOT_APPLIED;
				}
			}
		}
	}

	uint16_t index = transition * NumberMaxOfInputs;
	for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
	{
		if (AdjacencyList[index] == 0xFF)
		{
			break;
		}
		else
		{
			if (TokenVector[AdjacencyList[index]] < 1)
			{
				return FireError::PRODUCE_EXCEPTION;
			}
		}
	}

	return FireError::NO_ERROR;
}

FireError PetriNet::isTriggerable()
{

	if (hasConditions)
	{
		for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
		{
			if (FireVector[transition] > 0)
			{
				if (!isConditionSatisfied(transition))
				{
					return FireError::CONDITIONS_ARE_NOT_APPLIED;
				}
			}
		}
	}

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		uint16_t fireQuantity = FireVector[transition];
		uint16_t index = transition * NumberMaxOfInputs;

		for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
		{
			if (AdjacencyList[index] == 0xFF)
			{
				break;
			}
			else
			{
				if (TokenVector[AdjacencyList[index]] < fireQuantity)
				{
					// Return the token vector to its initial value when an exception occurs
					for (inputIndex--; inputIndex < 0; inputIndex--, index--)
					{
						TokenVector[AdjacencyList[index]] += fireQuantity;
					}

					for (transition--; transition < 0; transition--)
					{
						fireQuantity = FireVector[transition];
						index = transition * NumberMaxOfInputs;

						for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
						{
							if (AdjacencyList[index] == 0xFF)
							{
								break;
							}
							else
							{
								TokenVector[AdjacencyList[index]] -= fireQuantity;
							}
						}
					}

					return FireError::PRODUCE_EXCEPTION;
				}
				else
				{
					TokenVector[AdjacencyList[index]] -= fireQuantity;
				}
			}
		}
	}

	for (uint8_t transition = 0; transition < NumberOfTransitions; transition++)
	{
		uint16_t fireQuantity = FireVector[transition];
		uint16_t index = transition * NumberMaxOfInputs;

		for (uint8_t inputIndex = 0; inputIndex < NumberMaxOfInputs; inputIndex++, index++)
		{
			if (AdjacencyList[index] == 0xFF)
			{
				break;
			}
			else
			{
				TokenVector[AdjacencyList[index]] += fireQuantity;
			}
		}
	}

	return FireError::NO_ERROR;
}

// Private methods
void PetriNet::prepareMemoryStack()
{
	TokenVector = new uint16_t[NumberOfPlaces];
	FireVector = new uint16_t[NumberOfTransitions];

	uint16_t sizeOfAdjacencyList = (NumberMaxOfInputs + NumberMaxOfOutputs) * NumberOfTransitions;
	AdjacencyList = new uint8_t[sizeOfAdjacencyList];
	OutputListAuxiliarPointer = AdjacencyList + (NumberMaxOfInputs * NumberOfTransitions);

	for (uint8_t listIndex = 0; listIndex < sizeOfAdjacencyList; listIndex++)
	{
		AdjacencyList[listIndex] = 0xFF;
	}

	if (hasConditions)
	{
		int8_t sizeOfConditions = (NumberOfTransitions + 7) / 8;
		Conditions = new uint8_t[sizeOfConditions];
		for (uint8_t conditionsIndex = 0; conditionsIndex < sizeOfConditions; conditionsIndex++)
		{
			Conditions[conditionsIndex] = 0xFF; // Set all conditions to true;
		}
	}
	if (hasGoalToken)
	{
		GoalToken = new uint16_t[NumberOfPlaces];
	}
}

// Protected Methods
void PetriNet::print(char toPrint)
{
	platformInterface->print(toPrint);
}

void PetriNet::print(uint8_t toPrint)
{

	for (uint8_t base = 100; base >= 1; base = base / 10)
	{
		uint8_t num = toPrint / base;
		if (base == 1 || num != 0)
		{
			platformInterface->print('0' + num);
		}
		toPrint = toPrint % base;
	}
}

void PetriNet::print(uint16_t toPrint)
{
	for (uint16_t base = 10000; base >= 1; base = base / 10)
	{
		uint16_t num = toPrint / base;
		if (base == 1 || num != 0)
		{
			platformInterface->print('0' + num);
		}
		toPrint = toPrint % base;
	}
}

void PetriNet::print(uint32_t toPrint)
{
	for (uint32_t base = 1000000000; base >= 1; base = base / 10)
	{
		uint32_t num = toPrint / base;
		if (base == 1 || num != 0)
		{
			platformInterface->print('0' + num);
		}
		toPrint = toPrint % base;
	}
}

void PetriNet::print(char *toPrint)
{
	uint8_t counter = 0;
	while (toPrint[counter] != '\0')
	{
		platformInterface->print(toPrint[counter]);
		counter++;
	}
}