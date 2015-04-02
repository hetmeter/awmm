#include "Predicate.h"
#include "Ast.h"
#include "BooleanTerm.h"

Predicate::Predicate(config::booleanOperator op, BooleanTerm* leftTerm, BooleanTerm* rightTerm)
{
	boolOp = op;
	left = leftTerm;
	right = rightTerm;
}

Predicate::Predicate(Ast* node)
{
	boolOp = config::stringToBooleanOperator(node->name);

	if (boolOp == config::booleanOperator::INVALID)
	{
		config::throwCriticalError("Invalid boolean operator: \"" + node->name + "\"");
	}

	left = new BooleanTerm(node->children.at(0));
	
	if (boolOp != config::booleanOperator::NOT)
	{
		right = new BooleanTerm(node->children.at(1));
	}
	else
	{
		right = NULL;
	}
}