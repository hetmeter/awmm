#pragma once

#include <string>
#include <vector>

class Ast;

class CubeTreeNode
{
public:

//	CubeTreeNode();
//	CubeTreeNode(CubeTreeNode* source);

/* Constructors and destructor */
	CubeTreeNode(std::string representation);
	~CubeTreeNode();

/* Constants */
	static const char CUBE_STATE_OMIT = '-';
	static const char CUBE_STATE_IGNORE = 'X';
	static const char CUBE_STATE_DECIDED_FALSE = 'F';
	static const char CUBE_STATE_DECIDED_TRUE = 'T';
	static const char CUBE_STATE_UNDECIDED = '?';
	static const char CUBE_STATE_MAY_BE_FALSE = 'f';
	static const char CUBE_STATE_MAY_BE_TRUE = 't';

/* Attributes */
	std::string stringRepresentation;
	CubeTreeNode* parent;
	std::vector<CubeTreeNode*> children;
	bool impliesPredicate = false;

/* Access */
	std::string toString();

/* Cascading methods */
	void cascadingPopulate(int sizeLimit);
	void cascadingCheckImplication(Ast* predicate);
	void cascadingPrune(std::string implyingCubeRepresentation);
	std::vector<CubeTreeNode*> getImplyingCubes();

/* Static methods */
	static std::string getCubeStatePool(int predicateIndex);
	static std::string getCubeStatePool(std::vector<int> predicateIndices);
	static std::vector<std::string> getNaryCubeStateCombinations(std::vector<int> predicateIndices, int n);
	static std::vector<std::string> getImplicativeCubeStates(std::string pool, Ast* predicate);
	static std::string applyDecisionMask(std::string pool, std::string decisionMask);
	static std::string removeDecisionsFromPool(std::string pool, std::vector<std::string> decisions);
	static std::vector<Ast*> toAstRef(std::string pool);
	static bool isSubset(std::string possibleSubset, std::string possibleSuperset);

private:
	void populate(int sizeLimit);
	void prune(std::string implyingCubeRepresentation);
	void checkImplication(Ast* predicate);
};