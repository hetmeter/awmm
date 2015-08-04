#pragma once

#include <string>
#include <vector>
#include <map>
#include <assert.h>

class PredicateData;

class VariableEntry
{
public:
/* Enumerations */
	enum VariableType { GLOBAL, LOCAL, AUXILIARY };
	enum AuxiliaryType { BUFFER, COUNTER, FIRST_POINTER };

/* Constructor and destructor */
	VariableEntry(const std::string &name, VariableType type, PredicateData* associatedPredicate = nullptr);
	VariableEntry(const std::string &name, const std::string &globalNameOfAuxiliary, AuxiliaryType auxiliaryType, int auxiliaryScope);
	~VariableEntry();

/* Public fields */
	const std::string getName();
	void setName(const std::string &value);
	const VariableType getType();
	const AuxiliaryType getAuxiliaryType();
	int getAuxiliaryScope();
	PredicateData* getAssociatedPredicate();

	const std::vector<std::string> getAuxiliaryBufferNames(int process);
	const std::string getAuxiliaryCounterName(int process);
	const std::string getAuxiliaryFirstPointerName(int process);

	const std::string getGlobalName();

	const std::string getBooleanVariantName();
	const std::string getTemporaryVariantName();

	int getMaximumBufferSize(int process);
	void increaseMaximumBufferSize(int process, int value);

/* Public methods */
	void generateAuxiliaryVariables();

private:

/* Locals */
	std::string _name;
	VariableType _type;
	AuxiliaryType _auxiliaryType;
	int _auxiliaryScope;
	PredicateData* _associatedPredicate = nullptr;

	std::map<int, std::vector<std::string>> _auxiliaryBufferNamesOfGlobal;
	std::map<int, std::string> _auxiliaryCounterNamesOfGlobal;
	std::map<int, std::string> _auxiliaryFirstPointerNamesOfGlobal;

	std::string _globalNameOfAuxiliary;

	std::map<int, int> _maximumBufferSizePerProcess;
};