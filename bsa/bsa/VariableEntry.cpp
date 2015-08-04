#include "VariableEntry.h"

#include "config.h"
#include "bufferSizeMap.h"
#include "literalCode.h"
#include "PredicateData.h"
#include "Ast.h"

using namespace std;

/* Constructor and destructor */
	VariableEntry::VariableEntry(const string &name, VariableType type, PredicateData* associatedPredicate)
	{
		assert(type != AUXILIARY);

		_name = name;
		_type = type;
		_associatedPredicate = associatedPredicate;
	}

	VariableEntry::VariableEntry(const string &name, const string &globalNameOfAuxiliary, AuxiliaryType auxiliaryType, int auxiliaryScope)
	{
		_name = name;
		_type = AUXILIARY;
		_auxiliaryType = auxiliaryType;
		_globalNameOfAuxiliary = globalNameOfAuxiliary;
		_auxiliaryScope = auxiliaryScope;
	}

	VariableEntry::~VariableEntry()
	{
	}

/* Public fields */
	const string VariableEntry::getName()
	{
		return _name;
	}

	void VariableEntry::setName(const string &value)
	{
		_name = value;
	}

	const VariableEntry::VariableType VariableEntry::getType()
	{
		return _type;
	}

	const VariableEntry::AuxiliaryType VariableEntry::getAuxiliaryType()
	{
		return _auxiliaryType;
	}

	int VariableEntry::getAuxiliaryScope()
	{
		assert(_type == AUXILIARY);

		return _auxiliaryScope;
	}

	PredicateData* VariableEntry::getAssociatedPredicate()
	{
		assert(_associatedPredicate != nullptr);

		return _associatedPredicate;
	}

	const vector<string> VariableEntry::getAuxiliaryBufferNames(int process)
	{
		assert(_type == GLOBAL);

		return _auxiliaryBufferNamesOfGlobal[process];
	}

	const string VariableEntry::getAuxiliaryCounterName(int process)
	{
		assert(_type == GLOBAL);

		return _auxiliaryCounterNamesOfGlobal[process];
	}

	const string VariableEntry::getAuxiliaryFirstPointerName(int process)
	{
		assert(_type == GLOBAL);

		return _auxiliaryFirstPointerNamesOfGlobal[process];
	}

	const string VariableEntry::getGlobalName()
	{
		assert(_type == AUXILIARY);

		return _globalNameOfAuxiliary;
	}

	const string VariableEntry::getBooleanVariantName()
	{
		if (_associatedPredicate != nullptr)
		{
			return _associatedPredicate->getSingleBooleanVariableName();
		}

		return "";
	}

	const string VariableEntry::getTemporaryVariantName()
	{
		if (_associatedPredicate != nullptr)
		{
			return _associatedPredicate->getSingleTemporaryVariableName();
		}

		return "";
	}

	int VariableEntry::getMaximumBufferSize(int process)
	{
		if (_maximumBufferSizePerProcess.find(process) == _maximumBufferSizePerProcess.end())
		{
			_maximumBufferSizePerProcess[process] = 0;
		}

		return _maximumBufferSizePerProcess[process];
	}

	void VariableEntry::increaseMaximumBufferSize(int process, int value)
	{
		if (value == bsm::TOP_VALUE)
		{
			_maximumBufferSizePerProcess[process] = config::K;
		}
		else if (value > getMaximumBufferSize(process))
		{
			_maximumBufferSizePerProcess[process] = value + 1;
		}
	}

/* Public methods */
	// Registers a counter, a first pointer, and K buffer variables for each process
	void VariableEntry::generateAuxiliaryVariables()
	{
		for (int process : config::processes)
		{
			_auxiliaryCounterNamesOfGlobal.insert(pair<int, string>(process, config::forceRegisterSymbol(new VariableEntry(
					_name + literalCode::AUXILIARY_VARIABLE_SEPARATOR + literalCode::AUXILIARY_COUNTER_TAG +
					literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(process),
					_name, AuxiliaryType::COUNTER, process
				))));
			_auxiliaryFirstPointerNamesOfGlobal.insert(pair<int, string>(process, config::forceRegisterSymbol(new VariableEntry(
					_name + literalCode::AUXILIARY_VARIABLE_SEPARATOR + literalCode::AUXILIARY_FIRST_POINTER_TAG +
					literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(process),
					_name, AuxiliaryType::FIRST_POINTER, process
				))));
			_auxiliaryBufferNamesOfGlobal.insert(pair<int, vector<string>>(process, vector<string>()));

			for (int ctr = 1; ctr <= config::K; ctr++)
			{
				_auxiliaryBufferNamesOfGlobal[process].push_back(config::forceRegisterSymbol(new VariableEntry(
						_name + literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(ctr) +
						literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(process),
						_name, AuxiliaryType::BUFFER, process
					)));
			}

			sort(_auxiliaryBufferNamesOfGlobal[process].begin(), _auxiliaryBufferNamesOfGlobal[process].end());
		}
	}