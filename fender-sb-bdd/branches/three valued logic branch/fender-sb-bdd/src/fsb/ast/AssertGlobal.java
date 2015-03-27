package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.SBState;
import fsb.explore.State;

public class AssertGlobal implements AssertAritExpr {
	String global;
	
	public AssertGlobal(String global)
	{
		this.global = global;
	}
	
	@Override
	public ArithValue evaluate(State s) {		
		return ((SBState)s).getShared(global);
	}

	public String toString()
	{
		return global;
	}
}
