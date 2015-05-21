#pragma once

class Ast;

class ControlFlowEdge
{
public:
	ControlFlowEdge(Ast* startParameter, Ast* endParameter);
	~ControlFlowEdge();
	Ast* start;
	Ast* end;
};