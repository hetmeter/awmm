#ifndef __AST_H_INCLUDED__
#define __AST_H_INCLUDED__

#include <list>

using namespace std;

class Ast
{
	public:
		Ast();
		~Ast();

		list<Ast> children;
		int numberOfChildren;
};

Ast::Ast()
{
	numberOfChildren = 0;
}

Ast::~Ast()
{
	//
}

#endif