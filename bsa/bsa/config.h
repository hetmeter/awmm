#pragma once

#include <string>
#include <iostream>
#include <map>
#include <regex>
#include <bitset>
#include <z3++.h>
#include <algorithm>

class Ast;
class GlobalVariable;

namespace config
{
	const extern char LEFT_PARENTHESIS;
	const extern char RIGHT_PARENTHESIS;
	const extern char QUOTATION;
	const extern char COMMA;
	const extern char SEMICOLON;
	const extern char COLON;
	const extern char ASSIGN_OPERATOR;
	const extern char SPACE;
	const extern char TAB;
	const extern char LABEL_SEPARATOR;
	const extern char AUXILIARY_VARIABLE_SEPARATOR;
	const extern char PLUS;
	const extern char GREATER_THAN;

	const extern std::vector<std::string> UNARY_OPERATORS;
	const extern std::vector<std::string> BINARY_OPERATORS;
	const extern std::string EQUALS;
	const extern std::string LESS_THAN;
	const extern std::string LESS_EQUALS;
	const extern std::string GREATER_EQUALS;
	const extern std::string NOT_EQUALS;
	const extern std::string NOT;
	const extern std::string AND;
	const extern std::string OR;
	const extern std::string DOUBLE_AND;
	const extern std::string DOUBLE_OR;
	const extern std::string COMMENT_PREFIX;
	const extern std::string MULTILINE_COMMENT_PREFIX;
	const extern std::string MULTILINE_COMMENT_SUFFIX;
	const extern std::string REPLACING_CAPTION;
	const extern std::string FINISHED_REPLACING_CAPTION;
	const extern std::string OVERFLOW_MESSAGE;

	const extern std::string ID_TOKEN_NAME;
	const extern std::string INT_TOKEN_NAME;
	const extern std::string BOOL_TOKEN_NAME;
	const extern std::string PROGRAM_DECLARATION_TOKEN_NAME;
	const extern std::string BL_PROGRAM_DECLARATION_TOKEN_NAME;
	const extern std::string PROCESS_DECLARATION_TOKEN_NAME;
	const extern std::string BL_PROCESS_DECLARATION_TOKEN_NAME;
	const extern std::string STATEMENTS_TOKEN_NAME;
	const extern std::string FENCE_TOKEN_NAME;
	const extern std::string LABEL_TOKEN_NAME;
	const extern std::string GOTO_TOKEN_NAME;
	const extern std::string IF_ELSE_TOKEN_NAME;
	const extern std::string WHILE_TOKEN_NAME;
	const extern std::string NONE_TOKEN_NAME;
	const extern std::string NOP_TOKEN_NAME;
	const extern std::string ABORT_TOKEN_NAME;
	const extern std::string FLUSH_TOKEN_NAME;
	const extern std::string PROCESS_HEADER_TOKEN_NAME;
	const extern std::string ASTERISK_TOKEN_NAME;
	const extern std::string INITIALIZATION_BLOCK_TOKEN_NAME;
	const extern std::string BL_INITIALIZATION_BLOCK_TOKEN_NAME;
	const extern std::string BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME;
	const extern std::string BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME;
	const extern std::string LOCAL_ASSIGN_TOKEN_NAME;
	const extern std::string ASSUME_TOKEN_NAME;
	const extern std::string BEGIN_ATOMIC_TOKEN_NAME;
	const extern std::string END_ATOMIC_TOKEN_NAME;
	const extern std::string CHOOSE_TOKEN_NAME;

	const extern std::string BEGINIT_TAG_NAME;
	const extern std::string ENDINIT_TAG_NAME;
	const extern std::string PROCESS_TAG_NAME;
	const extern std::string INIT_TAG_NAME;
	const extern std::string IF_TAG_NAME;
	const extern std::string ELSE_TAG_NAME;
	const extern std::string ENDIF_TAG_NAME;
	const extern std::string TRUE_TAG_NAME;
	const extern std::string FALSE_TAG_NAME;

	const extern std::string AUXILIARY_COUNTER_TAG;
	const extern std::string AUXILIARY_FIRST_POINTER_TAG;

	const extern std::string PSO_TSO_STORE_TOKEN_NAME;
	const extern std::string PSO_TSO_LOAD_TOKEN_NAME;

	const extern std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME;
	const extern std::string RMA_PUT_TOKEN_NAME;
	const extern std::string RMA_GET_TOKEN_NAME;

	const extern std::string ACCEPTING_STATE_REGEX;
	const extern std::string EXTENSION_REGEX;
	
	const extern int TOP_VALUE;
	const extern int BOTTOM_VALUE;
	const extern int UNDEFINED_VALUE;
	const extern std::string TOP_STRING;
	const extern std::string BOTTOM_STRING;

	const extern std::string PSO_EXTENSION;
	const extern std::string TSO_EXTENSION;
	const extern std::string RMA_EXTENSION;
	const extern std::string BOOLEAN_EXTENSION;
	const extern std::string PREDICATE_EXTENSION;
	const extern std::string BSA_EXTENSION;
	const extern std::string OUT_EXTENSION;

	const extern std::string PREDICATE_ABSTRACTION_COMMENT_PREFIX;

	enum language { PSO, TSO, RMA };

	extern language currentLanguage;
	extern std::map<std::string, Ast*> labelLookupMap;
	extern std::vector<std::string> variableNames;
	extern std::map<std::string, GlobalVariable*> globalVariables;
	extern std::vector<Ast*> globalPredicates;
	extern int indexOf(Ast* predicate);
	extern int indexOf(std::string varName);
	extern std::vector<std::string> auxiliaryBooleanVariableNames;
	extern std::vector<std::string> auxiliaryTemporaryVariableNames;
	extern std::map<int, std::vector<int>> predicateVariableTransitiveClosures;
	extern std::vector<int> getPredicateVariableTransitiveClosure(int index);
	extern int currentAuxiliaryLabel;
	extern int getCurrentAuxiliaryLabel();
	extern int K;
	extern int globalCubeSizeLimit;
	extern std::map<Ast*, std::vector<Ast*>> lazyReplacements;

	extern void carryOutLazyReplacements();
	extern void initializeAuxiliaryVariables();
	extern std::string replicateString(std::string s, int n);
	extern std::string addTabs(std::string s, int numberOfTabs);
	extern void throwError(std::string msg);
	extern void throwCriticalError(std::string msg);

	extern bool stringVectorContains(std::vector<std::string> container, std::string element);
	extern bool stringVectorIsSubset(std::vector<std::string> possibleSubset, std::vector<std::string> possibleSuperset);

	extern std::vector<int> intVectorUnion(std::vector<int> first, std::vector<int> second);
	extern std::vector<std::vector<int>> intSetCartesianProduct(std::vector<int> first, std::vector<int> second);
	extern std::vector<std::vector<int>> intSetCartesianProduct(std::vector<std::vector<int>> first, std::vector<int> second);
	extern bool intVectorVectorContains(std::vector<std::vector<int>> container, std::vector<int> element);

	extern Ast* stringToAst(std::string parsedProgramString);

	enum booleanOperator { BOP_EQUALS, BOP_LESS_THAN, BOP_LESS_EQUALS, BOP_GREATER_THAN, BOP_GREATER_EQUALS, BOP_NOT_EQUALS, BOP_NOT, BOP_AND, BOP_OR, BOP_INVALID };
	extern booleanOperator stringToBooleanOperator(std::string operatorString);
	extern std::string booleanOperatorToString(booleanOperator boolOp);

	extern std::vector<std::vector<Ast*>> allSubsetsOfLengthK(std::vector<Ast*> superset, int K);
	extern std::vector<std::vector<Ast*>> powerSetOfLimitedCardinality(std::vector<Ast*> superset, int cardinalityLimit);
	extern std::string nextBinaryRepresentation(std::string currentBinaryRepresentation, int length);
	extern bool cubeImpliesPredicate(std::vector<Ast*> cube, Ast* predicate);
	extern bool expressionImpliesPredicate(z3::expr expression, Ast* predicate);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(Ast* predicate);
	extern std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(std::vector<Ast*> parallelAssignments);
	extern std::vector<int> getRelevantAuxiliaryBooleanVariableIndices(std::string variableName);

	const extern char CUBE_STATE_OMIT;
	const extern char CUBE_STATE_IGNORE;
	const extern char CUBE_STATE_UNDECIDED;
	const extern char CUBE_STATE_MAY_BE_FALSE;
	const extern char CUBE_STATE_MAY_BE_TRUE;
	const extern char CUBE_STATE_DECIDED_FALSE;
	const extern char CUBE_STATE_DECIDED_TRUE;
	extern std::string getCubeStatePool(std::vector<int> predicateIndices);
	extern std::string getCubeStatePool(int predicateIndex);
	extern std::vector<std::string> getNaryCubeStateCombinations(std::vector<int> predicateIndices, int n);
	extern std::vector<Ast*> cubeStringRepresentationToAstRef(std::string pool);
	extern std::vector<std::string> getImplicativeCubeStates(std::string pool, Ast* predicate);
	extern std::string removeDecisionsFromPool(std::string pool, std::vector<std::string> decisions);
	extern std::string applyDecisionMask(std::string pool, std::string decisionMask);
	extern bool isSubset(std::string possibleSubset, std::string possibleSuperset);

	extern z3::expr impliesDuplicate(z3::expr const &a, z3::expr const &b);
}