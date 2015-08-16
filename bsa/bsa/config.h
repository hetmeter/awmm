#pragma once

#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <time.h>
#include <vector>
#include <z3++.h>

class Ast;
class CubeTreeNode;
class PredicateData;
class MergeableSetContainer;
class VariableEntry;

namespace config
{
/* Global parameters */
	extern int K;
	extern int globalCubeSizeLimit;
	extern int originalPredicatesCount;
	extern int totalPredicatesCount;
	extern std::vector<std::string> originalPredicateCodes;
	extern std::map<std::string, PredicateData*> predicates;
	extern bool generateAuxiliaryPredicates;
	extern bool evaluationMode;
	extern bool verboseMode;
	extern int negatedLargestFalseImplicativeDisjunctionSizeThreshold;

	enum language { SALPL, RMA };
	extern language currentLanguage;

	enum ordering { PSO, TSO };
	extern ordering memoryOrdering;

/* Global variables */
	extern std::map<std::string, VariableEntry*> symbolMap;
	extern std::map<int, std::set<int>> labelMap;
	extern std::vector<int> processes;
	extern std::map<std::pair<std::string, std::string>, Ast*> weakestLiberalPreconditions;
	extern std::map<std::pair<std::pair<int, bool>, std::string>, Ast*> largestImplicativCubeDisjunctions;

	extern std::map<Ast*, std::vector<Ast*>> lazyReplacements;
	extern int currentAuxiliaryLabel;
	extern std::map<std::pair<int, int>, Ast*> labelLookupMap;
	extern Ast* assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes;
	extern Ast* falsePredicate;
	extern std::map<std::string, CubeTreeNode*> implicativeCubes;
	extern std::vector<std::string> allFalseImplyingCubeStringRepresentations;
	extern MergeableSetContainer* variableTransitiveClosures;

/* Global variable handling */

	extern PredicateData* getPredicateDataByCode(const std::string &code);

	extern bool tryRegisterGlobalSymbol(const std::string &name);
	extern bool tryRegisterLocalSymbol(const std::string &name);
	extern void generateAllAuxiliarySymbols();
	extern const std::string forceRegisterSymbol(VariableEntry* desiredSymbol);

	extern bool tryRegisterLabel(int process, int label);

	extern bool tryRegisterProcess(int process);

	extern void carryOutLazyReplacements();
	extern int getCurrentAuxiliaryLabel();
	extern std::vector<int> getVariableTransitiveClosureFromOriginalPredicates(std::set<std::string> variables);
	extern std::vector<std::string> getOriginalPredicateCodesContainingSymbol(const std::string &symbol);
	extern Ast* getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes();
	extern Ast* getFalsePredicate();
	extern std::string getEmptyCubeRepresentation();
	extern std::vector<std::string> getMinimalImplyingCubeStringRepresentations(Ast* predicate, const std::vector<int> &relevantIndices);
	extern std::vector<std::string> getAllFalseImplyingCubeStringRepresentations();
	extern CubeTreeNode* getImplicativeCube(const std::string &stringRepresentation);

/* String operations */
	extern std::string addTabs(const std::string &s, int numberOfTabs);

/* Vector operations */
	extern bool stringVectorContains(const std::vector<std::string> &container, const std::string &element);

/* Ast operations */
	extern void prepareNodeForLazyReplacement(const std::vector<Ast*> &replacement, Ast* replacedNode);
	extern void replaceNode(Ast* replacement, Ast* replacedNode);
	extern void replaceNode(const std::vector<Ast*> &replacement, Ast* replacedNode);
	extern Ast* getWeakestLiberalPrecondition(Ast* assignment, Ast* predicate);
	extern Ast* getLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables = true);

/* Initializations */
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