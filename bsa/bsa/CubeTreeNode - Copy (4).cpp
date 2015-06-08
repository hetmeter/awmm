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
			}
		}

		config::implicativeCubes.insert(pair<string, CubeTreeNode*>(_stringRepresentation, this));
	}

	CubeTreeNode::~CubeTreeNode()
	{}

/* Public fields */
	CubeTreeNode::Implication CubeTreeNode::getPredicateImplication(Ast* predicate)
	{
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

				const vector<string> supersetStringRepresentations = getSupersetStringRepresentations();

				for (string supersetStringRepresentation : supersetStringRepresentations)
				{
					config::getImplicativeCube(supersetStringRepresentation)->setPredicateImplication(predicateCode, SUPERSET_IMPLIES);
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
		_predicateImplications.insert(pair<string, Implication>(predicateCode, predicateImplication));

		if (predicateImplication == SUPERSET_IMPLIES)
		{
			const vector<string> supersetStringRepresentations = getSupersetStringRepresentations();

			for (string supersetStringRepresentation : supersetStringRepresentations)
			{
				config::getImplicativeCube(supersetStringRepresentation)->setPredicateImplication(predicateCode, SUPERSET_IMPLIES);
			}
		}
	}

	vector<string> CubeTreeNode::getSupersetStringRepresentations()
	{
		if (_supersetStringRepresentations.empty() && _varCount < _upperLimit)
		{
			string stringRepresentationCopy = string(_stringRepresentation);

			for (int ctr = 0; ctr < config::globalPredicatesCount; ctr++)
			{
				if (stringRepresentationCopy[ctr] == CUBE_STATE_OMIT)
				{
					stringRepresentationCopy[ctr] = CUBE_STATE_DECIDED_FALSE;
					_supersetStringRepresentations.push_back(stringRepresentationCopy);
					stringRepresentationCopy[ctr] = CUBE_STATE_DECIDED_TRUE;
					_supersetStringRepresentations.push_back(stringRepresentationCopy);
					stringRepresentationCopy[ctr] = CUBE_STATE_OMIT;
				}
			}
		}

		return _supersetStringRepresentations;
	}

	vector<string> CubeTreeNode::getCanonicalSupersetStringRepresentations()
	{
		if (_canonicalSupersetStringRepresentations.empty() && _varCount < _upperLimit && _suffixIndex < config::globalPredicatesCount)
		{
			string stringRepresentationCopy = string(_stringRepresentation);

			for (int ctr = _suffixIndex; ctr < config::globalPredicatesCount; ctr++)
			{
				stringRepresentationCopy[ctr] = CUBE_STATE_DECIDED_FALSE;
				_canonicalSupersetStringRepresentations.push_back(stringRepresentationCopy);
				stringRepresentationCopy[ctr] = CUBE_STATE_DECIDED_TRUE;
				_canonicalSupersetStringRepresentations.push_back(stringRepresentationCopy);
				stringRepresentationCopy[ctr] = CUBE_STATE_OMIT;
			}
		}

		return _canonicalSupersetStringRepresentations;
	}

/* Private fields */
	vector<Ast*> CubeTreeNode::getAstVectorRepresentation()
	{
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