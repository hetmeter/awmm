#pragma once

#include <string>
#include <vector>
#include <map>
#include <assert.h>

class VariableEntry
{
public:
/* Enumerations */
	enum VariableType { GLOBAL, LOCAL, AUXILIARY };

/* Constructor and destructor */
	VariableEntry(const std::string &name, VariableType type);
	VariableEntry(const std::string &name, const std::string &globalNameOfAuxiliary);
	~VariableEntry();

/* Public fields */
	const std::string getName();
	void setName(const std::string &value);
	const VariableType getType();

	const std::vector<std::string> getAuxiliaryBufferNames(int process);
	const std::string getAuxiliaryCounterName(int process);
	const std::string getAuxiliaryFirstPointerName(int process);

	const std::string getGlobalName();

	int getMaximumBufferSize(int process);
	void setMaximumBufferSize(int process, int value);

/* Public methods */
	void generateAuxiliaryVariables();

private:

/* Locals */
	std::string _name;
	VariableType _type;

	std::map<int, std::vector<std::string>> _auxiliaryBufferNamesOfGlobal;
	std::map<int, std::string> _auxiliaryCounterNamesOfGlobal;
	std::map<int, std::string> _auxiliaryFirstPointerNamesOfGlobal;

	std::string _globalNameOfAuxiliary;

	std::map<int, int> _maximumBufferSizePerProcess;
};