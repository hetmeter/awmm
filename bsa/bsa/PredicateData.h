#pragma once

#include <vector>
#include <set>
#include <string>
#include <map>

class Ast;

class PredicateData
{
public:

/* Constructor and destructor */

	PredicateData(Ast* predicateAst);
	~PredicateData();

/* Public fields */

	const std::string getSingleTemporaryVariableName();
	const std::string getSingleBooleanVariableName();
	Ast* getPredicateAst();
	Ast* getTemporaryRHS(bool value);
	Ast* getBooleanRHS(bool value);
	std::set<std::string> getPredicateIDs();
	std::vector<PredicateData*> getAllReplacementVariants(int process);

	const std::string getXReplacedByYCode(const std::string x, const std::string y);

private:

/* Locals */

	std::string _predicateCode;
	std::string _singleTemporaryVariableName;
	std::string _singleBooleanVariableName;

	Ast* _predicateAst;

	std::map<bool, Ast*> _temporaryRHS;
	std::map<bool, Ast*> _booleanRHS;

	std::set<std::string> _predicateIDs;
	std::map<std::pair<std::string, std::string>, std::string> _replacements;
	std::map<int, std::vector<PredicateData*>> _allReplacementVariants;

	bool _predicateIDsAreInitialized = false;
	bool _isBuffer;

/* Private methods */

	void initializePredicateIDs();

	const std::string getPredicateCode();
};