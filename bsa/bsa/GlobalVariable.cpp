#include "GlobalVariable.h"
#include "config.h"
#include "literalCode.h"

using namespace std;

GlobalVariable::GlobalVariable(const string &variableName, const vector<int> &processes)
{
	_variableName = variableName;

	for (int processNumber : processes)
	{
		auxiliaryCounterVariableNames[processNumber] = registerAuxiliaryVariable(variableName + literalCode::AUXILIARY_VARIABLE_SEPARATOR +
			literalCode::AUXILIARY_COUNTER_TAG + literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(processNumber));
		auxiliaryFirstPointerVariableNames[processNumber] = registerAuxiliaryVariable(variableName + literalCode::AUXILIARY_VARIABLE_SEPARATOR +
			literalCode::AUXILIARY_FIRST_POINTER_TAG + literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(processNumber));

		for (int ctr = 1; ctr <= config::K; ctr++)
		{
			auxiliaryWriteBufferVariableNames[pair<int, int>(processNumber, ctr)] = registerAuxiliaryVariable(variableName +
				literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(ctr) + literalCode::AUXILIARY_VARIABLE_SEPARATOR +
				to_string(processNumber));
		}
	}
}


GlobalVariable::~GlobalVariable()
{
}

string GlobalVariable::registerAuxiliaryVariable(const string &minimalName)
{
	if (!config::stringVectorContains(config::variableNames, minimalName))
	{
		config::variableNames.push_back(minimalName);
		return minimalName;
	}
	else
	{
		srand(time(NULL));
		string randomSuffix = to_string(rand());

		while (config::stringVectorContains(config::variableNames, minimalName + literalCode::AUXILIARY_VARIABLE_SEPARATOR + randomSuffix))
		{
			randomSuffix = to_string(rand());
		}

		config::variableNames.push_back(minimalName + literalCode::AUXILIARY_VARIABLE_SEPARATOR + randomSuffix);
		return minimalName + literalCode::AUXILIARY_VARIABLE_SEPARATOR + randomSuffix;
	}
}