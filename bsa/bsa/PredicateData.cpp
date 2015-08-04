#include "PredicateData.h"
#include "Ast.h"
#include "config.h"
#include "literalCode.h"
#include "VariableEntry.h"

using namespace std;

/* Constructor and destructor */

	PredicateData::PredicateData(Ast* predicateAst)
	{
		_predicateCode = predicateAst->getCode();
		_predicateAst = predicateAst->clone();
		_singleTemporaryVariableName = config::forceRegisterSymbol(new VariableEntry(string(1, literalCode::TEMPORARY_VARIABLE_PREFIX) +
			literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(config::totalPredicatesCount), VariableEntry::LOCAL, this));
		_singleBooleanVariableName = config::forceRegisterSymbol(new VariableEntry(string(1, literalCode::BOOLEAN_VARIABLE_PREFIX) +
			literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(config::totalPredicatesCount), VariableEntry::LOCAL, this));
		_isBuffer = false;

		vector<string> LHSvariables = _predicateAst->getChild(0)->getIDs();

		if (LHSvariables.size() == 1)
		{
			VariableEntry* LHSentry = config::symbolMap[LHSvariables[0]];
		}

		vector<string> variablesInvolved = _predicateAst->getIDs();

		for (string variableInvolved : variablesInvolved)
		{
			if (config::symbolMap[variableInvolved]->getType() == VariableEntry::AUXILIARY &&
				config::symbolMap[variableInvolved]->getAuxiliaryType() == VariableEntry::BUFFER)
			{
				_isBuffer = true;
				break;
			}
		}
	}

	PredicateData::~PredicateData()
	{
		delete _predicateAst;

		if (_temporaryRHS.find(true) == _temporaryRHS.end())
		{
			delete _temporaryRHS[true];
		}

		if (_temporaryRHS.find(false) == _temporaryRHS.end())
		{
			delete _temporaryRHS[false];
		}

		if (_booleanRHS.find(true) == _booleanRHS.end())
		{
			delete _booleanRHS[true];
		}

		if (_booleanRHS.find(false) == _booleanRHS.end())
		{
			delete _booleanRHS[false];
		}

		_temporaryRHS.clear();
		_booleanRHS.clear();
	}

/* Public fields */

	const string PredicateData::getSingleTemporaryVariableName()
	{
		return _singleTemporaryVariableName;
	}

	const string PredicateData::getSingleBooleanVariableName()
	{
		return _singleBooleanVariableName;
	}

	Ast* PredicateData::getPredicateAst()
	{
		return _predicateAst->clone();
	}

	Ast* PredicateData::getTemporaryRHS(bool value)
	{
		if (_temporaryRHS.find(value) == _temporaryRHS.end())
		{
			string operation = value ? literalCode::NOT_EQUALS : literalCode::EQUALS;
			_temporaryRHS[value] = Ast::newBinaryOp(Ast::newID(_singleTemporaryVariableName), Ast::newINT(0), operation);
		}

		return _temporaryRHS[value]->clone();
	}

	Ast* PredicateData::getBooleanRHS(bool value)
	{
		if (_booleanRHS.find(value) == _booleanRHS.end())
		{
			string operation = value ? literalCode::NOT_EQUALS : literalCode::EQUALS;
			_booleanRHS[value] = Ast::newBinaryOp(Ast::newID(_singleBooleanVariableName), Ast::newINT(0), operation);
		}

		return _booleanRHS[value]->clone();
	}

	set<string> PredicateData::getPredicateIDs()
	{
		if (!_predicateIDsAreInitialized)
		{
			initializePredicateIDs();
		}

		return _predicateIDs;
	}
	
	vector<PredicateData*> PredicateData::getAllReplacementVariants(int process)
	{
		if (_allReplacementVariants.find(process) == _allReplacementVariants.end())
		{
			if (!_predicateIDsAreInitialized)
			{
				initializePredicateIDs();
			}

			vector<PredicateData*> result;
			result.push_back(this);
			vector<PredicateData*> subResult;
			VariableEntry* currentVariable;
			vector<string> currentBufferVariableNames;
			int currentBufferSize;
			string currentPredicateCode;
			Ast* replacementPredicateAst;

			for (string ID : _predicateIDs)
			{
				if (config::symbolMap[ID]->getType() == VariableEntry::GLOBAL)
				{
					subResult.clear();
					currentVariable = config::symbolMap[ID];
					currentBufferVariableNames = currentVariable->getAuxiliaryBufferNames(process);
					currentBufferSize = currentVariable->getMaximumBufferSize(process);

					for (PredicateData* resultPredicate : result)
					{
						for (int ctr = 0; ctr < currentBufferSize; ++ctr)
						{
							currentPredicateCode = resultPredicate->getXReplacedByYCode(ID, currentBufferVariableNames[ctr]);

							if (config::predicates.find(currentPredicateCode) == config::predicates.end())
							{
								replacementPredicateAst = getPredicateAst();
								replacementPredicateAst->topDownCascadingReplaceIDNames(ID, currentBufferVariableNames[ctr]);
								config::predicates[currentPredicateCode] = new PredicateData(replacementPredicateAst);
								++(config::totalPredicatesCount);
							}

							subResult.push_back(config::predicates[currentPredicateCode]);
						}
					}

					result.insert(result.end(), subResult.begin(), subResult.end());
				}
			}

			result.erase(result.begin());

			_allReplacementVariants[process] = result;
		}

		return _allReplacementVariants[process];
	}

	const string PredicateData::getXReplacedByYCode(const std::string x, const std::string y)
	{
		if (!_predicateIDsAreInitialized)
		{
			initializePredicateIDs();
		}

		if (_predicateIDs.find(x) == _predicateIDs.end())
		{
			return getPredicateCode();
		}

		pair<string, string> key = pair<string, string>(x, y);

		if (_replacements.find(key) == _replacements.end())
		{
			Ast* replacementPredicateAst = getPredicateAst();
			replacementPredicateAst->topDownCascadingReplaceIDNames(x, y);
			_replacements[key] = replacementPredicateAst->getCode();
		}

		return _replacements[key];
	}

/* Private methods */

	void PredicateData::initializePredicateIDs()
	{
		vector<string> IDs = _predicateAst->getIDs();

		for (std::string ID : IDs)
		{
			_predicateIDs.insert(ID);
		}

		_predicateIDsAreInitialized = true;
	}

	const string PredicateData::getPredicateCode()
	{
		return _predicateCode;
	}