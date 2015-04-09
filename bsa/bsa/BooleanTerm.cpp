#include "BooleanTerm.h"
#include "Predicate.h"
#include "Ast.h"

using namespace std;

BooleanTerm::BooleanTerm(int value)
{
	integerValue = value;
	termType = explicitType::INTEGER;
}

BooleanTerm::BooleanTerm(string varName)
{
	variableName = varName;
	termType = explicitType::VARIABLE;
}

BooleanTerm::BooleanTerm(Predicate* predicate)
{
	predicateReference = predicate;
	termType = explicitType::PREDICATE;
}

BooleanTerm::BooleanTerm(Ast* node)
{
	string nodeName = node->name;

	if (nodeName == config::INT_TOKEN_NAME)
	{
		integerValue = stoi(node->children.at(0)->name);
		termType = explicitType::INTEGER;
	}
	else if (nodeName == config::ID_TOKEN_NAME)
	{
		variableName = node->children.at(0)->name;
		termType = explicitType::VARIABLE;
	}
	else if (config::stringToBooleanOperator(nodeName))
	{
		predicateReference = new Predicate(node);
		termType = explicitType::PREDICATE;
	}
	else
	{
		config::throwCriticalError("Not a valid BooleanTerm node: \"" + node->name + "\"");
	}
}

BooleanTerm::~BooleanTerm()
{

}

string BooleanTerm::toString()
{
	string result = "";

	if (termType == explicitType::INTEGER)
	{
		result += to_string(integerValue);
	}
	else if (termType == explicitType::VARIABLE)
	{
		result += variableName;
	}
	else if (termType == explicitType::PREDICATE)
	{
		result += predicateReference->toString();
	}

	return result;
}

vector<string> BooleanTerm::involvedVariableNames()
{
	vector<string> result;

	if (termType == explicitType::PREDICATE)
	{

	}

	return result;
}