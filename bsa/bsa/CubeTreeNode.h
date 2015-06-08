#pragma once

#include <string>
#include <map>
#include <vector>

class Ast;

class CubeTreeNode
{
public:

/* Constants */
	static const char CUBE_STATE_OMIT = '-';
	static const char CUBE_STATE_DECIDED_FALSE = 'F';
	static const char CUBE_STATE_DECIDED_TRUE = 'T';

/* Enumerations */
	enum Implication { IMPLIES, NOT_IMPLIES, SUPERSET_IMPLIES };

/* Constructors and destructor */
	CubeTreeNode(int upperLimit);
	CubeTreeNode(const std::string &stringRepresentation, int upperLimit);
	~CubeTreeNode();

/* Public fields */
	Implication getPredicateImplication(Ast* predicate, const std::vector<int> &relevantIndices);
	void setPredicateImplication(const std::string &predicateCode, Implication predicateImplication);
	void setPredicateImplication(const std::string &predicateCode, const std::vector<int> &relevantIndices);
	std::vector<std::string> getSupersetStringRepresentations(const std::vector<int> &relevantIndices);
	std::vector<std::string> getCanonicalSupersetStringRepresentations(const std::vector<int> &relevantIndices);

private:

/* Locals */
	std::string _stringRepresentation;
	int _upperLimit;
	int _varCount;
	int _suffixIndex;
	std::map<std::string, Implication> _predicateImplications;
	std::vector<Ast*> _astVectorRepresentation;
	std::map<std::vector<int>, std::vector<std::string>> _supersetStringRepresentations;
	std::map<std::vector<int>, std::vector<std::string>> _canonicalSupersetStringRepresentations;


/* Private fields */
	std::vector<Ast*> getAstVectorRepresentation();
};