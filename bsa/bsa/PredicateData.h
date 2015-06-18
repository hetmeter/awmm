#pragma once

#include <vector>
#include <string>

class Ast;

class PredicateData
{
public:

/* Constructor and destructor */
	PredicateData(Ast* predicateAst);
	~PredicateData();

/* Public fields */
	const std::string getPredicateCode();
	Ast* getPredicateAst();
	const std::string getSingleTemporaryVariableName();
	const std::string getSingleBooleanVariableName();
	Ast* getMultipleTemporaryVariableAst();
	Ast* getMultipleBooleanVariableAst();

private:

/* Locals */
	std::string _predicateCode;
	Ast* _predicateAst;
	std::string _singleTemporaryVariableName;
	std::string _singleBooleanVariableName;
	Ast* _multipleTemporaryVariableAst = nullptr;
	Ast* _multipleBooleanVariableAst = nullptr;

	bool _extendedPredicatesInitialized = false;
	std::vector<PredicateData*> _extendedPredicates;

/* Private methods */
	std::vector<PredicateData*> getExtendedPredicates();
};