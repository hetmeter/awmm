#include "VariableEntry.h"

#include "config.h"
#include "literalCode.h"
#include "bufferSizeMap.h"

using namespace std;

/* Constructor and destructor */
	VariableEntry::VariableEntry(const std::string &name, VariableType type)
	{
		assert(type != AUXILIARY);

		_name = name;
		_type = type;
	}

	VariableEntry::VariableEntry(const std::string &name, const std::string &globalNameOfAuxiliary)
	{
		_name = name;
		_type = AUXILIARY;
		_globalNameOfAuxiliary = globalNameOfAuxiliary;
	}

	VariableEntry::~VariableEntry()
	{
	}

/* Public fields */
	const std::string VariableEntry::getName()
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

	const std::vector<std::string> VariableEntry::getAuxiliaryBufferNames(int process)
	{
		assert(_type == GLOBAL);

		return _auxiliaryBufferNamesOfGlobal[process];
	}

	const std::string VariableEntry::getAuxiliaryCounterName(int process)
	{
		assert(_type == GLOBAL);

		return _auxiliaryCounterNamesOfGlobal[process];
	}

	const std::string VariableEntry::getAuxiliaryFirstPointerName(int process)
	{
		assert(_type == GLOBAL);

		return _auxiliaryFirstPointerNamesOfGlobal[process];
	}

	const std::string VariableEntry::getGlobalName()
	{
		assert(_type == AUXILIARY);

		return _globalNameOfAuxiliary;
	}

	int VariableEntry::getMaximumBufferSize(int process)
	{
		if (_maximumBufferSizePerProcess.find(process) == _maximumBufferSizePerProcess.end())
		{
			return bsm::BOTTOM_VALUE;
		}

		return _maximumBufferSizePerProcess[process];
	}

	void VariableEntry::setMaximumBufferSize(int process, int value)
	{
		if (value > getMaximumBufferSize(process) && value < config::K && value > 0)
		{
			_maximumBufferSizePerProcess[process] = value;
		}
		else if (value == bsm::TOP_VALUE || value >= config::K)
		{
			_maximumBufferSizePerProcess[process] = config::K;
		}
	}

/* Public methods */
	void VariableEntry::generateAuxiliaryVariables()
	{
		for (int process : config::processes)
		{
			_auxiliaryCounterNamesOfGlobal.insert(pair<int, string>(process, config::forceRegisterSymbol(new VariableEntry(
					_name + literalCode::AUXILIARY_VARIABLE_SEPARATOR + literalCode::AUXILIARY_COUNTER_TAG +
					literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(process),
					_name
				))));
			_auxiliaryFirstPointerNamesOfGlobal.insert(pair<int, string>(process, config::forceRegisterSymbol(new VariableEntry(
					_name + literalCode::AUXILIARY_VARIABLE_SEPARATOR + literalCode::AUXILIARY_FIRST_POINTER_TAG +
					literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(process),
					_name
				))));
			_auxiliaryBufferNamesOfGlobal.insert(pair<int, vector<string>>(process, vector<string>()));

			for (int ctr = 1; ctr <= config::K; ctr++)
			{
				_auxiliaryBufferNamesOfGlobal[process].push_back(config::forceRegisterSymbol(new VariableEntry(
						_name + literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(ctr) +
						literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(process),
						_name
					)));
			}
		}
	}