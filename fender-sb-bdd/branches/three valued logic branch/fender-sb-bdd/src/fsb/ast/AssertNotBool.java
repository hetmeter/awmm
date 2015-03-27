package fsb.ast;

import fsb.explore.State;

public class AssertNotBool implements AssertBoolExpr {	
	private AssertBoolExpr expr;
	
	public AssertNotBool(AssertBoolExpr expr)
	{
		this.expr = expr;
	}
	
	public boolean evaluate(State s)
	{
		boolean exprval = expr.evaluate(s);
		return !exprval;
	}
	
	public String toString()
	{
		return "!(" + expr.toString() + ")";
	}
}
