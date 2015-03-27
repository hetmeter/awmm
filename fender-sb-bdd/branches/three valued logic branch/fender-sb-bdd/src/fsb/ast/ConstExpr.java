package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.State;

public class ConstExpr implements AritExpr {
	ArithValue val;
	
	public ConstExpr(int val)
	{
		this.val = DeterArithValue.getInstance(val);	
	}

	@Override
	public ArithValue evaluate(State s, int pid) {
		return val;
	}
	
	public String toString()
	{
		return "" + val;
	}
}
