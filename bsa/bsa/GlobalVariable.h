#pragma once

#include <string>

class GlobalVariable
{
private:

	std::string _variableName;
	
	std::string registerAuxiliaryVariable(std::string minimalName);

public:

	GlobalVariable(std::string variableName);
	~GlobalVariable();

	std::map<int, std::string> auxiliaryCounterVariableNames;
	std::map<int, std::string> auxiliaryFirstPointerVariableNames;
};

