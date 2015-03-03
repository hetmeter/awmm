#ifndef __AST_H_INCLUDED__
#define __AST_H_INCLUDED__
#include "ast.h"
#endif

#ifndef __VECTOR_INCLUDED__
#define __VECTOR_INCLUDED__
#include <vector>
#endif

typedef struct ast ast;
using namespace std;

struct controlFlowEdge
{
	ast* start;
	ast* end;

	controlFlowEdge(ast* startParameter, ast* endParameter)
	{
		start = startParameter;
		end = endParameter;
	}
};
typedef struct controlFlowEdge controlFlowEdge;