#include "CubeTreeNode.h"
#include "Ast.h"

using namespace std;

CubeTreeNode::CubeTreeNode()
{
	stringRepresentation = string(config::globalPredicates.size(), config::CUBE_STATE_OMIT);
	parent = this;
}

CubeTreeNode::CubeTreeNode(std::string representation)
{
	assert(representation.size() == config::globalPredicates.size());

	stringRepresentation = representation;
	parent = this;
}

CubeTreeNode::CubeTreeNode(CubeTreeNode* source)
{
	CubeTreeNode* result = new CubeTreeNode(source->stringRepresentation);
	result->children = vector<CubeTreeNode*>(source->children);
}

void CubeTreeNode::cascadingPopulate(int sizeLimit)
{
	populate(sizeLimit);

	for (CubeTreeNode* child : children)
	{
		child->cascadingPopulate(sizeLimit);
	}
}

void CubeTreeNode::populate(int sizeLimit)
{
	if (config::globalVariables.size() - count(stringRepresentation.begin(), stringRepresentation.end(), config::CUBE_STATE_OMIT) -
		count(stringRepresentation.begin(), stringRepresentation.end(), config::CUBE_STATE_IGNORE) < sizeLimit)
	{
		assert(children.size() == 0);

		int numberOfPredicates = config::globalPredicates.size();
		int startIndex = numberOfPredicates - 1;
		string newRepresentation;

		while ((stringRepresentation[startIndex] == config::CUBE_STATE_OMIT || stringRepresentation[startIndex] == config::CUBE_STATE_IGNORE) &&
			startIndex >= 0)
		{
			startIndex--;
		}

		startIndex++;

		for (startIndex; startIndex < numberOfPredicates; startIndex++)
		{
			if (stringRepresentation[startIndex] != config::CUBE_STATE_IGNORE)
			{
				newRepresentation = string(stringRepresentation);

				newRepresentation[startIndex] = config::CUBE_STATE_DECIDED_TRUE;
				children.push_back(new CubeTreeNode(newRepresentation));

				newRepresentation[startIndex] = config::CUBE_STATE_DECIDED_FALSE;
				children.push_back(new CubeTreeNode(newRepresentation));

				newRepresentation[startIndex] = config::CUBE_STATE_OMIT;
			}
		}

		for (CubeTreeNode* child : children)
		{
			child->parent = this;
		}
	}
}

void CubeTreeNode::cascadingPrune(string implyingCubeRepresentation)
{
	if (!impliesPredicate)
	{
		prune(implyingCubeRepresentation);

		for (CubeTreeNode* child : children)
		{
			cascadingPrune(implyingCubeRepresentation);
		}
	}
}

void CubeTreeNode::prune(string implyingCubeRepresentation)
{
	if (!impliesPredicate)
	{
		for (int ctr = 0; ctr < children.size(); ctr++)
		{
			if (config::isSubset(implyingCubeRepresentation, children.at(ctr)->stringRepresentation))
			{
				children.at(ctr)->parent = NULL;
				children.erase(children.begin() + ctr);
				ctr--;
			}
		}
	}
}

void CubeTreeNode::cascadingCheckImplication(Ast* predicate)
{
	for (CubeTreeNode* child : children)
	{
		child->checkImplication(predicate);
	}

	for (CubeTreeNode* child : children)
	{
		child->cascadingCheckImplication(predicate);
	}
}

void CubeTreeNode::checkImplication(Ast* predicate)
{
	impliesPredicate = config::cubeImpliesPredicate(config::cubeStringRepresentationToAstRef(stringRepresentation), predicate);

	if (impliesPredicate)
	{
		for (CubeTreeNode* child : children)
		{
			child->parent = NULL;
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