#ifndef __INIT_H_INCLUDED__
#define __INIT_H_INCLUDED__

#include <list>
#include "Ast.h"

using namespace std;

class Init : public Ast
{
	public:
		Init();
		~Init();
};

Init::Init()
{
	numberOfChildren = 0;
}

Ast::~Ast()
{
	//
}

#endif