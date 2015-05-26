/*
Global variables, constants, and methods
*/

#include "config.h"
#include "literalCode.h"
#include "Ast.h"
#include "CubeTreeNode.h"

namespace config
{
/* Global parameters */
	int K;
	int globalCubeSizeLimit;
	std::vector<Ast*> globalPredicates;

	language currentLanguage;

/* Global variables */
	std::map<Ast*, std::vector<Ast*>> lazyReplacements;
	std::vector<std::string> variableNames;
	std::vector<std::string> auxiliaryBooleanVariableNames;
	std::vector<std::string> auxiliaryTemporaryVariableNames;
	std::map<std::string, GlobalVariable*> globalVariables;
	int currentAuxiliaryLabel = -1;
	std::map<std::string, Ast*> labelLookupMap;
	std::map<int, std::vector<int>> predicateVariableTransitiveClosures;
	bool falseImplicativeCubesIsInitialized = false;
	CubeTreeNode* falseImplicativeCubes;
	Ast* assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = nullptr;

/* Global variable handling */
	void carryOutLazyReplacements()
	{
		for (std::map<Ast*, std::vector<Ast*>>::iterator iterator = lazyReplacements.begin(); iterator != lazyReplacements.end(); iterator++)
		{
			Ast::replaceNode(iterator->second, iterator->first);
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
	
	int indexOf(const std::string &varName)
	{
		int result = -1;
	
		for (std::string auxiliaryBooleanVariableName : auxiliaryBooleanVariableNames)
		{
			result++;
	
			if (auxiliaryBooleanVariableName.compare(varName) == 0)
			{
				return result;
			}
		}
	
		result = -1;
	
		for (std::string auxiliaryTemporaryVariableName : auxiliaryTemporaryVariableNames)
		{
			result++;
	
			if (auxiliaryTemporaryVariableName.compare(varName) == 0)
			{
				return result;
			}
		}
	
		return -1;
	}
	
	std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(Ast* predicate)
	{
		//return getPredicateVariableTransitiveClosure(indexOf(predicate));
		std::vector<int> result;
		int numberOfPredicates = globalPredicates.size();

		if (predicate->name == literalCode::BOOL_TOKEN_NAME && predicate->children.at(0)->name == literalCode::FALSE_TAG_NAME)
		{
			for (int ctr = 0; ctr < numberOfPredicates; ctr++)
			{
				result.push_back(ctr);
			}

			return result;
		}
	
		std::vector<std::string> relevantIDs = predicate->getIDs();
	
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
	
	std::vector<int> getRelevantAuxiliaryTemporaryVariableIndices(const std::vector<Ast*> &parallelAssignments)
	{
		std::vector<std::string> relevantIDs;
		std::vector<int> relevantIDIndices;
		std::vector<int> result;
		int numberOfPredicates = globalPredicates.size();
	
		for (Ast* assignment : parallelAssignments)
		{
			relevantIDs = assignment->children.at(1)->children.at(1)->getIDs();
			relevantIDIndices.clear();
	
			for (std::string relevantID : relevantIDs)
			{
				relevantIDIndices.push_back(indexOf(relevantID));
			}
	
			result = intVectorUnion(result, relevantIDIndices);
				
			if (result.size() == numberOfPredicates)
			{
				break;
			}
		}
	
		std::sort(result.begin(), result.end());
	
		return result;
	}
	
	std::vector<int> getRelevantAuxiliaryBooleanVariableIndices(const std::string &variableName)
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

	bool impliesFalse(const std::string &cubeRepresentation)
	{
		if (std::count(cubeRepresentation.begin(), cubeRepresentation.end(), CubeTreeNode::CUBE_STATE_DECIDED_TRUE) == 0 &&
			std::count(cubeRepresentation.begin(), cubeRepresentation.end(), CubeTreeNode::CUBE_STATE_DECIDED_FALSE) == 0)
		{
			return false;
		}

		return falseImplicativeCubes->containsImplying(cubeRepresentation);
	}

	void initializeFalseImplicativeCubes()
	{
		int numberOfPredicates = globalPredicates.size();
		std::string pool = std::string(numberOfPredicates, CubeTreeNode::CUBE_STATE_OMIT);

		falseImplicativeCubes = new CubeTreeNode(pool, globalCubeSizeLimit);
		falseImplicativeCubes->cascadingPopulate(globalCubeSizeLimit);
		falseImplicativeCubes->breadthFirstCheckImplication(Ast::newFalse());
		falseImplicativeCubes->scour();
		falseImplicativeCubesIsInitialized = true;

		std::cout << "\nFalse implicative cubes created.\n\n";
	}

	Ast* getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes()
	{
		if (assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes == nullptr)
		{
			std::vector<CubeTreeNode*> falseImplicativeCubeNodes = falseImplicativeCubes->getImplyingCubes();
			std::vector<Ast*> implicativeCubes;

			for (CubeTreeNode* implicativeCubeNode : falseImplicativeCubeNodes)
			{
				implicativeCubes.push_back(Ast::newBooleanVariableCube(implicativeCubeNode->stringRepresentation, false));
			}

			if (implicativeCubes.size() == 0)
			{
				assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = Ast::newAssume(Ast::newTrue());
			}
			else
			{
				assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes =
					Ast::newAssume(Ast::newMultipleOperation(implicativeCubes, literalCode::DOUBLE_OR)->negate());
			}

			std::cout << "\nAssumption of negated largest false implicative disjunction of cubes created.\n\n";
		}

		return assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes;
	}

/* String operations */
	std::string addTabs(const std::string &s, int numberOfTabs)
	{
		std::regex newLineRegex("\\n");
		std::smatch stringMatch;
		std::string tabs = std::string(numberOfTabs, '\t');
		std::string result = tabs + s;
	
		result = std::regex_replace(result, newLineRegex, "\n" + tabs);
	
		return result;
	}

/* Vector operations */
	bool stringVectorContains(const std::vector<std::string> &container, const std::string &element)
	{
		return find(container.begin(), container.end(), element) != container.end();
	}
	
	std::vector<int> intVectorUnion(const std::vector<int> &first, const std::vector<int> &second)
	{
		std::vector<int> result(first);
	
		for (int elementOfSecond : second)
		{
			if (find(result.begin(), result.end(), elementOfSecond) == result.end())
			{
				result.push_back(elementOfSecond);
			}
		}
	
		sort(result.begin(), result.end());
	
		return result;
	}
	
	bool intVectorVectorContains(const std::vector<std::vector<int>> &container, const std::vector<int> &element)
	{
		return find(container.begin(), container.end(), element) != container.end();
	}
	
	std::vector<std::vector<int>> intSetCartesianProduct(const std::vector<int> &first, const std::vector<int> &second)
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
		
	std::vector<std::vector<int>> intSetCartesianProduct(const std::vector<std::vector<int>> &first, const std::vector<int> &second)
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

/* Initializations */
	void initializeAuxiliaryVariables()
	{
		int globalPredicateCount = globalPredicates.size();
		std::string currentVariableName;
	
		for (int ctr = 0; ctr < globalPredicateCount; ctr++)
		{
			srand(time(NULL));
			currentVariableName = "B" + std::to_string(ctr);
	
			while (stringVectorContains(variableNames, currentVariableName))
			{
				currentVariableName = "B" + std::to_string(ctr) + "_" + std::to_string(rand());
			}
	
			auxiliaryBooleanVariableNames.push_back(currentVariableName);
			variableNames.push_back(currentVariableName);
	
			currentVariableName = "T" + std::to_string(ctr);
	
			while (stringVectorContains(variableNames, currentVariableName))
			{
				currentVariableName = "T" + std::to_string(ctr) + "_" + std::to_string(rand());
			}
	
			auxiliaryTemporaryVariableNames.push_back(currentVariableName);
			variableNames.push_back(currentVariableName);
		}
	}

/* Messages */
	const std::string OVERFLOW_MESSAGE = "overflow";

/* Error handling */
	void throwError(const std::string &msg)
	{
		std::cout << "Error: " << msg << "\n";
	}
	
	void throwCriticalError(const std::string &msg)
	{
		std::cout << "Error: " << msg << "\n";
		std::exit(EXIT_FAILURE);
	}

/* Z3 */
	bool cubeImpliesPredicate(const std::vector<Ast*> &cube, Ast* predicate)
	{
		if (cube.empty())
		{
			return false;
		}

		/*z3::context c;
	
		std::cout << "\tCube: " + Ast::newMultipleOperation(cube, AND)->emitCode() + " -> " + predicate->emitCode() + " returns " +
			(expressionImpliesPredicate(&c, (Ast::newMultipleOperation(cube, AND))->astToZ3Expression(&c), predicate) ? "TRUE" : "FALSE") + "\n";
	
		z3::expr cubeExpression = (Ast::newMultipleOperation(cube, AND))->astToZ3Expression(&c);
		return expressionImpliesPredicate(&c, cubeExpression, predicate);*/
	
		z3::context c;
	
		/*std::cout << "\tCube: " + Ast::newMultipleOperation(cube, AND)->emitCode() + " -> " + predicate->emitCode() + " returns " +
			(expressionImpliesPredicate(&c, (Ast::newMultipleOperation(cube, AND))->astToZ3Expression(&c), predicate) ? "TRUE" : "FALSE") + "\n";*/
	
		z3::expr cubeExpression = (Ast::newMultipleOperation(cube, literalCode::AND))->toZ3Expression(&c);
		//std::cout << "Cube " << (Ast::newMultipleOperation(cube, AND))->emitCode() << ":\n";
		//std::cout << "\t" << cube.size() << " - " << cubeExpression << "\n";
		return expressionImpliesPredicate(cubeExpression, predicate);
	}

	bool expressionImpliesPredicate(z3::expr expression, Ast* predicate)
	{
		z3::context &c = expression.ctx();
		z3::expr const & predicateExpression = predicate->toZ3Expression(&c);
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
	
	z3::expr impliesDuplicate(z3::expr const &a, z3::expr const &b)
	{
		z3::check_context(a, b);
		assert(a.is_bool() && b.is_bool());
		Z3_ast r = Z3_mk_implies(a.ctx(), a, b);
		a.check_error();
		return z3::expr(a.ctx(), r);
	}
}


//
//
//
//
//
//
//	int indexOf(Ast* predicate)
//	{
//		int result = -1;
//
//		for (Ast* globalPredicate : globalPredicates)
//		{
//			result++;
//
//			if (predicate->astToString().compare(globalPredicate->astToString()) == 0)
//			{
//				break;
//			}
//		}
//
//		return result;
//	}
//
//
//
//	std::string replicateString(std::string s, int n)
//	{
//		std::string result = "";
//
//		for (int ctr = 0; ctr < n; ctr++)
//		{
//			result += s;
//		}
//
//		return result;
//	}
//
//
//	bool stringVectorIsSubset(std::vector<std::string> possibleSubset, std::vector<std::string> possibleSuperset)
//	{
//		for (std::string member : possibleSubset)
//		{
//			if (!stringVectorContains(possibleSuperset, member))
//			{
//				return false;
//			}
//		}
//
//		return true;
//	}
//
//
//	booleanOperator stringToBooleanOperator(std::string operatorString)
//	{
//		if (operatorString == EQUALS)
//		{
//			return booleanOperator::BOP_EQUALS;
//		}
//		else if (operatorString == LESS_THAN)
//		{
//			return booleanOperator::BOP_LESS_THAN;
//		}
//		else if (operatorString == LESS_EQUALS)
//		{
//			return booleanOperator::BOP_LESS_EQUALS;
//		}
//		else if (operatorString == std::string(1, GREATER_THAN))
//		{
//			return booleanOperator::BOP_GREATER_THAN;
//		}
//		else if (operatorString == GREATER_EQUALS)
//		{
//			return booleanOperator::BOP_GREATER_EQUALS;
//		}
//		else if (operatorString == NOT_EQUALS)
//		{
//			return booleanOperator::BOP_NOT_EQUALS;
//		}
//		else if (operatorString == NOT)
//		{
//			return booleanOperator::BOP_NOT;
//		}
//		else if (operatorString == AND)
//		{
//			return booleanOperator::BOP_AND;
//		}
//		else if (operatorString == OR)
//		{
//			return booleanOperator::BOP_OR;
//		}
//		else
//		{
//			return booleanOperator::BOP_INVALID;
//		}
//	}
//
//	std::string booleanOperatorToString(booleanOperator boolOp)
//	{
//		if (boolOp == booleanOperator::BOP_EQUALS)
//		{
//			return EQUALS;
//		}
//		else if (boolOp == booleanOperator::BOP_LESS_THAN)
//		{
//			return LESS_THAN;
//		}
//		else if (boolOp == booleanOperator::BOP_LESS_EQUALS)
//		{
//			return LESS_EQUALS;
//		}
//		else if (boolOp == booleanOperator::BOP_GREATER_THAN)
//		{
//			return std::string(1, GREATER_THAN);
//		}
//		else if (boolOp == booleanOperator::BOP_GREATER_EQUALS)
//		{
//			return GREATER_EQUALS;
//		}
//		else if (boolOp == booleanOperator::BOP_NOT_EQUALS)
//		{
//			return NOT_EQUALS;
//		}
//		else if (boolOp == booleanOperator::BOP_NOT)
//		{
//			return NOT;
//		}
//		else if (boolOp == booleanOperator::BOP_AND)
//		{
//			return AND;
//		}
//		else if (boolOp == booleanOperator::BOP_OR)
//		{
//			return OR;
//		}
//		else
//		{
//			return "INVALID";
//		}
//	}
//
//	std::vector<std::vector<Ast*>> allSubsetsOfLengthK(std::vector<Ast*> superset, int K)
//	{
//		std::vector<std::vector<Ast*>> result;
//		std::vector<Ast*> subResult;
//		int superSetCardinality = superset.size();
//
//		if (superSetCardinality >= K)
//		{
//			std::string currentMask = std::string(K, '0') + std::string(superSetCardinality - K, '1');
//			//std::cout << currentMask << "\n";
//
//			// For debug purposes
//			int iterCtr = 0;
//
//			do {
//				subResult.clear();
//
//				for (int ctr = 0; ctr < superSetCardinality; ctr++)
//				{
//					if (currentMask[ctr] == '1')
//					{
//						subResult.push_back(superset[ctr]);
//					}
//				}
//
//				std::cout << iterCtr++ << ": " << currentMask << "\n";
//
//				result.push_back(subResult);
//			} while (std::next_permutation(currentMask.begin(), currentMask.end()));
//		}
//
//		return result;
//	}
//
//	std::vector<std::vector<Ast*>> powerSetOfLimitedCardinality(std::vector<Ast*> superset, int cardinalityLimit)
//	{
//		std::vector<std::vector<Ast*>> result;
//		std::vector<std::vector<Ast*>> subResult;
//
//		int minimumLimit = std::min((int)superset.size(), cardinalityLimit);
//
//		for (int ctr = 1; ctr <= cardinalityLimit; ctr++)
//		{
//			subResult = allSubsetsOfLengthK(superset, ctr);
//			result.insert(result.end(), subResult.begin(), subResult.end());
//		}
//
//		return result;
//	}
//
//	std::string nextBinaryRepresentation(std::string currentBinaryRepresentation, int length)
//	{
//		char** endptr = NULL;
//		unsigned long long number = strtoull(currentBinaryRepresentation.c_str(), endptr, 2);
//		std::string result = std::bitset<sizeof(unsigned long long)>(number).to_string();
//		return result.substr(sizeof(unsigned long long) - length, std::string::npos);
//	}
//
//
//
//
//
//	std::vector<Ast*> cubeStringRepresentationToAstRef(std::string pool)
//	{
//		int numberOfPredicates = globalPredicates.size();
//		std::vector<Ast*> cubeTerms;
//		Ast* currentTerm;
//
//		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
//		{
//			if (pool[ctr] == CUBE_STATE_DECIDED_FALSE)
//			{
//				currentTerm = globalPredicates[ctr]->negate();
//				cubeTerms.push_back(currentTerm);
//			}
//			else if (pool[ctr] == CUBE_STATE_DECIDED_TRUE)
//			{
//				currentTerm = globalPredicates[ctr]->clone();
//				cubeTerms.push_back(currentTerm);
//			}
//		}
//
//		return cubeTerms;
//	}