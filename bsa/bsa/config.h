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
	extern CubeTreeNode* falseImplicativeCubes;
	extern Ast* assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes;

/* Global variable handling */
	extern void carryOutLazyReplacements();
	extern int getCurrentAuxiliaryLabel();
	extern int indexOf(const std::string &varName);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(Ast* predicate);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(const std::vector<Ast*> &parallelAssignments);
	extern std::vector<int> getRelevantAuxiliaryBooleanVariableIndices(const std::string &variableName);
	extern std::vector<int> getPredicateVariableTransitiveClosure(int index);
	extern bool impliesFalse(const std::string &cubeRepresentation);
	extern CubeTreeNode* getFalseImplicativeCubes();
	extern Ast* getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes();

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
	extern bool cubeImpliesPredicate(const std::vector<Ast*> &cube, Ast* predicate);
	extern bool expressionImpliesPredicate(z3::expr expression, Ast* predicate);
	extern z3::expr impliesDuplicate(z3::expr const &a, z3::expr const &b);
}


//	const extern std::string TOP_STRING;
//	const extern std::string BOTTOM_STRING;
//
//	extern int indexOf(Ast* predicate);
//
//	extern std::string replicateString(std::string s, int n);
//
//	extern bool stringVectorIsSubset(std::vector<std::string> possibleSubset, std::vector<std::string> possibleSuperset);
//
//
//	extern Ast* stringToAst(std::string parsedProgramString);
//
//	enum booleanOperator { BOP_EQUALS, BOP_LESS_THAN, BOP_LESS_EQUALS, BOP_GREATER_THAN, BOP_GREATER_EQUALS, BOP_NOT_EQUALS, BOP_NOT, BOP_AND, BOP_OR, BOP_INVALID };
//	extern booleanOperator stringToBooleanOperator(std::string operatorString);
//	extern std::string booleanOperatorToString(booleanOperator boolOp);
//
//	extern std::vector<std::vector<Ast*>> allSubsetsOfLengthK(std::vector<Ast*> superset, int K);
//	extern std::vector<std::vector<Ast*>> powerSetOfLimitedCardinality(std::vector<Ast*> superset, int cardinalityLimit);
//	extern std::string nextBinaryRepresentation(std::string currentBinaryRepresentation, int length);
//
//	const extern char CUBE_STATE_UNDECIDED;
//	const extern char CUBE_STATE_MAY_BE_FALSE;
//	const extern char CUBE_STATE_MAY_BE_TRUE;