#include "CubeTreeNode.h"
#include "config.h"
#include "Ast.h"

using namespace std;

/* Constructors and destructor */
	CubeTreeNode::CubeTreeNode(int upperLimit)
	{
		_stringRepresentation = string(config::globalPredicatesCount, CUBE_STATE_OMIT);
		_upperLimit = upperLimit;
		_varCount = 0;
		_suffixIndex = 0;

		config::implicativeCubes.insert(pair<string, CubeTreeNode*>(_stringRepresentation, this));
	}

	CubeTreeNode::CubeTreeNode(const string &stringRepresentation, int upperLimit)
	{
		_stringRepresentation = stringRepresentation;
		_upperLimit = upperLimit;
		_varCount = _stringRepresentation.size() - count(_stringRepresentation.begin(), _stringRepresentation.end(), CUBE_STATE_OMIT);
		_suffixIndex = 0;
		
		for (int ctr = config::globalPredicatesCount - 1; ctr >= 0; ctr--)
		{
			if (_stringRepresentation[ctr] != CUBE_STATE_OMIT)
			{
				_suffixIndex = ctr + 1;
				break;
			}
		}

		config::implicativeCubes.insert(pair<string, CubeTreeNode*>(_stringRepresentation, this));
	}

	CubeTreeNode::~CubeTreeNode()
	{}

/* Public fields */
	CubeTreeNode::Implication CubeTreeNode::getPredicateImplication(Ast* predicate, const vector<int> &relevantIndices)
	{
		//cout << "getPredicateImplication(" << predicate->getCode() << ", relevantIndices |" << relevantIndices.size() << "|)\t\t\t \n";
		//cout << "getPredicateImplication(" << predicate->getCode() << ", relevantIndices |" << relevantIndices.size() << "|)\t\t\t \r";

		if (_varCount == 0)
		{
			return NOT_IMPLIES;
		}

		const string predicateCode = predicate->getCode();

		if (_predicateImplications.find(predicateCode) == _predicateImplications.end())
		{
			if (config::cubeImpliesPredicate(getAstVectorRepresentation(), predicate))
			{
				setPredicateImplication(predicateCode, IMPLIES);

				const vector<string> supersetStringRepresentations = getSupersetStringRepresentations(relevantIndices);

				for (string supersetStringRepresentation : supersetStringRepresentations)
				{
					config::getImplicativeCube(supersetStringRepresentation)->setPredicateImplication(predicateCode, relevantIndices);
				}
			}
			else
			{
				setPredicateImplication(predicateCode, NOT_IMPLIES);
			}
		}

		return _predicateImplications.at(predicateCode);
	}

	void CubeTreeNode::setPredicateImplication(const string &predicateCode, CubeTreeNode::Implication predicateImplication)
	{
		//cout << "setPredicateImplication(" << predicateCode << ", relevantIndices |" << predicateImplication << "|)\t\t\t \n";
		//cout << "setPredicateImplication(" << predicateCode << ", relevantIndices |" << predicateImplication << "|)\t\t\t \r";

		assert(predicateImplication != SUPERSET_IMPLIES);
		_predicateImplications.insert(pair<string, Implication>(predicateCode, predicateImplication));
	}

	void CubeTreeNode::setPredicateImplication(const string &predicateCode, const vector<int> &relevantIndices)
	{
		//cout << "setPredicateImplication(" << predicateCode << ", relevantIndices |" << relevantIndices.size() << "|)\t\t\t \n";
		//cout << "setPredicateImplication(" << predicateCode << ", relevantIndices |" << relevantIndices.size() << "|)\t\t\t \r";

		_predicateImplications.insert(pair<string, Implication>(predicateCode, SUPERSET_IMPLIES));

		const vector<string> supersetStringRepresentations = getSupersetStringRepresentations(relevantIndices);

		for (string supersetStringRepresentation : supersetStringRepresentations)
		{
			config::getImplicativeCube(supersetStringRepresentation)->setPredicateImplication(predicateCode, relevantIndices);
		}
	}

	vector<string> CubeTreeNode::getSupersetStringRepresentations(const vector<int> &relevantIndices)
	{
		//cout << "getSupersetStringRepresentations(relevantIndices |" << relevantIndices.size() << "|)\t\t\t \n";
		//cout << "getSupersetStringRepresentations(relevantIndices |" << relevantIndices.size() << "|)\t\t\t \r";

		if (relevantIndices.empty())
		{
			return vector<string>();
		}

		if (_supersetStringRepresentations.find(relevantIndices) == _supersetStringRepresentations.end())
		{
			_supersetStringRepresentations.insert(pair<vector<int>, vector<string>>(relevantIndices, vector<string>()));
		}

		if (_supersetStringRepresentations.at(relevantIndices).empty() && _varCount < _upperLimit)
		{
			string stringRepresentationCopy = string(_stringRepresentation);

			for (int relevantIndex : relevantIndices)
			{
				if (stringRepresentationCopy[relevantIndex] == CUBE_STATE_OMIT)
				{
					stringRepresentationCopy[relevantIndex] = CUBE_STATE_DECIDED_FALSE;
					_supersetStringRepresentations.at(relevantIndices).push_back(stringRepresentationCopy);
					stringRepresentationCopy[relevantIndex] = CUBE_STATE_DECIDED_TRUE;
					_supersetStringRepresentations.at(relevantIndices).push_back(stringRepresentationCopy);
					stringRepresentationCopy[relevantIndex] = CUBE_STATE_OMIT;
				}
			}
		}

		return _supersetStringRepresentations.at(relevantIndices);
	}

	vector<string> CubeTreeNode::getCanonicalSupersetStringRepresentations(const vector<int> &relevantIndices)
	{
		//cout << _stringRepresentation << ".getCanonicalSupersetStringRepresentations(relevantIndices |" << relevantIndices.size() << "|)\n";
		//cout << _stringRepresentation << ".getCanonicalSupersetStringRepresentations(relevantIndices |" << relevantIndices.size() << "|)\t\t\t \r";

		if (relevantIndices.empty())
		{
			return vector<string>();
		}

		if (_canonicalSupersetStringRepresentations.find(relevantIndices) == _canonicalSupersetStringRepresentations.end())
		{
			_canonicalSupersetStringRepresentations.insert(pair<vector<int>, vector<string>>(relevantIndices, vector<string>()));
		}

		if (_canonicalSupersetStringRepresentations.at(relevantIndices).empty() &&
			_varCount < _upperLimit && _suffixIndex < config::globalPredicatesCount)
		{
			string stringRepresentationCopy = string(_stringRepresentation);

			for (int relevantIndex : relevantIndices)
			{
				if (relevantIndex >= _suffixIndex)
				{
					stringRepresentationCopy[relevantIndex] = CUBE_STATE_DECIDED_FALSE;
					_canonicalSupersetStringRepresentations.at(relevantIndices).push_back(stringRepresentationCopy);
					stringRepresentationCopy[relevantIndex] = CUBE_STATE_DECIDED_TRUE;
					_canonicalSupersetStringRepresentations.at(relevantIndices).push_back(stringRepresentationCopy);
					stringRepresentationCopy[relevantIndex] = CUBE_STATE_OMIT;
				}
			}
		}

		return _canonicalSupersetStringRepresentations.at(relevantIndices);
	}

/* Private fields */
	vector<Ast*> CubeTreeNode::getAstVectorRepresentation()
	{
		//cout << "getAstVectorRepresentation()\t\t\t \r";
		//cout << "getAstVectorRepresentation()\t\t\t \n";

		if (_astVectorRepresentation.empty() && _varCount < _upperLimit)
		{
			Ast* currentTerm;

			for (int ctr = 0; ctr < config::globalPredicatesCount; ctr++)
			{
				if (_stringRepresentation[ctr] == CUBE_STATE_DECIDED_FALSE)
				{
					currentTerm = config::globalPredicates[ctr]->negate();
					_astVectorRepresentation.push_back(currentTerm);
				}
				else if (_stringRepresentation[ctr] == CUBE_STATE_DECIDED_TRUE)
				{
					currentTerm = config::globalPredicates[ctr]->clone();
					_astVectorRepresentation.push_back(currentTerm);
				}
			}
		}

		return _astVectorRepresentation;
	}