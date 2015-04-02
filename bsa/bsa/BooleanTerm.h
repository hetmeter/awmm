#pragma once

#include <string>

class Predicate;
class Ast;

class BooleanTerm
{
public:
	BooleanTerm(int intValue);
	BooleanTerm(std::string varName);
	BooleanTerm(Ast* predicate);
	BooleanTerm(Predicate* predicate);
	~BooleanTerm();

	std::string toString();

private:
	enum explicitType { INTEGER, VARIABLE, PREDICATE };

	int integerValue;
	std::string variableName;
	Predicate* predicateReference;
};