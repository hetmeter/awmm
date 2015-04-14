/*
Global variables, constants, and methods
*/

#include "config.h"
#include "Ast.h"

namespace config
{
	const char LEFT_PARENTHESIS = '(';
	const char RIGHT_PARENTHESIS = ')';
	const char QUOTATION = '\"';
	const char COMMA = ',';
	const char SEMICOLON = ';';
	const char COLON = ':';
	const char ASSIGN_OPERATOR = '=';
	const char SPACE = ' ';
	const char TAB = '\t';
	const char LABEL_SEPARATOR = '.';
	const char AUXILIARY_VARIABLE_SEPARATOR = '_';
	const char PLUS = '+';
	const char GREATER_THAN = '>';

	const std::vector<std::string> UNARY_OPERATORS = { "!" };
	const std::vector<std::string> BINARY_OPERATORS = { "+", "-", "*", "/", "&", "|", "<", ">", "<=", ">=", "=",
		"==", "!=" };
	const std::string EQUALS = "==";
	const std::string LESS_THAN = "<";
	const std::string LESS_EQUALS = "<=";
	const std::string GREATER_EQUALS = ">=";
	const std::string NOT_EQUALS = "!=";
	const std::string NOT = "!";
	const std::string AND = "&";
	const std::string OR = "|";
	const std::string COMMENT_PREFIX = "//";
	const std::string REPLACING_CAPTION = "Replacing";
	const std::string FINISHED_REPLACING_CAPTION = "Finished replacing";
	const std::string OVERFLOW_MESSAGE = "overflow";

	const std::string ID_TOKEN_NAME = "ID";
	const std::string INT_TOKEN_NAME = "INT";
	const std::string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
	const std::string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
	const std::string STATEMENTS_TOKEN_NAME = "statements";
	const std::string FENCE_TOKEN_NAME = "fence";
	const std::string LABEL_TOKEN_NAME = "label";
	const std::string GOTO_TOKEN_NAME = "goto";
	const std::string IF_ELSE_TOKEN_NAME = "ifElse";
	const std::string WHILE_TOKEN_NAME = "while";
	const std::string NONE_TOKEN_NAME = "none";
	const std::string NOP_TOKEN_NAME = "nop";
	const std::string ABORT_TOKEN_NAME = "abort";
	const std::string FLUSH_TOKEN_NAME = "flush";
	const std::string PROCESS_HEADER_TOKEN_NAME = "processHeader";
	const std::string ASTERISK_TOKEN_NAME = "*";
	const std::string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
	const std::string LOCAL_ASSIGN_TOKEN_NAME = "localAssign";
	const std::string ASSUME_TOKEN_NAME = "assume";
	const std::string BEGIN_ATOMIC_TOKEN_NAME = "begin_atomic";
	const std::string END_ATOMIC_TOKEN_NAME = "end_atomic";
	const std::string CHOOSE_TOKEN_NAME = "choose";

	const std::string BEGINIT_TAG_NAME = "beginit";
	const std::string ENDINIT_TAG_NAME = "endinit";
	const std::string PROCESS_TAG_NAME = "process";
	const std::string IF_TAG_NAME = "if";
	const std::string ELSE_TAG_NAME = "else";
	const std::string ENDIF_TAG_NAME = "endif";

	const std::string AUXILIARY_COUNTER_TAG = "cnt";
	const std::string AUXILIARY_FIRST_POINTER_TAG = "fst";
	
	const std::string PSO_TSO_STORE_TOKEN_NAME = "store";
	const std::string PSO_TSO_LOAD_TOKEN_NAME = "load";
	
	const std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME = "processInitialization";
	const std::string RMA_PUT_TOKEN_NAME = "put";
	const std::string RMA_GET_TOKEN_NAME = "get";

	const std::string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";
	const std::string EXTENSION_REGEX = "(.*)\\.(\\S\\S\\S)\\.out";

	const int TOP_VALUE = -1;
	const int BOTTOM_VALUE = -2;
	const int UNDEFINED_VALUE = -3;
	const std::string TOP_STRING = "T";
	const std::string BOTTOM_STRING = "_";

	const std::string PSO_EXTENSION = "pso";
	const std::string TSO_EXTENSION = "tso";
	const std::string RMA_EXTENSION = "rma";
	const std::string PREDICATE_EXTENSION = "predicate";
	const std::string BSA_EXTENSION = "bsa";
	const std::string OUT_EXTENSION = "out";

	language currentLanguage;
	std::map<std::string, Ast*> labelLookupMap;
	std::vector<std::string> variableNames;
	std::map<std::string, GlobalVariable*> globalVariables;
	std::vector<Ast*> globalPredicates;
	std::vector<std::string> auxiliaryBooleanVariableNames;
	std::vector<std::string> auxiliaryTemporaryVariableNames;
	int currentAuxiliaryLabel = -1;
	int globalCubeSizeLimit;
	int K;
	std::map<Ast*, std::vector<Ast*>> lazyReplacements;

	void carryOutLazyReplacements()
	{
		for (std::map<Ast*, std::vector<Ast*>>::iterator iterator = lazyReplacements.begin(); iterator != lazyReplacements.end(); iterator++)
		{
			Ast::replaceNode(iterator->second, iterator->first);
		}
	}

	void initializeAuxiliaryVariables()
	{
		int globalPredicateCount = globalPredicates.size();
		std::string currentVariableName;

		for (int ctr = 0; ctr < globalPredicateCount; ctr++)
		{
			srand(time(NULL));
			currentVariableName = "b_" + std::to_string(ctr);

			while (stringVectorContains(variableNames, currentVariableName))
			{
				currentVariableName = "b_" + std::to_string(ctr) + "_" + std::to_string(rand());
			}

			auxiliaryBooleanVariableNames[ctr] = currentVariableName;
			variableNames.push_back(currentVariableName);

			currentVariableName = "t_" + std::to_string(ctr);

			while (stringVectorContains(variableNames, currentVariableName))
			{
				currentVariableName = "t_" + std::to_string(ctr) + "_" + std::to_string(rand());
			}

			auxiliaryTemporaryVariableNames[ctr] = currentVariableName;
			variableNames.push_back(currentVariableName);
		}
	}

	int getCurrentAuxiliaryLabel()
	{
		if (currentAuxiliaryLabel == -1)
		{
			int maxLabel = -1;
			int currentLabel;

			for (std::map<std::string, Ast*>::iterator iterator = labelLookupMap.begin(); iterator != labelLookupMap.end(); iterator++)
			{
				currentLabel = std::stoi(iterator->first);
				if (currentLabel > maxLabel)
				{
					maxLabel = currentLabel;
				}
			}

			currentAuxiliaryLabel = currentLabel;
		}

		return ++currentAuxiliaryLabel;
	}

	void throwError(std::string msg)
	{
		std::cout << "Error: " << msg << "\n";
	}

	void throwCriticalError(std::string msg)
	{
		std::cout << "Error: " << msg << "\n";
		std::exit(EXIT_FAILURE);
	}

	std::string replicateString(std::string s, int n)
	{
		std::string result = "";

		for (int ctr = 0; ctr < n; ctr++)
		{
			result += s;
		}

		return result;
	}

	std::string addTabs(std::string s, int numberOfTabs)
	{
		std::regex newLineRegex("\\n");
		std::smatch stringMatch;
		std::string tabs = replicateString("\t", numberOfTabs);
		std::string result = tabs + s;

		result = std::regex_replace(result, newLineRegex, "\n" + tabs);

		return result;
	}

	bool stringVectorContains(std::vector<std::string> container, std::string element)
	{
		return find(container.begin(), container.end(), element) != container.end();
	}

	Ast* stringToAst(std::string parsedProgramString)
	{
		// Parse through the AST representation string character by character
		Ast* currentAst = new Ast();
		Ast* result = currentAst;
		char currentChar;
		std::string currentName = "";
		int parsedProgramStringLength = parsedProgramString.length();

		for (int ctr = 0; ctr < parsedProgramStringLength; ctr++)
		{
			currentChar = parsedProgramString.at(ctr);

			if (currentChar == config::LEFT_PARENTHESIS)	// Create a new node with the word parsed so far as its name.
			{												// Subsequently found nodes are to be added as this one's children
				currentAst->name = currentName;				// until the corresponding ')' is found.
				currentName = "";
				currentAst->addChild(new Ast);
				currentAst = currentAst->children.at(0);
			}
			else if (currentChar == config::COMMA)			// Add the currently parsed node as the previous one's sibling
			{
				if (!currentName.empty())
				{
					currentAst->name = currentName;
					currentName = "";
				}

				currentAst = currentAst->parent;
				currentAst->addChild(new Ast);
				currentAst = currentAst->children.at(currentAst->children.size() - 1);
			}
			else if (currentChar == config::RIGHT_PARENTHESIS)	// Move up one level
			{
				if (!currentName.empty())
				{
					currentAst->name = currentName;
					currentName = "";
				}

				currentAst = currentAst->parent;
			}
			else // Continue parsing the name of the next node
			{
				currentName += currentChar;
			}
		}

		return result;
	}

	booleanOperator stringToBooleanOperator(std::string operatorString)
	{
		if (operatorString == EQUALS)
		{
			return booleanOperator::BOP_EQUALS;
		}
		else if (operatorString == LESS_THAN)
		{
			return booleanOperator::BOP_LESS_THAN;
		}
		else if (operatorString == LESS_EQUALS)
		{
			return booleanOperator::BOP_LESS_EQUALS;
		}
		else if (operatorString == std::string(1, GREATER_THAN))
		{
			return booleanOperator::BOP_GREATER_THAN;
		}
		else if (operatorString == GREATER_EQUALS)
		{
			return booleanOperator::BOP_GREATER_EQUALS;
		}
		else if (operatorString == NOT_EQUALS)
		{
			return booleanOperator::BOP_NOT_EQUALS;
		}
		else if (operatorString == NOT)
		{
			return booleanOperator::BOP_NOT;
		}
		else if (operatorString == AND)
		{
			return booleanOperator::BOP_AND;
		}
		else if (operatorString == OR)
		{
			return booleanOperator::BOP_OR;
		}
		else
		{
			return booleanOperator::BOP_INVALID;
		}
	}

	std::string booleanOperatorToString(booleanOperator boolOp)
	{
		if (boolOp == booleanOperator::BOP_EQUALS)
		{
			return EQUALS;
		}
		else if (boolOp == booleanOperator::BOP_LESS_THAN)
		{
			return LESS_THAN;
		}
		else if (boolOp == booleanOperator::BOP_LESS_EQUALS)
		{
			return LESS_EQUALS;
		}
		else if (boolOp == booleanOperator::BOP_GREATER_THAN)
		{
			return std::string(1, GREATER_THAN);
		}
		else if (boolOp == booleanOperator::BOP_GREATER_EQUALS)
		{
			return GREATER_EQUALS;
		}
		else if (boolOp == booleanOperator::BOP_NOT_EQUALS)
		{
			return NOT_EQUALS;
		}
		else if (boolOp == booleanOperator::BOP_NOT)
		{
			return NOT;
		}
		else if (boolOp == booleanOperator::BOP_AND)
		{
			return AND;
		}
		else if (boolOp == booleanOperator::BOP_OR)
		{
			return OR;
		}
		else
		{
			return "INVALID";
		}
	}

	std::vector<std::vector<Ast*>> allSubsetsOfLengthK(std::vector<Ast*> superset, int K)
	{
		std::vector<std::vector<Ast*>> result;
		std::vector<Ast*> subResult;
		int superSetCardinality = superset.size();
		std::string currentMask = std::string('0', superSetCardinality - K) + std::string('1', K);

		do {
			subResult.clear();

			for (int ctr = 0; ctr < superSetCardinality; ctr++)
			{
				if (currentMask[ctr] == '1')
				{
					subResult.push_back(superset[ctr]);
				}
			}

			result.push_back(subResult);
		} while (std::next_permutation(currentMask.begin(), currentMask.end()));

		return result;
	}

	std::vector<std::vector<Ast*>> powerSetOfLimitedCardinality(std::vector<Ast*> superset, int cardinalityLimit)
	{
		std::vector<std::vector<Ast*>> result;
		std::vector<std::vector<Ast*>> subResult;

		for (int ctr = 1; ctr <= cardinalityLimit; ctr++)
		{
			subResult = allSubsetsOfLengthK(superset, ctr);
			result.insert(result.end(), subResult.begin(), subResult.end());
		}

		return result;
	}

	std::string nextBinaryRepresentation(std::string currentBinaryRepresentation, int length)
	{
		char** endptr;
		unsigned long long number = strtoull(currentBinaryRepresentation.c_str(), endptr, 2);
		std::string result = std::bitset<sizeof(unsigned long long)>(number).to_string();
		return result.substr(sizeof(unsigned long long) - length, std::string::npos);
	}

	bool cubeImpliesPredicate(std::vector<Ast*> cube, Ast* predicate)
	{
		z3::context c;
		z3::expr e = c.bool_const("x");

		z3::solver s(c);

		z3::check_result satisfiability = s.check();

		if (satisfiability == z3::check_result::sat)
		{
			return true;
		}
		else if (satisfiability == z3::check_result::unknown)
		{
			throwError("Couldn't determine satisfiability of the cube's implication of the predicate.");
		}

		return false;
	}

	bool stringVectorIsSubset(std::vector<std::string> possibleSubset, std::vector<std::string> possibleSuperset)
	{
		for (std::string member : possibleSubset)
		{
			if (!stringVectorContains(possibleSuperset, member))
			{
				return false;
			}
		}

		return true;
	}

	std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(Ast* predicate)
	{
		std::vector<int> result;
		std::vector<std::string> predicateTerms = predicate->getIDs();
		std::vector<std::string> currentGlobalPredicateTerms;
		int ctr = 0;

		for (Ast* globalPredicate : globalPredicates)
		{
			if (stringVectorIsSubset(predicateTerms, globalPredicate->getIDs()))
			{
				result.push_back(ctr);
			}

			ctr++;
		}

		return result;
	}
}