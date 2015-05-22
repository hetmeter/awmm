#include "CubeTreeNode.h"
#include "config.h"
#include "Ast.h"

using namespace std;

/* Constructors and destructor */

	CubeTreeNode::CubeTreeNode(const string &representation)
	{
		assert(representation.size() == config::globalPredicates.size());

		stringRepresentation = representation;
		parent = this;
	}

	CubeTreeNode::~CubeTreeNode()
	{

	}

/* Access */

	string CubeTreeNode::toString()
	{
		string result = stringRepresentation + (impliesPredicate ? "\t- implying" : "") + (toBeDeleted ? "\t- to be deleted" : "");
		string subResult = "";

		for (CubeTreeNode* child : children)
		{
			subResult += "\n" + child->toString();
		}

		regex newLineRegex("\\n");
		smatch stringMatch;

		subResult = regex_replace(subResult, newLineRegex, "\n|");

		return result + subResult;
	}

	void CubeTreeNode::cull()
	{
		for (CubeTreeNode* child : children)
		{
			child->cull();
		}

		for (CubeTreeNode* child : children)
		{
			delete child;
		}

		children.clear();
		parent = nullptr;
	}

/* Cascading methods */
	
	void CubeTreeNode::cascadingPopulate(int sizeLimit)
	{
		populate(sizeLimit);
	
		for (CubeTreeNode* child : children)
		{
			if (!child->toBeDeleted)
			{
				child->cascadingPopulate(sizeLimit);
			}
		}
	}
	
	void CubeTreeNode::cascadingCheckImplication(Ast* predicate)
	{
		for (CubeTreeNode* child : children)
		{
			if (!child->toBeDeleted)
			{
				child->checkImplication(predicate);
			}
		}
	
		for (CubeTreeNode* child : children)
		{
			if (!child->toBeDeleted)
			{
				child->cascadingCheckImplication(predicate);
			}
		}
	}
	
	void CubeTreeNode::cascadingPrune(const string &implyingCubeRepresentation)
	{
		if (!impliesPredicate)
		{
			prune(implyingCubeRepresentation);
	
			for (CubeTreeNode* child : children)
			{
				if (!child->toBeDeleted && !child->impliesPredicate)
				{
					child->cascadingPrune(implyingCubeRepresentation);
				}
			}
		}
	}

	vector<CubeTreeNode*> CubeTreeNode::getImplyingCubes()
	{
		vector<CubeTreeNode*> result;
		vector<CubeTreeNode*> subResult;

		if (impliesPredicate)
		{
			result.push_back(this);
		}

		for (CubeTreeNode* child : children)
		{
			subResult = child->getImplyingCubes();
			result.insert(result.end(), subResult.begin(), subResult.end());
		}

		return result;
	}

	void CubeTreeNode::scour()
	{
		for (int ctr = 0; ctr < children.size(); ctr++)
		{
			if (children.at(ctr)->toBeDeleted)
			{
				children.at(ctr)->cull();
				delete children.at(ctr);
				children.erase(children.begin() + ctr);
				ctr--;
			}
			else
			{
				children.at(ctr)->scour();
			}
		}
	}

/* Static methods */

	std::string CubeTreeNode::getCubeStatePool(int predicateIndex)
	{
		std::string result = std::string(config::globalPredicates.size(), CUBE_STATE_OMIT);
		result[predicateIndex] = CUBE_STATE_UNDECIDED;
		return result;
	}

	string CubeTreeNode::getCubeStatePool(const vector<int> &predicateIndices)
	{
		string result = string(config::globalPredicates.size(), CUBE_STATE_OMIT);
	
		for (int predicateIndex : predicateIndices)
		{
			result[predicateIndex] = CUBE_STATE_UNDECIDED;
		}
	
		return result;
	}

	vector<string> CubeTreeNode::getNaryCubeStateCombinations(const vector<int> &predicateIndices, int n)
	{
		int numberOfPredicates = config::globalPredicates.size();
		string emptyCube = std::string(numberOfPredicates, CUBE_STATE_OMIT);
		string currentCube;
		vector<string> result;

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
			vector<vector<int>> cubeIndexSet = config::intSetCartesianProduct(predicateIndices, predicateIndices);

			for (int ctr = 2; ctr < n; ctr++)
			{
				cubeIndexSet = config::intSetCartesianProduct(cubeIndexSet, predicateIndices);
			}

			for (std::vector<int> cubeIndices : cubeIndexSet)
			{
				currentCube = getCubeStatePool(cubeIndices);
				result.push_back(currentCube);
			}
		}

		return result;
	}

	std::vector<std::string> CubeTreeNode::getImplicativeCubeStates(const string &pool, Ast* predicate)
	{
		int numberOfPredicates = config::globalPredicates.size();
		bool fullyDefined = true;
		std::vector<std::string> result;
		std::vector<std::string> subResult;
		std::string poolCopy;

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			//std::cout << "\t" << ctr << " / " << numberOfPredicates << "\n";

			if (pool[ctr] == CUBE_STATE_UNDECIDED || pool[ctr] == CUBE_STATE_MAY_BE_FALSE)
			{
				poolCopy = std::string(pool);
				poolCopy[ctr] = CUBE_STATE_DECIDED_FALSE;

				subResult = getImplicativeCubeStates(poolCopy, predicate);
				result.insert(result.end(), subResult.begin(), subResult.end());

				/*if (subResult.size() > 0)
				{
					std::cout << "\tPool " << pool << " threw\n";

					for (std::string temp : subResult)
					{
						std::cout << "\t\t" << temp << "\n";
					}
				}*/
				
				fullyDefined = false;
			}

			if (pool[ctr] == CUBE_STATE_UNDECIDED || pool[ctr] == CUBE_STATE_MAY_BE_TRUE)
			{
				poolCopy = std::string(pool);
				poolCopy[ctr] = CUBE_STATE_DECIDED_TRUE;

				subResult = getImplicativeCubeStates(poolCopy, predicate);

				result.insert(result.end(), subResult.begin(), subResult.end());

				/*if (subResult.size() > 0)
				{
					std::cout << "\tPool " << pool << " threw\n";

					for (std::string temp : subResult)
					{
						std::cout << "\t\t" << temp << "\n";
					}
				}*/

				fullyDefined = false;
			}

			if (!fullyDefined)
			{
				break;
			}
		}

		if (fullyDefined)
		{
			if (config::cubeImpliesPredicate(toAstRef(pool), predicate))
			{
				result.push_back(pool);
			}
		}

		return result;
	}

	std::string CubeTreeNode::applyDecisionMask(const string &pool, const string &decisionMask)
	{
		int numberOfPredicates = config::globalPredicates.size();
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

	std::string CubeTreeNode::removeDecisionsFromPool(const string &pool, const vector<string> &decisions)
	{
		int numberOfPredicates = config::globalPredicates.size();
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
	
	std::vector<Ast*> CubeTreeNode::toAstRef(const string &pool)
	{
		int numberOfPredicates = config::globalPredicates.size();
		std::vector<Ast*> cubeTerms;
		Ast* currentTerm;
	
		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			if (pool[ctr] == CUBE_STATE_DECIDED_FALSE)
			{
				currentTerm = config::globalPredicates[ctr]->negate();
				cubeTerms.push_back(currentTerm);
			}
			else if (pool[ctr] == CUBE_STATE_DECIDED_TRUE)
			{
				currentTerm = config::globalPredicates[ctr]->clone();
				cubeTerms.push_back(currentTerm);
			}
		}
	
		return cubeTerms;
	}
	
	bool CubeTreeNode::isSubset(const string &possibleSubset, const string &possibleSuperset)
	{
		int numberOfPredicates = config::globalPredicates.size();
	
		int possibleSubsetOmitCount = std::count(possibleSubset.begin(), possibleSubset.end(), CUBE_STATE_OMIT);
		int possibleSubsetIgnoreCount = std::count(possibleSubset.begin(), possibleSubset.end(), CUBE_STATE_IGNORE);
		int possibleSupersetOmitCount = std::count(possibleSuperset.begin(), possibleSuperset.end(), CUBE_STATE_OMIT);
		int possibleSupersetIgnoreCount = std::count(possibleSuperset.begin(), possibleSuperset.end(), CUBE_STATE_IGNORE);
	
		if (possibleSubsetOmitCount + possibleSubsetIgnoreCount == possibleSubset.size() ||
			possibleSupersetOmitCount + possibleSupersetIgnoreCount == possibleSuperset.size())
		{
			return false;
		}
	
		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			if (possibleSubset[ctr] != CUBE_STATE_OMIT && possibleSubset[ctr] != CUBE_STATE_IGNORE &&
				possibleSubset[ctr] != possibleSuperset[ctr])
			{
				return false;
			}
		}
	
		return true;
	}

/* Private methods */
	
	void CubeTreeNode::populate(int sizeLimit)
	{
		if (!toBeDeleted)
		{
			int numberOfPredicates = config::globalPredicates.size();
			int numberOfOmits = count(stringRepresentation.begin(), stringRepresentation.end(), CUBE_STATE_OMIT);
			int numberOfIgnores = count(stringRepresentation.begin(), stringRepresentation.end(), CUBE_STATE_IGNORE);

			if (numberOfPredicates - numberOfOmits - numberOfIgnores < sizeLimit)
			{
				assert(children.size() == 0);

				int startIndex = numberOfPredicates - 1;
				string newRepresentation;

				while (startIndex >= 0 &&
					(stringRepresentation[startIndex] == CUBE_STATE_OMIT || stringRepresentation[startIndex] == CUBE_STATE_IGNORE))
				{
					startIndex--;
				}

				startIndex++;

				for (startIndex; startIndex < numberOfPredicates; startIndex++)
				{
					if (stringRepresentation[startIndex] != CUBE_STATE_IGNORE)
					{
						newRepresentation = string(stringRepresentation);

						newRepresentation[startIndex] = CUBE_STATE_DECIDED_TRUE;
						children.push_back(new CubeTreeNode(newRepresentation));

						newRepresentation[startIndex] = CUBE_STATE_DECIDED_FALSE;
						children.push_back(new CubeTreeNode(newRepresentation));

						newRepresentation[startIndex] = CUBE_STATE_OMIT;
					}
				}

				for (CubeTreeNode* child : children)
				{
					child->parent = this;
				}
			}
		}
	}
	
	void CubeTreeNode::prune(const string &implyingCubeRepresentation)
	{
		//cout << "\tPruned : " << stringRepresentation << "\n";
		if (!impliesPredicate && !toBeDeleted)
		{
			for (int ctr = 0; ctr < children.size(); ctr++)
			{
				if (isSubset(implyingCubeRepresentation, children.at(ctr)->stringRepresentation))
				{
					/*children.at(ctr)->parent = nullptr;
					children.erase(children.begin() + ctr);
					ctr--;*/

					children.at(ctr)->toBeDeleted = true;
				}
			}
		}
	}

	void CubeTreeNode::checkImplication(Ast* predicate)
	{
		if (!toBeDeleted && !impliesPredicate)
		{
			impliesPredicate = config::cubeImpliesPredicate(toAstRef(stringRepresentation), predicate);

			if (impliesPredicate)
			{
				for (CubeTreeNode* child : children)
				{
					child->parent = nullptr;
				}

				children.clear();

				CubeTreeNode* possibleRoot = parent;

				while (possibleRoot->parent != possibleRoot)
				{
					possibleRoot = possibleRoot->parent;
				}

				possibleRoot->cascadingPrune(stringRepresentation);
			}
		}
	}

//CubeTreeNode::CubeTreeNode()
//{
//	stringRepresentation = string(config::globalPredicates.size(), config::CUBE_STATE_OMIT);
//	parent = this;
//}
//
//CubeTreeNode::CubeTreeNode(CubeTreeNode* source)
//{
//	CubeTreeNode* result = new CubeTreeNode(source->stringRepresentation);
//	result->children = vector<CubeTreeNode*>(source->children);
//}