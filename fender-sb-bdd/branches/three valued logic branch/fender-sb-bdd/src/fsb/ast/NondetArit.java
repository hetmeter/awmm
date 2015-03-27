package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.NonDeterArithValue;
import fsb.explore.State;

//Like NondetBool, this is a huge hack.
//In this case, though, it's not clear how it could be avoided.
public class NondetArit implements AritExpr {
	int val;
	
	public NondetArit() { }

	@Override
	public ArithValue evaluate(State s, int pid) {
		return NonDeterArithValue.getInstance();
	}
	
	public String toString()
	{
		return "*";
	}
}
