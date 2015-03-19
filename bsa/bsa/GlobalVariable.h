#pragma once

#include <string>

class GlobalVariable
{
private:

	std::string _variableName;
	
	std::string registerAuxiliaryVariable(std::string minimalName);

public:

	GlobalVariable(std::string variableName, std::vector<int> processes);
	~GlobalVariable();

	std::map<int, std::string> auxiliaryCounterVariableNames;
	std::map<int, std::string> auxiliaryFirstPointerVariableNames;
	std::map<std::pair<int, int>, std::string> auxiliaryWriteBufferVariableNames;
};

