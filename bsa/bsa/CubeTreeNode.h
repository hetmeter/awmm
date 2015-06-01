#pragma once

#include <string>
#include <map>
#include <vector>

class Ast;

class CubeTreeNode
{
public:

/* Constructors and destructor */
	CubeTreeNode(int upperLimit);
	CubeTreeNode(const std::string &stringRepresentation, int upperLimit, CubeTreeNode* parent);
	~CubeTreeNode();

/* Constants */
	static const char CUBE_STATE_OMIT = '-';
	static const char CUBE_STATE_DECIDED_FALSE = 'F';
	static const char CUBE_STATE_DECIDED_TRUE = 'T';

/* Enumerations */
	enum Implication { IMPLIES, NOT_IMPLIES, SUPERSET_IMPLIES };

/* Public fields */
	CubeTreeNode* getRoot();
	void setImplication(const std::string &key, const Implication &value);
	Implication getImplication(Ast* predicate);
	std::string getStringRepresentation();
	int getFirstUsableIndex();
	int getVarCount();
	int getUpperLimit();
	CubeTreeNode* getChild(const int &index);

/* Public methods */
	std::vector<std::string> getMinimalImplyingCubes(Ast* predicate, const std::vector<int> &relevantIndices);
	std::vector<std::string> getAllFalseImplyingCubes();
	void reportImplication(const std::string &representation, Ast* predicate);
	void setSubtreeImplication(Ast* predicate, const Implication &implicationValue);
	bool isProperSubset(const std::string &possibleSubset);
	bool hasImplicationData(const std::string &predicateCode);

private:

/* Locals */
	std::string _stringRepresentation;
	CubeTreeNode* _parent = nullptr;
	int _upperLimit;
	int _varCount;
	int _firstUsableIndex;
	int _childrenCount = 0;
	std::map<std::string, Implication> _predicateImplications;
	std::vector<CubeTreeNode*> _children;
	std::vector<Ast*> _astRepresentation;

/* Private fields */
	std::vector<Ast*> getAstRepresentation();
	std::vector<std::string> getCurrentSubtreeRepresentations();

/* Private methods */
	void initializeChildren();
	void calculateImplication(Ast* predicate);
};