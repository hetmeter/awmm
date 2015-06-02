#include "CubeTreeNode.h"
#include "config.h"
#include "Ast.h"

using namespace std;

/* Constructors and destructor */
	CubeTreeNode::CubeTreeNode(int upperLimit)
	{
		_upperLimit = upperLimit;
		_stringRepresentation = string(config::globalPredicatesCount, CUBE_STATE_OMIT);
		_varCount = 0;
		_firstUsableIndex = 0;
		_childrenCount = 2 * config::globalPredicatesCount;
		initializeChildren();
	}

	CubeTreeNode::CubeTreeNode(const string &stringRepresentation, int upperLimit, CubeTreeNode* parent)
	{
		_upperLimit = upperLimit;
		_stringRepresentation = string(stringRepresentation);
		_varCount = _stringRepresentation.size() - count(_stringRepresentation.begin(), _stringRepresentation.end(), CUBE_STATE_OMIT);
		_firstUsableIndex = 0;
		_parent = parent;

		for (int ctr = config::globalPredicatesCount - 1; ctr >= 0; ctr--)
		{
			if (_stringRepresentation[ctr] != CUBE_STATE_OMIT)
			{
				_firstUsableIndex = ctr + 1;
				break;
			}
		}

		_childrenCount = 2 * (config::globalPredicatesCount - _firstUsableIndex);
		initializeChildren();
	}
	
	CubeTreeNode::~CubeTreeNode()
	{
		for (CubeTreeNode* child : _children)
		{
			if (child != nullptr)
			{
				delete child;
			}
		}
	}

/* Public fields */
	CubeTreeNode* CubeTreeNode::getRoot()
	{
		if (_parent == nullptr)
		{
			return this;
		}

		return _parent->getRoot();
	}

	void CubeTreeNode::setImplication(const string &key, const Implication &value)
	{
		_predicateImplications[key] = value;
	}

	CubeTreeNode::Implication CubeTreeNode::getImplication(Ast* predicate)
	{
		string predicateCode = predicate->emitCode();

		if (!hasImplicationData(predicateCode))
		{
			if (config::getFalsePredicate()->isEquivalent(predicate))
			{
				calculateImplication(predicate);
			}
			else if (getImplication(config::getFalsePredicate()) == NOT_IMPLIES)
			{
				calculateImplication(predicate);
			}
			else
			{
				_predicateImplications[predicateCode] = NOT_IMPLIES;
			}
		}

		return _predicateImplications[predicateCode];
	}

	string CubeTreeNode::getStringRepresentation()
	{
		return _stringRepresentation;
	}

	int CubeTreeNode::getFirstUsableIndex()
	{
		return _firstUsableIndex;
	}

	int CubeTreeNode::getVarCount()
	{
		return _varCount;
	}

	int CubeTreeNode::getUpperLimit()
	{
		return _upperLimit;
	}

	CubeTreeNode* CubeTreeNode::getChild(const int &index)
	{
		if (index >= 0 && index < _childrenCount && _varCount < _upperLimit)
		{
			if (_children[index] == nullptr)
			{
				string representation = string(_stringRepresentation);

				if ((index / 2) * 2 == index)
				{
					representation[_firstUsableIndex + (index / 2)] = CUBE_STATE_DECIDED_FALSE;
				}
				else
				{
					representation[_firstUsableIndex + (index / 2)] = CUBE_STATE_DECIDED_TRUE;
				}

				_children[index] = new CubeTreeNode(representation, _upperLimit, this);

				cout << "Created " << _stringRepresentation << "[" << index << "] = " << representation << "\r";
			}

			return _children[index];
		}

		return nullptr;
	}

/* Public methods */
	vector<string> CubeTreeNode::getMinimalImplyingCubes(Ast* predicate, const vector<int> &relevantIndices)
	{
		vector<string> result;
		bool predicateIsFalse = config::getFalsePredicate()->isEquivalent(predicate);

		if (!predicateIsFalse && (config::isTautology(predicate) || config::isTautology(predicate->negate())))
		{
			return result;
		}

		vector<CubeTreeNode*> searchQueue;
		CubeTreeNode* currentNode;
		searchQueue.push_back(this);
		string predicateCode = predicate->emitCode();
		Implication currentImplication;
		int currentFirstUsableIndex;

		while (!searchQueue.empty())
		{
			currentNode = searchQueue.at(0);

			if (!currentNode->hasImplicationData(predicateCode))
			{
				if (predicateIsFalse)
				{
					currentNode->calculateImplication(predicate);
				}
				else if (currentNode->getImplication(config::getFalsePredicate()) == NOT_IMPLIES)
				{
					currentNode->calculateImplication(predicate);
				}
				else
				{
					currentNode->setImplication(predicateCode, NOT_IMPLIES);
				}
			}

			currentImplication = currentNode->getImplication(predicate);
			currentFirstUsableIndex = currentNode->getFirstUsableIndex();

			if (currentImplication == IMPLIES)
			{
				result.push_back(currentNode->getStringRepresentation());
			}
			else if (currentImplication == NOT_IMPLIES &&
				currentNode->getVarCount() < currentNode->getUpperLimit())
			{
				for (int relevantIndex : relevantIndices)
				{
					if (relevantIndex >= currentFirstUsableIndex)
					{
						searchQueue.push_back(currentNode->
							getChild(2 * (relevantIndex - currentFirstUsableIndex)));
						searchQueue.push_back(currentNode->
							getChild(2 * (relevantIndex - currentFirstUsableIndex) + 1));
					}
				}
			}

			searchQueue.erase(searchQueue.begin());
		}

		return result;
	}
	/*vector<string> CubeTreeNode::getMinimalImplyingCubes(Ast* predicate, const vector<int> &relevantIndices)
	{
		vector<string> result;
		Implication currentImplication = getImplication(predicate);

		if (currentImplication == IMPLIES)
		{
			result.push_back(_stringRepresentation);
		}
		else if (currentImplication == NOT_IMPLIES && _varCount < _upperLimit)
		{
			vector<string> subResult;

			for (int relevantIndex : relevantIndices)
			{
				if (relevantIndex >= _firstUsableIndex)
				{
					subResult = getChild(2 * (relevantIndex - _firstUsableIndex))->
						getMinimalImplyingCubes(predicate, relevantIndices);
					result.insert(result.end(), subResult.begin(), subResult.end());

					subResult = getChild(2 * (relevantIndex - _firstUsableIndex) + 1)->
						getMinimalImplyingCubes(predicate, relevantIndices);
					result.insert(result.end(), subResult.begin(), subResult.end());
				}
			}
		}

		return result;
	}*/

	vector<string> CubeTreeNode::getAllFalseImplyingCubes()
	{
		vector<string> result;
		Implication currentImplication = getImplication(config::getFalsePredicate());

		if (currentImplication == IMPLIES || currentImplication == SUPERSET_IMPLIES)
		{
			vector<string> subResult = getCurrentSubtreeRepresentations();
			result.insert(result.end(), subResult.begin(), subResult.end());
		}
		else if (currentImplication == NOT_IMPLIES && _varCount < _upperLimit)
		{
			vector<string> subResult;

			for (int ctr = 0; ctr < _childrenCount; ctr++)
			{
				subResult = getChild(ctr)->getAllFalseImplyingCubes();
				result.insert(result.end(), subResult.begin(), subResult.end());
			}
		}

		return result;
	}

	void CubeTreeNode::reportImplication(const string &representation, Ast* predicate)
	{
		if (!hasImplicationData(predicate->emitCode()) || getImplication(predicate) != SUPERSET_IMPLIES)
		{
			if (isProperSubset(representation))
			{
				setSubtreeImplication(predicate, SUPERSET_IMPLIES);

				cout << "Set " << _stringRepresentation << "[" << predicate->emitCode() << "] = SUPERSET_IMPLIES\t\t\t \r";
			}
			else if (_varCount < _upperLimit)
			{
				for (int ctr = 0; ctr < _childrenCount; ctr++)
				{
					getChild(ctr)->reportImplication(representation, predicate);
				}
			}
		}
	}

	void CubeTreeNode::setSubtreeImplication(Ast* predicate, const Implication &implicationValue)
	{
		_predicateImplications[predicate->emitCode()] = implicationValue;

		if (_varCount < _upperLimit)
		{
			for (int ctr = 0; ctr < _childrenCount; ctr++)
			{
				getChild(ctr)->setSubtreeImplication(predicate, implicationValue);
			}
		}
	}

	bool CubeTreeNode::isProperSubset(const string &possibleSubset)
	{
		int numberOfPredicates = config::globalPredicates.size();

		int possibleSubsetOmitCount = std::count(possibleSubset.begin(), possibleSubset.end(), CUBE_STATE_OMIT);
		int possibleSupersetOmitCount = std::count(_stringRepresentation.begin(), _stringRepresentation.end(), CUBE_STATE_OMIT);

		if (possibleSubsetOmitCount == possibleSubset.size() || possibleSupersetOmitCount == config::globalPredicatesCount)
		{
			return false;
		}

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			if (possibleSubset[ctr] != CUBE_STATE_OMIT && possibleSubset[ctr] != _stringRepresentation[ctr])
			{
				return false;
			}
		}

		return possibleSubset != _stringRepresentation;
	}

	bool CubeTreeNode::hasImplicationData(const string &predicateCode)
	{
		return _predicateImplications.find(predicateCode) != _predicateImplications.end();
	}

/* Private fields */

	vector<Ast*> CubeTreeNode::getAstRepresentation()
	{
		if (_astRepresentation.empty())
		{
			Ast* currentTerm;

			for (int ctr = 0; ctr < config::globalPredicatesCount; ctr++)
			{
				if (_stringRepresentation[ctr] == CUBE_STATE_DECIDED_FALSE)
				{
					currentTerm = config::globalPredicates[ctr]->negate();
					_astRepresentation.push_back(currentTerm);
				}
				else if (_stringRepresentation[ctr] == CUBE_STATE_DECIDED_TRUE)
				{
					currentTerm = config::globalPredicates[ctr]->clone();
					_astRepresentation.push_back(currentTerm);
				}
			}
		}

		return _astRepresentation;
	}

	vector<string> CubeTreeNode::getCurrentSubtreeRepresentations()
	{
		vector<string> result;

		result.push_back(_stringRepresentation);

		if (_varCount < _upperLimit)
		{
			vector<string> subResult;

			for (int ctr = 0; ctr < _childrenCount; ctr++)
			{
				subResult = getChild(ctr)->getCurrentSubtreeRepresentations();
				result.insert(result.end(), subResult.begin(), subResult.end());
			}
		}

		return result;
	}

/* Private methods */
	void CubeTreeNode::initializeChildren()
	{
		assert(_children.empty());

		if (_varCount < _upperLimit)
		{
			for (int ctr = 0; ctr < _childrenCount; ctr++)
			{
				_children.push_back(nullptr);
			}
		}
	}

	void CubeTreeNode::calculateImplication(Ast* predicate)
	{
		string predicateCode = predicate->emitCode();

		if (config::cubeImpliesPredicate(getAstRepresentation(), predicate))
		{
			_predicateImplications[predicateCode] = IMPLIES;

			cout << "\t\t\t\t\tSet " << _stringRepresentation << "[" << predicateCode << "] = IMPLIES\t\t\t \n";

			getRoot()->reportImplication(_stringRepresentation, predicate);
		}
		else
		{
			_predicateImplications[predicateCode] = NOT_IMPLIES;

			cout << "Set " << _stringRepresentation << "[" << predicateCode << "] = NOT_IMPLIES\t\t\t \r";
		}
	}