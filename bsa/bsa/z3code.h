#pragma once

#include <string>

namespace z3code
{
	class Predicate;
	class Term;
	class Ast;

	enum booleanOperator { EQUALS, NOT_EQUALS, GREATER, GREATER_EQUALS, LESS, LESS_EQUALS, NOT };
	enum arithmeticOperator { PLUS, MINUS, DIVIDE, MULTIPLY };
	enum booleanPrimitives { TRUE, FALSE, UNKNOWN };

	const extern std::string PUSH_PREFIX;
	const extern std::string GENERAL_SUFFIX;

	extern std::string push(int number);
	extern std::string ite(std::string conditionalTerm, std::string thenTerm, std::string elseTerm);
	extern Term weakestPrecondition(Predicate x, Predicate phi);

	class Predicate
	{
	public:
		Predicate::Predicate(booleanOperator op, Term* left, Term* right);
		Predicate::~Predicate();
	};

	class Term
	{
	public:
		Term::Term(booleanPrimitives primitive);
		Term::Term(std::string name);
		Term::Term(Ast* node);
		Term::Term(arithmeticOperator op, Term* left, Term* right);
		Term::~Term();

		std::string Term::ToString();
	};
}