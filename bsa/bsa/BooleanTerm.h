#pragma once

#include <string>
#include <vector>

class Predicate;
class Ast;

class BooleanTerm
{
public:
	BooleanTerm(int intValue);
	BooleanTerm(std::string varName);
	BooleanTerm(Predicate* predicate);
	BooleanTerm(Ast* node);
	~BooleanTerm();

	std::string toString();
	std::vector<std::string> involvedVariableNames();

private:
	enum explicitType { INTEGER, VARIABLE, PREDICATE };

	explicitType termType;
	int integerValue;
	std::string variableName;
	Predicate* predicateReference;
};