package fsb.ast;

import fsb.ast.tvl.BoolValue;
import fsb.ast.tvl.DeterBoolValue;
import fsb.explore.State;

public class ConstBool implements BoolExpr {
	BoolValue val;
	
	public ConstBool(boolean val) 
	{
		this.val = DeterBoolValue.getInstance(val);
	}

	@Override
	public BoolValue evaluate(State s, int pid) {
		return val;
	}
	
	public String toString()
	{
		return "" + val;
	}
}
