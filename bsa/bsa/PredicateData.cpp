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
			literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(config::globalPredicatesCount), VariableEntry::LOCAL));
		_singleBooleanVariableName = config::forceRegisterSymbol(new VariableEntry(string(1, literalCode::BOOLEAN_VARIABLE_PREFIX) +
			literalCode::AUXILIARY_VARIABLE_SEPARATOR + to_string(config::globalPredicatesCount), VariableEntry::LOCAL));
	}

	PredicateData::~PredicateData()
	{
		delete _predicateAst;
		delete _multipleBooleanVariableAst;
		delete _multipleTemporaryVariableAst;
	}

/* Public fields */
	const string PredicateData::getPredicateCode()
	{
		return _predicateCode;
	}

	Ast* PredicateData::getPredicateAst()
	{
		return _predicateAst->clone();
	}

	const string PredicateData::getSingleTemporaryVariableName()
	{
		return _singleTemporaryVariableName;
	}

	const string PredicateData::getSingleBooleanVariableName()
	{
		return _singleBooleanVariableName;
	}

	Ast* PredicateData::getMultipleTemporaryVariableAst()
	{
		if (_multipleTemporaryVariableAst == nullptr)
		{
			vector<Ast*> terms;
			vector<PredicateData*> extendedPredicates = getExtendedPredicates();

			terms.push_back(Ast::newID(_singleTemporaryVariableName));

			for (PredicateData* extendedPredicate : extendedPredicates)
			{
				terms.push_back(Ast::newID(extendedPredicate->getSingleTemporaryVariableName()));
			}

			if (terms.size() > 1)
			{
				_multipleTemporaryVariableAst = Ast::newMultipleOperation(terms, literalCode::DOUBLE_AND);
			}
			else
			{
				_multipleTemporaryVariableAst = terms[0];
			}
		}

		return _multipleTemporaryVariableAst;
	}

	Ast* PredicateData::getMultipleBooleanVariableAst()
	{
		if (_multipleBooleanVariableAst == nullptr)
		{
			vector<Ast*> terms;
			vector<PredicateData*> extendedPredicates = getExtendedPredicates();

			terms.push_back(Ast::newID(_singleBooleanVariableName));

			for (PredicateData* extendedPredicate : extendedPredicates)
			{
				terms.push_back(Ast::newID(extendedPredicate->getSingleBooleanVariableName()));
			}

			if (terms.size() > 1)
			{
				_multipleBooleanVariableAst = Ast::newMultipleOperation(terms, literalCode::DOUBLE_AND);
			}
			else
			{
				_multipleBooleanVariableAst = terms[0];
			}
		}

		return _multipleBooleanVariableAst;
	}

/* Private methods */
	vector<PredicateData*> PredicateData::getExtendedPredicates()
	{
		if (!_extendedPredicatesInitialized)
		{
			vector<string> IDs = _predicateAst->getIDs();
			bool containsAuxiliaries = false;

			for (string ID : IDs)
			{
				if (config::symbolMap[ID]->getType() == VariableEntry::AUXILIARY)
				{
					containsAuxiliaries = true;
					break;
				}
			}

			if (!containsAuxiliaries)
			{
				VariableEntry* currentID;
				vector<string> currentAuxiliaryBufferNames;
				Ast* currentTerm;

				for (string ID : IDs)
				{
					currentID = config::symbolMap[ID];

					for (int process : config::processes)
					{
						currentAuxiliaryBufferNames = currentID->getAuxiliaryBufferNames(process);

						for (string auxiliaryBufferName : currentAuxiliaryBufferNames)
						{
							currentTerm = _predicateAst->clone();
							currentTerm->topDownCascadingReplaceIDNames(ID, auxiliaryBufferName);
							_extendedPredicates.push_back(config::getPredicateData(currentTerm));
						}
					}
				}
			}

			_extendedPredicatesInitialized = true;
		}

		return _extendedPredicates;
	}