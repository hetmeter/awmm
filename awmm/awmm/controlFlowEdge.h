#pragma once

struct ast;
typedef struct ast ast;

struct controlFlowEdge
{
	ast* start;
	ast* end;
	controlFlowEdge(ast* startParameter, ast* endParameter);
};
typedef struct controlFlowEdge controlFlowEdge;