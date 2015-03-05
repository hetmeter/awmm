#ifndef CONTROLFLOWEDGE_H_INCLUDED
#define CONTROLFLOWEDGE_H_INCLUDED
#include "controlFlowEdge.h"
#endif

struct ast;
typedef struct ast ast;

using namespace std;

controlFlowEdge::controlFlowEdge(ast* startParameter, ast* endParameter)
{
	start = startParameter;
	end = endParameter;
}
typedef struct controlFlowEdge controlFlowEdge;