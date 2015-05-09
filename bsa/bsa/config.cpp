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
	const std::string BOOL_TOKEN_NAME = "BOOL";
	const std::string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
	const std::string BL_PROGRAM_DECLARATION_TOKEN_NAME = "booleanProgramDeclaration";
	const std::string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
	const std::string BL_PROCESS_DECLARATION_TOKEN_NAME = "booleanProcess";
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
	const std::string BL_INITIALIZATION_BLOCK_TOKEN_NAME = "booleanInitialization";
	const std::string BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME = "local";
	const std::string BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME = "shared";
	const std::string LOCAL_ASSIGN_TOKEN_NAME = "localAssign";
	const std::string ASSUME_TOKEN_NAME = "assume";
	const std::string BEGIN_ATOMIC_TOKEN_NAME = "begin_atomic";
	const std::string END_ATOMIC_TOKEN_NAME = "end_atomic";
	const std::string CHOOSE_TOKEN_NAME = "choose";

	const std::string BEGINIT_TAG_NAME = "beginit";
	const std::string ENDINIT_TAG_NAME = "endinit";
	const std::string PROCESS_TAG_NAME = "process";
	const std::string INIT_TAG_NAME = "init";
	const std::string IF_TAG_NAME = "if";
	const std::string ELSE_TAG_NAME = "else";
	const std::string ENDIF_TAG_NAME = "endif";
	const std::string TRUE_TAG_NAME = "true";
	const std::string FALSE_TAG_NAME = "false";

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
	const std::string BOOLEAN_EXTENSION = "bl";
	const std::string PREDICATE_EXTENSION = "predicate";
	const std::string BSA_EXTENSION = "bsa";
	const std::string OUT_EXTENSION = "out";

	const std::string PREDICATE_ABSTRACTION_COMMENT_PREFIX = "Predicate abstraction: ";

	language currentLanguage;
	std::map<std::string, Ast*> labelLookupMap;
	std::vector<std::string> variableNames;
	std::map<std::string, GlobalVariable*> globalVariables;
	std::vector<Ast*> globalPredicates;

	int indexOf(Ast* predicate)
	{
		int result = -1;

		for (Ast* globalPredicate : globalPredicates)
		{
			result++;

			if (predicate->astToString().compare(globalPredicate->astToString()) == 0)
			{
				break;
			}
		}

		return result;
	}

	std::vector<std::string> auxiliaryBooleanVariableNames;
	std::vector<std::string> auxiliaryTemporaryVariableNames;
	std::map<int, std::vector<int>> predicateVariableTransitiveClosures;

	std::vector<int> getPredicateVariableTransitiveClosure(int index)
	{
		if (predicateVariableTransitiveClosures.find(index) == predicateVariableTransitiveClosures.end())
		{
			std::map<int, std::vector<std::string>> remainingGlobalPredicateTerms;
			std::map<int, std::vector<std::string>> toBeTransferred;
			std::map<int, std::vector<std::string>> unhandledClosureElements;
			int currentClosureElement;
			std::vector<std::string> currentTempClosure;
			std::vector<int> handledClosureElements;

			unhandledClosureElements[index] = globalPredicates[index]->getIDs();

			int ctr = 0;
			for (Ast* globalPredicate : globalPredicates)
			{
				if (ctr != index)
				{
					remainingGlobalPredicateTerms[ctr] = globalPredicate->getIDs();
				}

				ctr++;
			}

			while (!unhandledClosureElements.empty())
			{
				currentClosureElement = unhandledClosureElements.begin()->first;
				currentTempClosure = unhandledClosureElements.begin()->second;
				toBeTransferred.clear();

				for (std::vector<std::string>::iterator termIterator = currentTempClosure.begin(); termIterator != currentTempClosure.end(); termIterator++)
				{
					for (std::map<int, std::vector<std::string>>::iterator predicateIterator = remainingGlobalPredicateTerms.begin(); predicateIterator != remainingGlobalPredicateTerms.end(); predicateIterator++)
					{
						if (stringVectorContains(predicateIterator->second, *termIterator))
						{
							toBeTransferred[predicateIterator->first] = predicateIterator->second;
						}
					}
				}

				for (std::map<int, std::vector<std::string>>::iterator transferIterator = toBeTransferred.begin(); transferIterator != toBeTransferred.end(); transferIterator++)
				{
					unhandledClosureElements[transferIterator->first] = transferIterator->second;
					remainingGlobalPredicateTerms.erase(transferIterator->first);
				}

				handledClosureElements.push_back(currentClosureElement);
				unhandledClosureElements.erase(currentClosureElement);
			}

			predicateVariableTransitiveClosures[index] = handledClosureElements;
		}

		return predicateVariableTransitiveClosures[index];
	}

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

			auxiliaryBooleanVariableNames.push_back(currentVariableName);
			variableNames.push_back(currentVariableName);

			currentVariableName = "t_" + std::to_string(ctr);

			while (stringVectorContains(variableNames, currentVariableName))
			{
				currentVariableName = "t_" + std::to_string(ctr) + "_" + std::to_string(rand());
			}

			auxiliaryTemporaryVariableNames.push_back(currentVariableName);
			variableNames.push_back(currentVariableName);
		}
	}

	int getCurrentAuxiliaryLabel()
	{
		if (currentAuxiliaryLabel == -1)
		{
			int maxLabel = -1;
			int currentLabel = 1;

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

	std::vector<int> intVectorUnion(std::vector<int> first, std::vector<int> second)
	{
		std::vector<int> result(first);

		for (int elementOfSecond : second)
		{
			if (find(first.begin(), first.end(), elementOfSecond) == first.end())
			{
				result.push_back(elementOfSecond);
			}
		}

		sort(result.begin(), result.end());

		return result;
	}

	std::vector<std::vector<int>> intSetCartesianProduct(std::vector<int> first, std::vector<int> second)
	{
		std::vector<std::vector<int>> result;
		std::vector<int> currentProduct;

		for (int elementOfFirst : first)
		{
			for (int elementOfSecond : second)
			{
				if (elementOfFirst != elementOfSecond)
				{
					currentProduct = std::vector<int>();
					currentProduct.push_back(std::min(elementOfFirst, elementOfSecond));
					currentProduct.push_back(std::max(elementOfFirst, elementOfSecond));

					if (!intVectorVectorContains(result, currentProduct))
					{
						result.push_back(currentProduct);
					}
				}
			}
		}

		return result;
	}

	bool intVectorVectorContains(std::vector<std::vector<int>> container, std::vector<int> element)
	{
		return find(container.begin(), container.end(), element) != container.end();
	}
	
	std::vector<std::vector<int>> intSetCartesianProduct(std::vector<std::vector<int>> first, std::vector<int> second)
	{
		std::vector<std::vector<int>> result;
		std::vector<int> currentProduct;

		for (std::vector<int> elementOfFirst : first)
		{
			for (int elementOfSecond : second)
			{
				if (find(elementOfFirst.begin(), elementOfFirst.end(), elementOfSecond) == elementOfFirst.end())
				{
					currentProduct = std::vector<int>(elementOfFirst);
					currentProduct.push_back(elementOfSecond);
					std::sort(currentProduct.begin(), currentProduct.end());

					if (!intVectorVectorContains(result, currentProduct))
					{
						result.push_back(currentProduct);
					}
				}
			}
		}

		return result;
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

		if (superSetCardinality >= K)
		{
			std::string currentMask = std::string(K, '0') + std::string(superSetCardinality - K, '1');
			//std::cout << currentMask << "\n";

			// For debug purposes
			int iterCtr = 0;

			do {
				subResult.clear();

				for (int ctr = 0; ctr < superSetCardinality; ctr++)
				{
					if (currentMask[ctr] == '1')
					{
						subResult.push_back(superset[ctr]);
					}
				}

				std::cout << iterCtr++ << ": " << currentMask << "\n";

				result.push_back(subResult);
			} while (std::next_permutation(currentMask.begin(), currentMask.end()));
		}

		return result;
	}

	std::vector<std::vector<Ast*>> powerSetOfLimitedCardinality(std::vector<Ast*> superset, int cardinalityLimit)
	{
		std::vector<std::vector<Ast*>> result;
		std::vector<std::vector<Ast*>> subResult;

		int minimumLimit = std::min((int)superset.size(), cardinalityLimit);

		for (int ctr = 1; ctr <= cardinalityLimit; ctr++)
		{
			subResult = allSubsetsOfLengthK(superset, ctr);
			result.insert(result.end(), subResult.begin(), subResult.end());
		}

		return result;
	}

	std::string nextBinaryRepresentation(std::string currentBinaryRepresentation, int length)
	{
		char** endptr = NULL;
		unsigned long long number = strtoull(currentBinaryRepresentation.c_str(), endptr, 2);
		std::string result = std::bitset<sizeof(unsigned long long)>(number).to_string();
		return result.substr(sizeof(unsigned long long) - length, std::string::npos);
	}

	bool cubeImpliesPredicate(std::vector<Ast*> cube, Ast* predicate)
	{
		/*z3::context c;

		std::cout << "\tCube: " + Ast::newMultipleOperation(cube, AND)->emitCode() + " -> " + predicate->emitCode() + " returns " +
			(expressionImpliesPredicate(&c, (Ast::newMultipleOperation(cube, AND))->astToZ3Expression(&c), predicate) ? "TRUE" : "FALSE") + "\n";

		z3::expr cubeExpression = (Ast::newMultipleOperation(cube, AND))->astToZ3Expression(&c);
		return expressionImpliesPredicate(&c, cubeExpression, predicate);*/

		z3::context c;

		/*std::cout << "\tCube: " + Ast::newMultipleOperation(cube, AND)->emitCode() + " -> " + predicate->emitCode() + " returns " +
			(expressionImpliesPredicate(&c, (Ast::newMultipleOperation(cube, AND))->astToZ3Expression(&c), predicate) ? "TRUE" : "FALSE") + "\n";*/

		z3::expr cubeExpression = (Ast::newMultipleOperation(cube, AND))->astToZ3Expression(&c);
		//std::cout << "Cube " << (Ast::newMultipleOperation(cube, AND))->emitCode() << ":\n";
		//std::cout << "\t" << cube.size() << " - " << cubeExpression << "\n";
		return expressionImpliesPredicate(cubeExpression, predicate);
	}

	bool expressionImpliesPredicate(z3::expr expression, Ast* predicate)
	{
		z3::context &c = expression.ctx();
		z3::expr const & predicateExpression = predicate->astToZ3Expression(&c);
		z3::solver s(c);
		z3::expr implication = impliesDuplicate(expression, predicateExpression);
		s.add(!implication);

		z3::check_result satisfiability = s.check();

		if (satisfiability == z3::check_result::unsat)
		{
			//std::cout << expression << "\tIMPLIES\t" << predicateExpression << "\n";
			return true;
		}
		else //if (satisfiability == z3::check_result::unknown)
		{
			//std::cout << expression << "\DOES NOT IMPLY\t" << predicateExpression << "\n";
			return false;
		}
	}

	std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(Ast* predicate)
	{
		//return getPredicateVariableTransitiveClosure(indexOf(predicate));

		std::vector<std::string> relevantIDs = predicate->getIDs();
		std::vector<int> result;
		int numberOfPredicates = globalPredicates.size();

		for (std::string id : relevantIDs)
		{
			for (int ctr = 0; ctr < numberOfPredicates; ctr++)
			{
				if (stringVectorContains(globalPredicates[ctr]->getIDs(), id))
				{
					result = intVectorUnion(result, getPredicateVariableTransitiveClosure(ctr));
					
					// The closures of two predicates sharing at least one term are equal,
					// therefore once we've found one relevant predicate, we needn't find another one
					break;
				}
			}
		}

		return result;
	}

	std::vector<int> getRelevantAuxiliaryBooleanVariableIndices(std::string variableName)
	{
		std::vector<int> result;
		int numberOfPredicates = globalPredicates.size();

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			if (stringVectorContains(globalPredicates[ctr]->getIDs(), variableName))
			{
				result.push_back(ctr);
			}
		}

		return result;
	}

	const char CUBE_STATE_OMIT = '-';
	const char CUBE_STATE_UNDECIDED = '?';
	const char CUBE_STATE_MAY_BE_FALSE = 'f';
	const char CUBE_STATE_MAY_BE_TRUE = 't';
	const char CUBE_STATE_DECIDED_FALSE = 'F';
	const char CUBE_STATE_DECIDED_TRUE = 'T';

	std::string getCubeStatePool(std::vector<int> predicateIndices)
	{
		std::string result = std::string(globalPredicates.size(), CUBE_STATE_OMIT);

		for (int predicateIndex : predicateIndices)
		{
			result[predicateIndex] = CUBE_STATE_UNDECIDED;
		}

		return result;
	}

	std::string getCubeStatePool(int predicateIndex)
	{
		std::string result = std::string(globalPredicates.size(), CUBE_STATE_OMIT);
		result[predicateIndex] = CUBE_STATE_UNDECIDED;
		return result;
	}

	std::vector<std::string> getNaryCubeStateCombinations(std::vector<int> predicateIndices, int n)
	{
		int numberOfPredicates = globalPredicates.size();
		std::string emptyCube = std::string(numberOfPredicates, CUBE_STATE_OMIT);
		std::string currentCube;
		std::vector<std::string> result;

		if (n == 1)
		{
			for (int predicateIndex : predicateIndices)
			{
				currentCube = getCubeStatePool(predicateIndex);
				result.push_back(currentCube);
			}
		}
		else if (n > 1)
		{
			std::vector<std::vector<int>> cubeIndexSet = intSetCartesianProduct(predicateIndices, predicateIndices);

			for (int ctr = 2; ctr < n; ctr++)
			{
				cubeIndexSet = intSetCartesianProduct(cubeIndexSet, predicateIndices);
			}

			for (std::vector<int> cubeIndices : cubeIndexSet)
			{
				currentCube = getCubeStatePool(cubeIndices);
				result.push_back(currentCube);
			}
		}

		return result;
	}

	std::vector<std::string> getImplicativeCubeStates(std::string pool, Ast* predicate)
	{
		int numberOfPredicates = globalPredicates.size();
		bool fullyDefined = true;
		std::vector<std::string> result;
		std::vector<std::string> subResult;
		std::string poolCopy;

		int a;

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			//std::cout << "\t" << ctr << " / " << numberOfPredicates << "\n";

			if (pool[ctr] == CUBE_STATE_UNDECIDED || pool[ctr] == CUBE_STATE_MAY_BE_FALSE)
			{
				poolCopy = std::string(pool);
				poolCopy[ctr] = CUBE_STATE_DECIDED_FALSE;

				subResult = getImplicativeCubeStates(poolCopy, predicate);
				result.insert(result.end(), subResult.begin(), subResult.end());
				
				fullyDefined = false;
			}

			if (pool[ctr] == CUBE_STATE_UNDECIDED || pool[ctr] == CUBE_STATE_MAY_BE_TRUE)
			{
				poolCopy = std::string(pool);
				poolCopy[ctr] = CUBE_STATE_DECIDED_TRUE;

				subResult = getImplicativeCubeStates(poolCopy, predicate);
				result.insert(result.end(), subResult.begin(), subResult.end());

				fullyDefined = false;
			}
		}

		if (fullyDefined)
		{
			std::vector<Ast*> cubeTerms;
			Ast* currentTerm;

			for (int ctr = 0; ctr < numberOfPredicates; ctr++)
			{
				if (pool[ctr] == CUBE_STATE_DECIDED_FALSE)
				{
					currentTerm = globalPredicates[ctr]->negate();
					cubeTerms.push_back(currentTerm);
				}
				else if (pool[ctr] == CUBE_STATE_DECIDED_TRUE)
				{
					currentTerm = globalPredicates[ctr]->clone();
					cubeTerms.push_back(currentTerm);
				}
			}

			if (cubeImpliesPredicate(cubeTerms, predicate))
			{
				result.push_back(pool);
			}
		}

		return result;
	}

	std::string removeDecisionsFromPool(std::string pool, std::vector<std::string> decisions)
	{
		int numberOfPredicates = globalPredicates.size();
		std::string result(pool);

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			if (result[ctr] != CUBE_STATE_OMIT)
			{
				for (std::string decision : decisions)
				{
					if (decision[ctr] == CUBE_STATE_DECIDED_FALSE)
					{
						if (result[ctr] == CUBE_STATE_UNDECIDED)
						{
							result[ctr] = CUBE_STATE_MAY_BE_TRUE;
						}
						else if (result[ctr] == CUBE_STATE_MAY_BE_FALSE)
						{
							result[ctr] = CUBE_STATE_OMIT;
						}
					}
					else if (decision[ctr] == CUBE_STATE_DECIDED_TRUE)
					{
						if (result[ctr] == CUBE_STATE_UNDECIDED)
						{
							result[ctr] = CUBE_STATE_MAY_BE_FALSE;
						}
						else if (result[ctr] == CUBE_STATE_MAY_BE_TRUE)
						{
							result[ctr] = CUBE_STATE_OMIT;
						}
					}
					else if (decision[ctr] == CUBE_STATE_OMIT)
					{
						result[ctr] = CUBE_STATE_OMIT;
					}
				}
			}
		}

		/*std::cout << "\t" << pool;

		for (std::string d : decisions)
		{
			std::cout << "\t" << d;
		}
		
		std::cout << "\t" << result << "\n";*/

		return result;
	}

	std::string applyDecisionMask(std::string pool, std::string decisionMask)
	{
		int numberOfPredicates = globalPredicates.size();
		std::string result(pool);

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			if (result[ctr] != CUBE_STATE_OMIT)
			{
				if (decisionMask[ctr] == CUBE_STATE_OMIT)
				{
					result[ctr] = CUBE_STATE_OMIT;
				}
				else if (decisionMask[ctr] == CUBE_STATE_MAY_BE_FALSE && result[ctr] == CUBE_STATE_UNDECIDED)
				{
					result[ctr] = CUBE_STATE_MAY_BE_FALSE;
				}
				else if (decisionMask[ctr] == CUBE_STATE_MAY_BE_TRUE && result[ctr] == CUBE_STATE_UNDECIDED)
				{
					result[ctr] = CUBE_STATE_MAY_BE_TRUE;
				}
			}
		}

		//std::cout << pool << "\t" << decisionMask << "\t" << result << "\n";

		return result;
	}

	z3::expr impliesDuplicate(z3::expr const &a, z3::expr const &b)
	{
		z3::check_context(a, b);
		assert(a.is_bool() && b.is_bool());
		Z3_ast r = Z3_mk_implies(a.ctx(), a, b);
		a.check_error();
		return z3::expr(a.ctx(), r);
	}
}