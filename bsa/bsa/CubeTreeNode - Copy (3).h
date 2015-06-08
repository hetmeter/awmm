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
	Implication getPredicateImplication(Ast* predicate);
	void setPredicateImplication(const std::string &predicateCode, Implication predicateImplication);
	std::vector<std::string> getSupersetStringRepresentations();
	std::vector<std::string> getCanonicalSupersetStringRepresentations();

private:

/* Locals */
	std::string _stringRepresentation;
	int _upperLimit;
	int _varCount;
	int _suffixIndex;
	std::map<std::string, Implication> _predicateImplications;
	std::vector<Ast*> _astVectorRepresentation;
	std::vector<std::string> _supersetStringRepresentations;
	std::vector<std::string> _canonicalSupersetStringRepresentations;


/* Private fields */
	std::vector<Ast*> getAstVectorRepresentation();
};