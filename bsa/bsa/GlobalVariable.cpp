#include "GlobalVariable.h"
#include "config.h"

using namespace std;

GlobalVariable::GlobalVariable(string variableName, vector<int> processes)
{
	_variableName = variableName;

	for (int processNumber : processes)
	{
		auxiliaryCounterVariableNames[processNumber] = registerAuxiliaryVariable(variableName + config::AUXILIARY_VARIABLE_SEPARATOR +
			config::AUXILIARY_COUNTER_TAG + config::AUXILIARY_VARIABLE_SEPARATOR + to_string(processNumber));
		auxiliaryFirstPointerVariableNames[processNumber] = registerAuxiliaryVariable(variableName + config::AUXILIARY_VARIABLE_SEPARATOR +
			config::AUXILIARY_FIRST_POINTER_TAG + config::AUXILIARY_VARIABLE_SEPARATOR + to_string(processNumber));

		for (int ctr = 1; ctr <= config::K; ctr++)
		{
			auxiliaryWriteBufferVariableNames[pair<int, int>(processNumber, ctr)] = registerAuxiliaryVariable(variableName +
				config::AUXILIARY_VARIABLE_SEPARATOR + to_string(ctr) + config::AUXILIARY_VARIABLE_SEPARATOR +
				to_string(processNumber));
		}
	}
}


GlobalVariable::~GlobalVariable()
{
}

string GlobalVariable::registerAuxiliaryVariable(string minimalName)
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

		while (config::stringVectorContains(config::variableNames, minimalName + config::AUXILIARY_VARIABLE_SEPARATOR + randomSuffix))
		{
			randomSuffix = to_string(rand());
		}

		config::variableNames.push_back(minimalName + config::AUXILIARY_VARIABLE_SEPARATOR + randomSuffix);
		return minimalName + config::AUXILIARY_VARIABLE_SEPARATOR + randomSuffix;
	}
}