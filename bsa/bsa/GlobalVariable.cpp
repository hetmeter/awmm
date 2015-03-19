#include "GlobalVariable.h"
#include "config.h"

using namespace std;

GlobalVariable::GlobalVariable(string variableName, vector<int> processes)
{
	_variableName = variableName;

	for (int processNumber : processes)
	{

	}
	auxiliaryCounterVariableName = registerAuxiliaryVariable(variableName + config::AUXILIARY_VARIABLE_SEPARATOR + config::AUXILIARY_COUNTER_TAG + config::)
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