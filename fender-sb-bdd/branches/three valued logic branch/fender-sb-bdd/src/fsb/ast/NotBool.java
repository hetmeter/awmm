package fsb.ast;

import fsb.ast.tvl.BoolValue;
import fsb.explore.State;

public class NotBool implements BoolExpr {	
	private BoolExpr expr;
	
	public NotBool(BoolExpr expr)
	{
		this.expr = expr;
	}
	
	public BoolValue evaluate(State s, int pid)
	{
		BoolValue exprval = expr.evaluate(s, pid);
		return exprval.not();
	}
	
	public String toString()
	{
		return "!(" + expr.toString() + ")";
	}
}
