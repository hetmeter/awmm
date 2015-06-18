/*
Global variables, constants, and methods
*/

#include "config.h"
#include "literalCode.h"
#include "Ast.h"
#include "CubeTreeNode.h"
#include "GlobalVariable.h"
#include "VariableEntry.h"
#include "PredicateData.h"
//#include "Cube.h"

namespace config
{
/* Global parameters */
	int K;
	int globalCubeSizeLimit;
	int globalPredicatesCount = 0;
	std::vector<PredicateData*> globalPredicates;
	//std::vector<Ast*> globalPredicates;
	bool generateAuxiliaryPredicates = true;

	language currentLanguage;

/* Global variables */
	std::map<std::string, VariableEntry*> symbolMap;
	std::map<int, std::set<int>> labelMap;
	std::vector<int> processes;
	std::map<std::pair<std::string, std::string>, Ast*> weakestLiberalPreconditions;

	std::map<Ast*, std::vector<Ast*>> lazyReplacements;
	std::vector<std::string> variableNames;
	std::vector<std::string> auxiliaryBooleanVariableNames;
	std::vector<std::string> auxiliaryTemporaryVariableNames;
	std::map<std::string, GlobalVariable*> globalVariables;
	int currentAuxiliaryLabel = -1;
	std::map<std::string, Ast*> labelLookupMap;
	std::map<int, std::vector<int>> predicateVariableTransitiveClosures;
	bool falseImplicativeCubesIsInitialized = false;
	Ast* assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = nullptr;
	Ast* falsePredicate = nullptr;
	std::string emptyCubeRepresentation;
	std::map<std::string, CubeTreeNode*> implicativeCubes;
	std::vector<std::string> allFalseImplyingCubes;
	std::map<std::string, std::pair<Ast*, Ast*>> predicateAstRepresentations;
	/*std::map<int, std::vector<CubeTreeNode*>> implicativeCubesPerLevel;
	std::map<std::vector<int>, std::vector<int>> relevantCubeIndices;*/
	//CubeTreeNode* implicativeCubes = nullptr;

/* Global variable handling */
	PredicateData* getPredicateData(Ast* predicateAst)
	{
		std::string soughtCode = predicateAst->getCode();

		for (PredicateData* globalPredicate : globalPredicates)
		{
			if (globalPredicate->getPredicateCode().compare(soughtCode) == 0)
			{
				return globalPredicate;
			}
		}

		globalPredicates.push_back(new PredicateData(predicateAst));
		globalPredicatesCount++;
		return globalPredicates[globalPredicatesCount - 1];
	}

	bool tryRegisterGlobalSymbol(const std::string &name)
	{
		if (symbolMap.find(name) != symbolMap.end())
		{
			return false;
		}

		symbolMap.insert(std::pair<std::string, VariableEntry*>(name, new VariableEntry(name, VariableEntry::GLOBAL)));

		return true;
	}

	bool tryRegisterLocalSymbol(const std::string &name)
	{
		if (symbolMap.find(name) != symbolMap.end())
		{
			return false;
		}

		symbolMap.insert(std::pair<std::string, VariableEntry*>(name, new VariableEntry(name, VariableEntry::LOCAL)));

		return true;
	}

	bool tryRegisterAuxiliarySymbol(const std::string &name, const std::string &globalName)
	{
		if (symbolMap.find(name) != symbolMap.end() || symbolMap.find(globalName) == symbolMap.end())
		{
			return false;
		}

		symbolMap.insert(std::pair<std::string, VariableEntry*>(name, new VariableEntry(name, globalName)));

		return true;
	}

	void generateAllAuxiliarySymbols()
	{
		std::vector<std::string> allSymbolNames;

		for (std::map<std::string, VariableEntry*>::iterator it = symbolMap.begin(); it != symbolMap.end(); ++it)
		{
			allSymbolNames.push_back(it->first);
		}

		for (std::string symbolName : allSymbolNames)
		{
			if (symbolMap[symbolName]->getType() == VariableEntry::GLOBAL)
			{
				symbolMap[symbolName]->generateAuxiliaryVariables();
			}
		}
	}

	const std::string forceRegisterSymbol(VariableEntry* desiredSymbol)
	{
		std::string originalName = desiredSymbol->getName();
		std::string currentName = originalName;
		srand(time(NULL));

		while (symbolMap.find(currentName) != symbolMap.end())
		{
			currentName = originalName + literalCode::AUXILIARY_VARIABLE_SEPARATOR + std::to_string(rand());
		}

		if (currentName.compare(originalName) != 0)
		{
			desiredSymbol->setName(currentName);
		}

		symbolMap.insert(std::pair<std::string, VariableEntry*>(currentName, desiredSymbol));

		return currentName;
	}

	bool tryRegisterLabel(int process, int label)
	{
		if (labelMap.find(process) == labelMap.end())
		{
			labelMap.insert(std::pair<int, std::set<int>>(process, std::set<int>()));
		}

		if (labelMap[process].find(label) == labelMap[process].end())
		{
			labelMap[process].insert(label);
			return true;
		}

		return false;
	}

	bool tryRegisterProcess(int process)
	{
		if (std::find(processes.begin(), processes.end(), process) == processes.end())
		{
			processes.push_back(process);
			return true;
		}

		return false;
	}

	void carryOutLazyReplacements()
	{
		for (std::map<Ast*, std::vector<Ast*>>::iterator iterator = lazyReplacements.begin(); iterator != lazyReplacements.end(); iterator++)
		{
			replaceNode(iterator->second, iterator->first);
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
		std::vector<int> result;

		if (predicate->getName() == literalCode::BOOL_TOKEN_NAME && predicate->getChild(0)->getName() == literalCode::FALSE_TAG_NAME)
		{
			for (int ctr = 0; ctr < globalPredicatesCount; ctr++)
			{
				result.push_back(ctr);
			}

			return result;
		}
	
		std::vector<std::string> relevantIDs = predicate->getIDs();
	
		for (std::string id : relevantIDs)
		{
			for (int ctr = 0; ctr < globalPredicatesCount; ctr++)
			{
				if (stringVectorContains(globalPredicates[ctr]->getPredicateAst()->getIDs(), id))
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
	
		for (Ast* assignment : parallelAssignments)
		{
			relevantIDs = assignment->getChild(1)->getChild(1)->getIDs();
			relevantIDIndices.clear();
	
			for (std::string relevantID : relevantIDs)
			{
				relevantIDIndices.push_back(indexOf(relevantID));
			}
	
			result = intVectorUnion(result, relevantIDIndices);
				
			if (result.size() == globalPredicatesCount)
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
		VariableEntry* varSymbol = symbolMap[variableName];
		std::string usingVariableName = generateAuxiliaryPredicates && varSymbol->getType() == VariableEntry::AUXILIARY ?
			varSymbol->getGlobalName() : variableName;
	
		for (int ctr = 0; ctr < globalPredicatesCount; ctr++)
		{
			if (stringVectorContains(globalPredicates[ctr]->getPredicateAst()->getIDs(), variableName))
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
	
			unhandledClosureElements[index] = globalPredicates[index]->getPredicateAst()->getIDs();
	
			int ctr = 0;
			Ast* currentPredicateAst;
			for (PredicateData* globalPredicate : globalPredicates)
			{
				currentPredicateAst = globalPredicate->getPredicateAst();
				if (ctr != index)
				{
					remainingGlobalPredicateTerms[ctr] = currentPredicateAst->getIDs();
				}

				ctr++;
			}
			/*for (Ast* globalPredicate : globalPredicates)
			{
				if (ctr != index)
				{
					remainingGlobalPredicateTerms[ctr] = globalPredicate->getIDs();
				}
	
				ctr++;
			}*/
	
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

	Ast* getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes()
	{
		if (assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes == nullptr)
		{
			std::vector<int> allIndices;
			for (int ctr = 0; ctr < globalPredicatesCount; ctr++)
			{
				allIndices.push_back(ctr);
			}

			//std::vector<std::string> falseImplicativeCubeStrings = getImplicativeCubes()->getMinimalImplyingCubes(getFalsePredicate(), allIndices);
			//std::vector<std::string> falseImplicativeCubeStrings = getImplicativeCubes()->getAllFalseImplyingCubes();
			std::vector<std::string> falseImplicativeCubeStrings = getAllFalseImplyingCubes();

			std::string falseString = getFalsePredicate()->getCode();
			std::vector<Ast*> falseImplicativeCubes;

			for (std::string falseImplicativeCubeString : falseImplicativeCubeStrings)
			{
				falseImplicativeCubes.push_back(Ast::newBooleanVariableCube(falseImplicativeCubeString, false));
			}

			if (falseImplicativeCubes.size() == 0)
			{
				assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = Ast::newAssume(Ast::newTrue());
			}
			else
			{
				assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes =
					Ast::newAssume(Ast::newMultipleOperation(falseImplicativeCubes, literalCode::DOUBLE_OR)->negate());
			}

			std::cout << "\nAssumption of negated largest false implicative disjunction of cubes created.\n\n";
		}

		return assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes;
	}

	Ast* getFalsePredicate()
	{
		if (falsePredicate == nullptr)
		{
			falsePredicate = Ast::newFalse();
		}

		return falsePredicate;
	}

	std::string getEmptyCubeRepresentation()
	{
		if (emptyCubeRepresentation.empty())
		{
			emptyCubeRepresentation = std::string(globalPredicatesCount, CubeTreeNode::CUBE_STATE_OMIT);
		}

		return emptyCubeRepresentation;
	}

	std::vector<std::string> getMinimalImplyingCubes(Ast* predicate, const std::vector<int> &relevantIndices)
	{
		std::vector<std::string> result;

		if (isTautology(predicate) || isTautology(predicate->negate()))
		{
			return result;
		}

		std::vector<std::string> pending;
		pending.push_back(getEmptyCubeRepresentation());

		CubeTreeNode* currentCube;
		CubeTreeNode::Implication currentImplication;
		std::vector<std::string> currentCanonicalSupersetRepresentations;

		while (!pending.empty())
		{
			currentCube = getImplicativeCube(pending[0]);
			currentImplication = currentCube->getPredicateImplication(predicate, relevantIndices);

			if (currentImplication == CubeTreeNode::IMPLIES)
			{
				result.push_back(pending[0]);
			}
			else if (currentImplication == CubeTreeNode::NOT_IMPLIES)
			{
				currentCanonicalSupersetRepresentations = currentCube->getCanonicalSupersetStringRepresentations(relevantIndices);

				pending.insert(pending.end(), currentCanonicalSupersetRepresentations.begin(),
					currentCanonicalSupersetRepresentations.end());
			}

			pending.erase(pending.begin());
		}

		return result;
	}

	std::vector<std::string> getAllFalseImplyingCubes()
	{
		if (allFalseImplyingCubes.empty() && globalPredicatesCount > 0)
		{
			std::vector<std::string> pending;
			pending.push_back(getEmptyCubeRepresentation());

			CubeTreeNode* currentCube;
			CubeTreeNode::Implication currentImplication;
			std::vector<std::string> currentCanonicalSupersetRepresentations;

			std::vector<int> relevantIndices;
			for (int ctr = 0; ctr < globalPredicatesCount; ctr++)
			{
				relevantIndices.push_back(ctr);
			}

			while (!pending.empty())
			{
				currentCube = getImplicativeCube(pending[0]);
				currentImplication = currentCube->getPredicateImplication(getFalsePredicate(), relevantIndices);

				if (currentImplication == CubeTreeNode::IMPLIES || currentImplication == CubeTreeNode::SUPERSET_IMPLIES)
				{
					allFalseImplyingCubes.push_back(pending[0]);
				}


				currentCanonicalSupersetRepresentations = currentCube->getCanonicalSupersetStringRepresentations(relevantIndices);
				pending.insert(pending.end(), currentCanonicalSupersetRepresentations.begin(),
					currentCanonicalSupersetRepresentations.end());

				pending.erase(pending.begin());
			}
		}

		return allFalseImplyingCubes;
	}

	CubeTreeNode* getImplicativeCube(const std::string &stringRepresentation)
	{
		if (implicativeCubes.find(stringRepresentation) == implicativeCubes.end())
		{
			implicativeCubes.insert(std::pair<std::string, CubeTreeNode*>(stringRepresentation,
				new CubeTreeNode(stringRepresentation, globalCubeSizeLimit)));
		}

		return implicativeCubes.at(stringRepresentation);
	}

	/*std::pair<Ast*, Ast*> getPredicateAstRepresentationPair(Ast* predicate)
	{
		if (predicateAstRepresentations.find(predicate->getCode()) == predicateAstRepresentations.end())
		{

		}
	}
	
	Ast* getPredicateTemporaryAstRepresentation(Ast* predicate)
	{

	}
	
	Ast* getPredicateBooleanAstRepresentation(Ast* predicate)
	{

	}*/

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

/* Ast operations */
	void prepareNodeForLazyReplacement(Ast* replacement, Ast* replacedNode)
	{
		if (lazyReplacements.find(replacedNode) == lazyReplacements.end())
		{
			std::vector<Ast*> replacementVector;
			replacementVector.push_back(replacement);

			lazyReplacements.insert(std::pair<Ast*, std::vector<Ast*>>(replacedNode, replacementVector));
		}
		else
		{
			throwCriticalError("Node \"" + replacedNode->getCode() + "\" is already scheduled for replacement.");
		}
	}

	void prepareNodeForLazyReplacement(const std::vector<Ast*> &replacement, Ast* replacedNode)
	{
		if (lazyReplacements.find(replacedNode) == lazyReplacements.end())
		{
			lazyReplacements.insert(std::pair<Ast*, std::vector<Ast*>>(replacedNode, replacement));
		}
		else
		{
			throwCriticalError("Node \"" + replacedNode->getCode() + "\" is already scheduled for replacement.");
		}
	}

	void replaceNode(Ast* replacement, Ast* replacedNode)
	{
		Ast* parent = replacedNode->getParent();
		int index = replacedNode->getIndexAsChild();

		parent->replaceChild(replacement, index);
	}

	void replaceNode(const std::vector<Ast*> &replacement, Ast* replacedNode)
	{
		Ast* parent = replacedNode->getParent();
		int index = replacedNode->getIndexAsChild();

		parent->replaceChild(replacement, index);
	}

	Ast* getWeakestLiberalPrecondition(Ast* assignment, Ast* predicate)
	{
		std::pair<std::string, std::string> key = std::pair<std::string, std::string>(assignment->getCode(), predicate->getCode());

		if (weakestLiberalPreconditions.find(key) == weakestLiberalPreconditions.end())
		{
			weakestLiberalPreconditions.insert(
					std::pair<std::pair<std::string, std::string>, Ast*>(
						key, Ast::newWeakestLiberalPrecondition(assignment, predicate)
					)
				);
		}

		return weakestLiberalPreconditions[key];
	}

/* Initializations */
	void initializeAuxiliaryVariables()
	{
		std::string currentVariableName;
	
		for (int ctr = 0; ctr < globalPredicatesCount; ctr++)
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

	void initializeAuxiliaryPredicates()
	{
		Ast* currentNewPredicate;
		Ast* currentPredicateAst;
		std::vector<PredicateData*> newPredicates;
		int currentMaximumBufferSize;
		std::string currentCounterName;
		std::string currentFirstPointerName;
		std::vector<std::string> currentBufferVariables;

		// for each x
		for (std::pair<std::string, VariableEntry*> symbol : symbolMap)
		{
			if (symbol.second->getType() == VariableEntry::GLOBAL)
			{
				// for each t
				for (int processNumber : processes)
				{
					currentMaximumBufferSize = symbol.second->getMaximumBufferSize(processNumber);
					currentCounterName = symbol.second->getAuxiliaryCounterName(processNumber);
					currentFirstPointerName = symbol.second->getAuxiliaryFirstPointerName(processNumber);
					currentBufferVariables = symbol.second->getAuxiliaryBufferNames(processNumber);

					newPredicates.push_back(new PredicateData(Ast::newBinaryOp(Ast::newID(currentCounterName), Ast::newINT(K), literalCode::LESS_EQUALS)));
					newPredicates.push_back(new PredicateData(Ast::newBinaryOp(Ast::newID(currentCounterName), Ast::newINT(0), literalCode::GREATER_EQUALS)));
					newPredicates.push_back(new PredicateData(Ast::newBinaryOp(Ast::newID(currentFirstPointerName), Ast::newINT(K), literalCode::LESS_EQUALS)));
					newPredicates.push_back(new PredicateData(Ast::newBinaryOp(Ast::newID(currentFirstPointerName), Ast::newINT(0), literalCode::GREATER_EQUALS)));
					newPredicates.push_back(new PredicateData(Ast::newBinaryOp(Ast::newID(currentFirstPointerName), Ast::newID(currentCounterName), literalCode::LESS_EQUALS)));

					for (int i = 1; i <= currentMaximumBufferSize; i++)
					{
						for (PredicateData* globalPredicate : config::globalPredicates)
						{
							currentPredicateAst = globalPredicate->getPredicateAst();

							if (stringVectorContains(currentPredicateAst->getIDs(), symbol.first))
							{
								currentNewPredicate = currentPredicateAst->clone();
								currentNewPredicate->topDownCascadingReplaceIDNames(symbol.first, currentBufferVariables[i - 1]);
								newPredicates.push_back(new PredicateData(currentNewPredicate));
							}
						}
						/*for (Ast* predicate : config::globalPredicates)
						{
							if (stringVectorContains(predicate->getIDs(), symbol.first))
							{
								currentNewPredicate = predicate->clone();
								currentNewPredicate->topDownCascadingReplaceIDNames(symbol.first, currentBufferVariables[i - 1]);
								newPredicates.push_back(currentNewPredicate);
							}
						}*/
					}
				}
			}
		}

		globalPredicates.insert(globalPredicates.end(), newPredicates.begin(), newPredicates.end());
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
		throw 20;
		std::cout << "Error: " << msg << "\n";
		std::exit(EXIT_FAILURE);
	}

/* Z3 */
	bool isTautology(Ast* predicate)
	{
		z3::context c;
		z3::expr trueConstant = c.bool_val(true);

		return expressionImpliesPredicate(trueConstant, predicate);
	}

	bool cubeImpliesPredicate(const std::vector<Ast*> &cube, Ast* predicate)
	{
		if (cube.empty())
		{
			return false;
		}
	
		z3::context c;
		z3::expr cubeExpression = (Ast::newMultipleOperation(cube, literalCode::AND))->getZ3Expression(&c);

		return expressionImpliesPredicate(cubeExpression, predicate);
	}

	bool expressionImpliesPredicate(z3::expr expression, Ast* predicate)
	{
		z3::context &c = expression.ctx();
		z3::expr const & predicateExpression = predicate->getZ3Expression(&c);
		z3::solver s(c);
		z3::expr implication = impliesDuplicate(expression, predicateExpression);
		s.add(!implication);
	
		z3::check_result satisfiability = s.check();
	
		if (satisfiability == z3::check_result::unsat)
		{
			return true;
		}
		else //if (satisfiability == z3::check_result::unknown)
		{
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