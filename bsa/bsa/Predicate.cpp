#include "Predicate.h"
#include "Ast.h"
#include "BooleanTerm.h"

using namespace std;

Predicate::Predicate(config::booleanOperator op, BooleanTerm* leftTerm, BooleanTerm* rightTerm)
{
	boolOp = op;
	left = leftTerm;
	right = rightTerm;
}

Predicate::Predicate(Ast* node)
{
	boolOp = config::stringToBooleanOperator(node->name);

	if (boolOp == config::booleanOperator::BOP_INVALID)
	{
		config::throwCriticalError("Invalid boolean operator: \"" + node->name + "\"");
	}

	left = new BooleanTerm(node->children.at(0));
	
	if (boolOp != config::booleanOperator::BOP_NOT)
	{
		right = new BooleanTerm(node->children.at(1));
	}
	else
	{
		right = NULL;
	}
}

Predicate::Predicate(Ast* statement, Predicate* predicate)
{

}

string Predicate::toString()
{
	return "(" + left->toString() + " " + config::booleanOperatorToString(boolOp) + " " + right->toString() + ")";
}