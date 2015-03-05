#include "ControlFlowEdge.h"


ControlFlowEdge::ControlFlowEdge(Ast* startParameter, Ast* endParameter)
{
	start = startParameter;
	end = endParameter;
}


ControlFlowEdge::~ControlFlowEdge()
{
}