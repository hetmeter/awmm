package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.State;

public class AssertConstExpr implements AssertAritExpr {
	ArithValue val;
	
	public AssertConstExpr(int val)
	{
		this.val = DeterArithValue.getInstance(val);	
	}

	@Override
	public ArithValue evaluate(State s) {
		return val;
	}
	
	public String toString()
	{
		return "" + val;
	}
}
