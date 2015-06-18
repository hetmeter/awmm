#pragma once

#include <string>
#include <vector>
#include <time.h>
#include <iostream>
#include <map>
#include <set>
#include <regex>
#include <z3++.h>

class Ast;
class GlobalVariable;
class VariableEntry;
class CubeTreeNode;
class PredicateData;

namespace config
{
/* Global parameters */
	extern int K;
	extern int globalCubeSizeLimit;
	extern int globalPredicatesCount;
	//extern std::vector<Ast*> globalPredicates;
	extern std::vector<PredicateData*> globalPredicates;
	extern bool generateAuxiliaryPredicates;
	
	enum language { PSO, TSO, RMA };
	extern language currentLanguage;

/* Global variables */
	extern std::map<std::string, VariableEntry*> symbolMap;
	extern std::map<int, std::set<int>> labelMap;
	extern std::vector<int> processes;
	extern std::map<std::pair<std::string, std::string>, Ast*> weakestLiberalPreconditions;

	extern std::map<Ast*, std::vector<Ast*>> lazyReplacements;
	/*extern std::vector<std::string> variableNames;*/
	/*extern std::vector<std::string> auxiliaryBooleanVariableNames;
	extern std::vector<std::string> auxiliaryTemporaryVariableNames;*/
	//extern std::map<std::string, GlobalVariable*> globalVariables;
	extern int currentAuxiliaryLabel;
	extern std::map<std::string, Ast*> labelLookupMap;
	extern std::map<int, std::vector<int>> predicateVariableTransitiveClosures;
	extern bool falseImplicativeCubesIsInitialized;
	extern Ast* assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes;
	extern Ast* falsePredicate;
	extern std::string emptyCubeRepresentation;
	extern std::map<std::string, CubeTreeNode*> implicativeCubes;
	extern std::vector<std::string> allFalseImplyingCubes;
	//extern std::map<std::string, std::pair<Ast*, Ast*>> predicateAstRepresentations;

/* Global variable handling */
	extern PredicateData* getPredicateData(Ast* predicateAst);

	extern bool tryRegisterGlobalSymbol(const std::string &name);
	extern bool tryRegisterLocalSymbol(const std::string &name);
	extern bool tryRegisterAuxiliarySymbol(const std::string &name, const std::string &globalName);
	extern void generateAllAuxiliarySymbols();
	extern const std::string forceRegisterSymbol(VariableEntry* desiredSymbol);

	extern bool tryRegisterLabel(int process, int label);

	extern bool tryRegisterProcess(int process);

	extern void carryOutLazyReplacements();
	extern int getCurrentAuxiliaryLabel();
	extern int indexOf(const std::string &varName);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(Ast* predicate);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(const std::vector<Ast*> &parallelAssignments);
	extern std::vector<int> getRelevantAuxiliaryBooleanVariableIndices(const std::string &variableName);
	extern std::vector<int> getPredicateVariableTransitiveClosure(int index);
	extern Ast* getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes();
	extern Ast* getFalsePredicate();
	extern std::string getEmptyCubeRepresentation();
	extern std::vector<std::string> getMinimalImplyingCubes(Ast* predicate, const std::vector<int> &relevantIndices);
	extern std::vector<std::string> getAllFalseImplyingCubes();
	extern CubeTreeNode* getImplicativeCube(const std::string &stringRepresentation);
	/*extern std::pair<Ast*, Ast*> getPredicateAstRepresentationPair(Ast* predicate);
	extern Ast* getPredicateTemporaryAstRepresentation(Ast* predicate);
	extern Ast* getPredicateBooleanAstRepresentation(Ast* predicate);*/
	/*extern void initializeImplicativeCubes();
	extern void registerImplicativeCube(CubeTreeNode* cube);
	extern std::vector<int> getRelevantCubeIndices(const std::vector<int> &relevantIndices);
	extern void reportImplication(const std::string &representation, Ast* predicate);*/

/* String operations */
	extern std::string addTabs(const std::string &s, int numberOfTabs);

/* Vector operations */
	extern bool stringVectorContains(const std::vector<std::string> &container, const std::string &element);
	extern std::vector<int> intVectorUnion(const std::vector<int> &first, const std::vector<int> &second);
	extern bool intVectorVectorContains(const std::vector<std::vector<int>> &container, const std::vector<int> &element);
	extern std::vector<std::vector<int>> intSetCartesianProduct(const std::vector<int> &first, const std::vector<int> &second);
	extern std::vector<std::vector<int>> intSetCartesianProduct(const std::vector<std::vector<int>> &first, const std::vector<int> &second);

/* Ast operations */
	extern void prepareNodeForLazyReplacement(Ast* replacement, Ast* replacedNode);
	extern void prepareNodeForLazyReplacement(const std::vector<Ast*> &replacement, Ast* replacedNode);
	extern void replaceNode(Ast* replacement, Ast* replacedNode);
	extern void replaceNode(const std::vector<Ast*> &replacement, Ast* replacedNode);
	extern Ast* getWeakestLiberalPrecondition(Ast* assignment, Ast* predicate);

/* Initializations */
	extern void initializeAuxiliaryVariables();
	extern void initializeAuxiliaryPredicates();

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