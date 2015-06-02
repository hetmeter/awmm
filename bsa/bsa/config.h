#pragma once

#include <string>
#include <vector>
#include <time.h>
#include <iostream>
#include <map>
#include <regex>
#include <z3++.h>

class Ast;
class GlobalVariable;
class CubeTreeNode;

namespace config
{
/* Global parameters */
	extern int K;
	extern int globalCubeSizeLimit;
	extern int globalPredicatesCount;
	extern std::vector<Ast*> globalPredicates;
	
	enum language { PSO, TSO, RMA };
	extern language currentLanguage;

/* Global variables */
	extern std::map<Ast*, std::vector<Ast*>> lazyReplacements;
	extern std::vector<std::string> variableNames;
	extern std::vector<std::string> auxiliaryBooleanVariableNames;
	extern std::vector<std::string> auxiliaryTemporaryVariableNames;
	extern std::map<std::string, GlobalVariable*> globalVariables;
	extern int currentAuxiliaryLabel;
	extern std::map<std::string, Ast*> labelLookupMap;
	extern std::map<int, std::vector<int>> predicateVariableTransitiveClosures;
	extern bool falseImplicativeCubesIsInitialized;
	extern Ast* assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes;
	extern Ast* falsePredicate;
	//extern CubeTreeNode* implicativeCubes;
	extern CubeTreeNode* implicativeCubes;

/* Global variable handling */
	extern void carryOutLazyReplacements();
	extern int getCurrentAuxiliaryLabel();
	extern int indexOf(const std::string &varName);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(Ast* predicate);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(const std::vector<Ast*> &parallelAssignments);
	extern std::vector<int> getRelevantAuxiliaryBooleanVariableIndices(const std::string &variableName);
	extern std::vector<int> getPredicateVariableTransitiveClosure(int index);
	extern Ast* getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes();
	extern Ast* getFalsePredicate();
	extern CubeTreeNode* getImplicativeCubes();

/* String operations */
	extern std::string addTabs(const std::string &s, int numberOfTabs);

/* Vector operations */
	extern bool stringVectorContains(const std::vector<std::string> &container, const std::string &element);
	extern std::vector<int> intVectorUnion(const std::vector<int> &first, const std::vector<int> &second);
	extern bool intVectorVectorContains(const std::vector<std::vector<int>> &container, const std::vector<int> &element);
	extern std::vector<std::vector<int>> intSetCartesianProduct(const std::vector<int> &first, const std::vector<int> &second);
	extern std::vector<std::vector<int>> intSetCartesianProduct(const std::vector<std::vector<int>> &first, const std::vector<int> &second);

/* Initializations */
	extern void initializeAuxiliaryVariables();

/* Messages */
	const extern std::string OVERFLOW_MESSAGE;

/* Error handling */
	extern void throwError(const std::string &msg);
	extern void throwCriticalError(const std::string &msg);

/* Z3 */
	extern bool isTautology(Ast* predicate);
	extern bool cubeImpliesPredicate(const std::vector<Ast*> &cube, Ast* predicate);
	extern bool expressionImpliesPredicate(z3::expr expression, Ast* predicate);
	extern z3::expr impliesDuplicate(z3::expr const &a, z3::expr const &b);
}