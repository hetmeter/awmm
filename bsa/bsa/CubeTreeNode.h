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
	CubeTreeNode(const std::string &representation);
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
	bool toBeDeleted = false;

/* Access */
	std::string toString();
	void cull();

/* Cascading methods */
	void cascadingPopulate(int sizeLimit);
	void cascadingCheckImplication(Ast* predicate);
	void cascadingPrune(const std::string &implyingCubeRepresentation);
	std::vector<CubeTreeNode*> getImplyingCubes();
	void scour();

/* Static methods */
	static std::string getCubeStatePool(int predicateIndex);
	static std::string getCubeStatePool(const std::vector<int> &predicateIndices);
	static std::vector<std::string> getNaryCubeStateCombinations(const std::vector<int> &predicateIndices, int n);
	static std::vector<std::string> getImplicativeCubeStates(const std::string &pool, Ast* predicate);
	static std::string applyDecisionMask(const std::string &pool, const std::string &decisionMask);
	static std::string removeDecisionsFromPool(const std::string &pool, const std::vector<std::string> &decisions);
	static std::vector<Ast*> toAstRef(const std::string &pool);
	static bool isSubset(const std::string &possibleSubset, const std::string &possibleSuperset);

private:
	void populate(int sizeLimit);
	void prune(const std::string &implyingCubeRepresentation);
	void checkImplication(Ast* predicate);
};