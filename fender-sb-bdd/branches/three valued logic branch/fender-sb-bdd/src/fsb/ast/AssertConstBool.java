package fsb.ast;

import fsb.explore.State;

public class AssertConstBool implements AssertBoolExpr {
	boolean val;
	
	public AssertConstBool(boolean val) 
	{
		this.val = val;
	}

	@Override
	public boolean evaluate(State s) {
		return val;
	}
	
	public String toString()
	{
		return "" + val;
	}
}
