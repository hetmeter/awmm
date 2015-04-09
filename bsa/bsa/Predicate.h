#pragma once

#include "config.h"

class Ast;
class BooleanTerm;

class Predicate
{
public:
	Predicate(config::booleanOperator op, BooleanTerm* leftTerm, BooleanTerm* rightTerm);
	Predicate(Ast* node);
	Predicate(Ast* statement, Predicate* predicate);

	~Predicate();

	std::string toString();

private:
	config::booleanOperator boolOp;
	BooleanTerm* left;
	BooleanTerm* right;
};