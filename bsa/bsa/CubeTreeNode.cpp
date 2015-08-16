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

#include "CubeTreeNode.h"
#include "Ast.h"
#include "config.h"
#include "PredicateData.h"

using namespace std;

/* Constructors and destructor */

	CubeTreeNode::CubeTreeNode(const string &stringRepresentation, int upperLimit)
	{
		_stringRepresentation = stringRepresentation;
		_upperLimit = upperLimit;
		_varCount = _stringRepresentation.size() - count(_stringRepresentation.begin(), _stringRepresentation.end(), CUBE_STATE_OMIT);
		_suffixIndex = 0;
		
		for (int ctr = config::originalPredicatesCount - 1; ctr >= 0; ctr--)
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

	vector<string> CubeTreeNode::getCanonicalSupersetStringRepresentations(const vector<int> &relevantIndices)
	{
		if (relevantIndices.empty())
		{
			return vector<string>();
		}

		if (_canonicalSupersetStringRepresentations.find(relevantIndices) == _canonicalSupersetStringRepresentations.end())
		{
			_canonicalSupersetStringRepresentations.insert(pair<vector<int>, vector<string>>(relevantIndices, vector<string>()));
		}

		if (_canonicalSupersetStringRepresentations.at(relevantIndices).empty() &&
			_varCount < _upperLimit && _suffixIndex < config::originalPredicatesCount)
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

	void CubeTreeNode::setPredicateImplication(const string &predicateCode, CubeTreeNode::Implication predicateImplication)
	{
		assert(predicateImplication != SUPERSET_IMPLIES);
		_predicateImplications.insert(pair<string, Implication>(predicateCode, predicateImplication));
	}

	void CubeTreeNode::setPredicateImplication(const string &predicateCode, const vector<int> &relevantIndices)
	{
		_predicateImplications.insert(pair<string, Implication>(predicateCode, SUPERSET_IMPLIES));

		const vector<string> supersetStringRepresentations = getSupersetStringRepresentations(relevantIndices);

		for (string supersetStringRepresentation : supersetStringRepresentations)
		{
			config::getImplicativeCube(supersetStringRepresentation)->setPredicateImplication(predicateCode, relevantIndices);
		}
	}

	vector<Ast*> CubeTreeNode::getAstVectorRepresentation()
	{
		if (_astVectorRepresentation.empty() && _varCount < _upperLimit)
		{
			Ast* currentTerm;

			for (int ctr = 0; ctr < config::originalPredicatesCount; ctr++)
			{
				if (_stringRepresentation[ctr] == CUBE_STATE_DECIDED_FALSE)
				{
					currentTerm = config::predicates[config::originalPredicateCodes[ctr]]->getPredicateAst()->negate();
					_astVectorRepresentation.push_back(currentTerm);
				}
				else if (_stringRepresentation[ctr] == CUBE_STATE_DECIDED_TRUE)
				{
					currentTerm = config::predicates[config::originalPredicateCodes[ctr]]->getPredicateAst()->clone();
					_astVectorRepresentation.push_back(currentTerm);
				}
			}
		}

		return _astVectorRepresentation;
	}

	vector<string> CubeTreeNode::getSupersetStringRepresentations(const vector<int> &relevantIndices)
	{
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