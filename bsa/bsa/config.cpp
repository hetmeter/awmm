/*The MIT License(MIT)

Copyright(c) 2015 Attila Zoltán Printz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
Global variables, constants, and methods
*/

#include "config.h"
#include "literalCode.h"
#include "Ast.h"
#include "CubeTreeNode.h"
#include "VariableEntry.h"
#include "PredicateData.h"
#include "MergeableSetContainer.h"

namespace config
{
/* Global parameters */

	int K;
	int globalCubeSizeLimit;
	int originalPredicatesCount = 0;
	int totalPredicatesCount = 0;
	std::vector<std::string> originalPredicateCodes;
	std::map<std::string, PredicateData*> predicates;
	bool generateAuxiliaryPredicates = true;
	bool evaluationMode = false;
	bool verboseMode = false;
	int negatedLargestFalseImplicativeDisjunctionSizeThreshold = 500;

	language currentLanguage;
	
	ordering memoryOrdering;

/* Global variables */

	std::map<std::string, VariableEntry*> symbolMap;
	std::map<int, std::set<int>> labelMap;
	std::vector<int> processes;
	std::map<std::pair<std::string, std::string>, Ast*> weakestLiberalPreconditions;
	std::map<std::pair<std::pair<int, bool>, std::string>, Ast*> largestImplicativCubeDisjunctions;

	std::map<Ast*, std::vector<Ast*>> lazyReplacements;
	int currentAuxiliaryLabel = -1;
	std::map<std::pair<int, int>, Ast*> labelLookupMap;
	Ast* assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = nullptr;
	Ast* falsePredicate = nullptr;
	std::string emptyCubeRepresentation;
	std::map<std::string, CubeTreeNode*> implicativeCubes;
	std::vector<std::string> allFalseImplyingCubeStringRepresentations;
	MergeableSetContainer* variableTransitiveClosures;

/* Global variable handling */
	
	// If there is no symbol registered under the desired name, registers it as global and returns true. Otherwise returns false.
	bool tryRegisterGlobalSymbol(const std::string &name)
	{
		if (symbolMap.find(name) != symbolMap.end())
		{
			return false;
		}

		symbolMap.insert(std::pair<std::string, VariableEntry*>(name, new VariableEntry(name, VariableEntry::GLOBAL)));

		return true;
	}

	// If there is no symbol registered under the desired name, registers it as local and returns true. Otherwise returns false.
	bool tryRegisterLocalSymbol(const std::string &name)
	{
		if (symbolMap.find(name) != symbolMap.end())
		{
			return false;
		}

		symbolMap.insert(std::pair<std::string, VariableEntry*>(name, new VariableEntry(name, VariableEntry::LOCAL)));

		return true;
	}
	
	// Prompts each symbol to generate a counter-, a first pointer-, and K buffer variables per process
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

	// Registers a symbol with the given name, if unique. Otherwise, registers is with the given name and a random suffix.
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

	// If the target label is not registered, registers it and returns true. Otherwise returns false.
	bool tryRegisterLabel(int process, int label)
	{
		if (labelMap.find(process) == labelMap.end())
		{
			//labelMap.insert(std::pair<int, std::set<int>>(process, std::set<int>()));
			labelMap[process] = std::set<int>();
		}

		if (labelMap[process].find(label) == labelMap[process].end())
		{
			labelMap[process].insert(label);
			return true;
		}

		return false;
	}

	// If the target process is not registered, registers it and returns true. Otherwise returns false.
	bool tryRegisterProcess(int process)
	{
		if (std::find(processes.begin(), processes.end(), process) == processes.end())
		{
			processes.push_back(process);
			return true;
		}

		return false;
	}

	// Prompts each entry in the lazy replacements map to carry out the scheduled replacements
	void carryOutLazyReplacements()
	{
		for (std::map<Ast*, std::vector<Ast*>>::iterator iterator = lazyReplacements.begin(); iterator != lazyReplacements.end(); iterator++)
		{
			replaceNode(iterator->second, iterator->first);
		}
	}
	
	// Returns the highest non-existing label so far
	int getCurrentAuxiliaryLabel()
	{
		if (currentAuxiliaryLabel == -1)
		{
			for (int process : processes)
			{
				for (int label : labelMap[process])
				{
					if (label > currentAuxiliaryLabel)
					{
						currentAuxiliaryLabel = label + 1;
					}
				}
			}
		}
	
		return ++currentAuxiliaryLabel;
	}
	
	// Returns the indices of all original predicates that are in the union of the transitive closures of all parameter variables
	std::vector<int> getVariableTransitiveClosureFromOriginalPredicates(std::set<std::string> variables)
	{
		if (variableTransitiveClosures == nullptr)
		{
			variableTransitiveClosures = new MergeableSetContainer();
		}

		std::set<int> resultSet = variableTransitiveClosures->getSet(variables);
		std::vector<int> resultVector;

		for (std::set<int>::iterator it = resultSet.begin(); it != resultSet.end(); ++it)
		{
			resultVector.push_back(*it);
		}

		return resultVector;
	}

	// Returns the codes of all original predicates that contain the specified symbol
	std::vector<std::string> getOriginalPredicateCodesContainingSymbol(const std::string &symbol)
	{
		std::vector<std::string> result;
		std::set<std::string> subResult;

		for (std::string originalPredicateCode : originalPredicateCodes)
		{
			subResult = predicates[originalPredicateCode]->getPredicateIDs();

			if (subResult.find(symbol) != subResult.end())
			{
				result.push_back(originalPredicateCode);
			}
		}

		return result;
	}

	// Returns an ast node containing the assumption of the negation of the disjunction of all computable cubes implying false
	Ast* getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes()
	{
		if (assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes == nullptr)
		{
			Ast* returnedNop = Ast::newLabel(getCurrentAuxiliaryLabel(), Ast::newNop());

			if (getAllFalseImplyingCubeStringRepresentations().size() == 0)
			{
				returnedNop->setStartComment(literalCode::ASSUMPTION_MOOT_CAPTION);
				assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = returnedNop;
				return returnedNop;
			}

			int cubeCtr = 1;
			int cubeSubCtr;
			std::vector<Ast*> resultVector;
			std::vector<std::pair<std::set<std::string>, Ast*>> originalCubes;
			Ast* currentCube;
			std::set<std::string> currentCubeIDs;
			std::set<std::string> originalIDs;

			for (std::string falseCube : allFalseImplyingCubeStringRepresentations)
			{
				currentCube = Ast::newBooleanVariableCube(falseCube, false);
				currentCubeIDs = currentCube->getIDset();
				originalIDs.insert(currentCubeIDs.begin(), currentCubeIDs.end());
				originalCubes.push_back(std::pair<std::set<std::string>, Ast*>(currentCubeIDs, currentCube));
				resultVector.push_back(currentCube);
			}

			std::map<std::pair<std::string, int>, std::set<std::string>> replacementMap;
			std::pair<std::string, int> currentKey;
			PredicateData* currentPredicate;
			std::vector<PredicateData*> currentVariants;

			for (std::set<std::string>::iterator it = originalIDs.begin(); it != originalIDs.end(); ++it)
			{
				currentPredicate = config::symbolMap[*it]->getAssociatedPredicate();

				for (int process : processes)
				{
					currentKey = std::pair<std::string, int>(*it, process);
					currentVariants = currentPredicate->getAllReplacementVariants(process);

					for (PredicateData* variant : currentVariants)
					{
						replacementMap[currentKey].insert(variant->getSingleBooleanVariableName());
					}
				}
			}

			std::vector<std::pair<std::set<std::string>, Ast*>> subResult;
			int subResultCount;
			std::set<std::string> currentIDset;
			std::set<std::string> newIDset;
			std::set<std::string> currentReplacementSet;

			for (int process : processes)
			{
				cubeSubCtr = 1;
				subResult.clear();
				subResult.insert(subResult.begin(), originalCubes.begin(), originalCubes.end());

				for (std::set<std::string>::iterator it = originalIDs.begin(); it != originalIDs.end(); ++it)
				{
					subResultCount = subResult.size();

					for (int ctr = 0; ctr < subResultCount; ++ctr)
					{
						currentIDset = subResult[ctr].first;

						if (currentIDset.find(*it) != currentIDset.end())
						{
							currentReplacementSet = replacementMap[std::pair<std::string, int>(*it, process)];

							for (std::set<std::string>::iterator subIt = currentReplacementSet.begin();
								subIt != currentReplacementSet.end(); ++subIt)
							{
								currentCube = subResult[ctr].second->clone();
								currentCube->topDownCascadingReplaceIDNames(*it, *subIt);
								newIDset = currentCube->getIDset();
								subResult.push_back(std::pair<std::set<std::string>, Ast*>(newIDset, currentCube));

								if (++cubeSubCtr >= negatedLargestFalseImplicativeDisjunctionSizeThreshold)
								{
									returnedNop->setStartComment(literalCode::ASSUMPTION_EXCESSIVE_CAPTION);
									assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = returnedNop;
									return returnedNop;
								}
							}
						}
					}
				}

				for (std::pair<std::set<std::string>, Ast*> subResultEntry : subResult)
				{
					resultVector.push_back(subResultEntry.second);

					if (++cubeCtr >= negatedLargestFalseImplicativeDisjunctionSizeThreshold)
					{
						returnedNop->setStartComment(literalCode::ASSUMPTION_EXCESSIVE_CAPTION);
						assumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes = returnedNop;
						return returnedNop;
					}
				}
			}

			return Ast::newAssume(Ast::newMultipleOperation(resultVector, literalCode::DOUBLE_OR)->negate());
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

	// Returns the string representation of an empty cube, i.e. all terms are omitted
	std::string getEmptyCubeRepresentation()
	{
		if (emptyCubeRepresentation.empty())
		{
			emptyCubeRepresentation = std::string(originalPredicatesCount, CubeTreeNode::CUBE_STATE_OMIT);
		}

		return emptyCubeRepresentation;
	}

	// Returns a vector containing the string representations of all cubes that imply the predicate, without any of the cubes implying eachother
	std::vector<std::string> getMinimalImplyingCubeStringRepresentations(Ast* predicate, const std::vector<int> &relevantIndices)
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

	// Returns a vector containing the string representations of all cubes that imply false, without any of the cubes implying eachother
	std::vector<std::string> getAllFalseImplyingCubeStringRepresentations()
	{
		if (allFalseImplyingCubeStringRepresentations.empty() && originalPredicatesCount > 0)
		{
			std::vector<std::string> pending;
			pending.push_back(getEmptyCubeRepresentation());

			CubeTreeNode* currentCube;
			CubeTreeNode::Implication currentImplication;
			std::vector<std::string> currentCanonicalSupersetRepresentations;

			std::vector<int> relevantIndices;
			for (int ctr = 0; ctr < originalPredicatesCount; ctr++)
			{
				relevantIndices.push_back(ctr);
			}

			while (!pending.empty())
			{
				currentCube = getImplicativeCube(pending[0]);
				currentImplication = currentCube->getPredicateImplication(getFalsePredicate(), relevantIndices);

				//if (currentImplication == CubeTreeNode::IMPLIES || currentImplication == CubeTreeNode::SUPERSET_IMPLIES)
				if (currentImplication == CubeTreeNode::IMPLIES)
				{
					allFalseImplyingCubeStringRepresentations.push_back(pending[0]);
				}


				currentCanonicalSupersetRepresentations = currentCube->getCanonicalSupersetStringRepresentations(relevantIndices);
				pending.insert(pending.end(), currentCanonicalSupersetRepresentations.begin(),
					currentCanonicalSupersetRepresentations.end());

				pending.erase(pending.begin());
			}
		}

		return allFalseImplyingCubeStringRepresentations;
	}

	// Returns the CubeTreeNode* associated with the given string representation of a cube
	CubeTreeNode* getImplicativeCube(const std::string &stringRepresentation)
	{
		if (implicativeCubes.find(stringRepresentation) == implicativeCubes.end())
		{
			implicativeCubes.insert(std::pair<std::string, CubeTreeNode*>(stringRepresentation,
				new CubeTreeNode(stringRepresentation, globalCubeSizeLimit)));
		}

		return implicativeCubes.at(stringRepresentation);
	}
	
/* String operations */

	// Adds a tab character to the beginning of each line
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

	// Returns whether the container has the specified element
	bool stringVectorContains(const std::vector<std::string> &container, const std::string &element)
	{
		return find(container.begin(), container.end(), element) != container.end();
	}
	
	// Performs a set union operation on two vectors
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
	
/* Ast operations */

	void prepareNodeForLazyReplacement(const std::vector<Ast*> &replacement, Ast* replacedNode)
	{
		if (lazyReplacements.find(replacedNode) == lazyReplacements.end())
		{
			lazyReplacements.insert(std::pair<Ast*, std::vector<Ast*>>(replacedNode, replacement));
		}
		else
		{
			std::string originalReplacementCode = "";
			std::string newReplacementCode = "";

			for (Ast* originalReplacement : lazyReplacements[replacedNode])
			{
				originalReplacementCode += "\n\t{" + originalReplacement->getCode() + "}";
			}

			for (Ast* newReplacement : replacement)
			{
				newReplacementCode += "\n\t{" + newReplacement->getCode() + "}";
			}

			if (originalReplacementCode.compare(newReplacementCode) != 0)
			{
				throwCriticalError("Node \"" + replacedNode->getCode() + "\" is already scheduled for replacement.\nCurrent replacement:" +
					originalReplacementCode + "\nNew replacement:" + newReplacementCode);
			}
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

	Ast* getLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables)
	{
		std::string predicateCode = predicate->getCode();
		std::pair<std::pair<int, bool>, std::string> key = std::pair<std::pair<int, bool>, std::string>(
			std::pair<int, bool>(cubeSizeUpperLimit, useTemporaryVariables), predicateCode);

		if (largestImplicativCubeDisjunctions.find(key) == largestImplicativCubeDisjunctions.end())
		{
			std::pair<std::pair<int, bool>, std::string> alternateKey = std::pair<std::pair<int, bool>, std::string>(
				std::pair<int, bool>(cubeSizeUpperLimit, !useTemporaryVariables), predicateCode);

			if (largestImplicativCubeDisjunctions.find(alternateKey) == largestImplicativCubeDisjunctions.end())
			{
				if (isTautology(predicate))
				{
					largestImplicativCubeDisjunctions[key] = Ast::newTrue();
				}
				else if (isTautology(predicate->negate()))
				{
					largestImplicativCubeDisjunctions[key] = Ast::newFalse();
				}
				else
				{
					largestImplicativCubeDisjunctions[key] = Ast::newLargestImplicativeDisjunctionOfCubes(
						cubeSizeUpperLimit, predicate, useTemporaryVariables);
				}
			}
			else
			{
				largestImplicativCubeDisjunctions[key] = largestImplicativCubeDisjunctions[alternateKey]->clone();
				std::set<std::string> IDset = largestImplicativCubeDisjunctions[key]->getIDset();

				for (std::set<std::string>::iterator it = IDset.begin(); it != IDset.end(); ++it)
				{
					largestImplicativCubeDisjunctions[key]->topDownCascadingReplaceIDNames(*it, useTemporaryVariables ?
						symbolMap[*it]->getTemporaryVariantName() : symbolMap[*it]->getBooleanVariantName());
				}
			}
		}

		return largestImplicativCubeDisjunctions[key];
	}

/* Initializations */

	void initializeAuxiliaryPredicates()
	{
		if (!config::evaluationMode)
		{
			std::cout << "Generating auxiliary predicates...\n";
		}

		std::set<std::string> currentPredicateIDs;

		for (std::string originalPredicateCode : originalPredicateCodes)
		{
			currentPredicateIDs = predicates[originalPredicateCode]->getPredicateIDs();

			for (int process : processes)
			{
				predicates[originalPredicateCode]->getAllReplacementVariants(process);
			}
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