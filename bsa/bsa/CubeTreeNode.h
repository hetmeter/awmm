#pragma once

#include <string>
#include <algorithm>

#include "config.h"

class Ast;

class CubeTreeNode
{
public:
	CubeTreeNode();
	CubeTreeNode(std::string representation);
	CubeTreeNode(CubeTreeNode* source);

	~CubeTreeNode();

	std::string stringRepresentation;
	std::vector<CubeTreeNode*> children;
	CubeTreeNode* parent;
	bool impliesPredicate = false;

	void cascadingPopulate(int sizeLimit);
	void cascadingCheckImplication(Ast* predicate);

	std::vector<CubeTreeNode*> getImplyingCubes();

private:
	void populate(int sizeLimit);
	void cascadingPrune(std::string implyingCubeRepresentation);
	void prune(std::string implyingCubeRepresentation);
	void checkImplication(Ast* predicate);
};