package fsb.ast;

import fsb.ast.tvl.BoolValue;
import fsb.ast.tvl.NonDeterBoolValue;
import fsb.explore.State;

//This is a huge hack. No more?
public class NondetBool implements BoolExpr {
	public NondetBool() { }

	@Override
	public BoolValue evaluate(State s, int pid) {
		return NonDeterBoolValue.getInstance();
	}
	
	public String toString()
	{
		return "*";
	}
}
