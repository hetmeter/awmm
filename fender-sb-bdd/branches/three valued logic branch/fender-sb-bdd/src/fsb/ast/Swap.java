package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.SBState;
import fsb.explore.State;

public class Swap extends Statement {
	String res;
	SharedVal shared;
	AritExpr newval;
	
	public Swap(String res, SharedVal shared, AritExpr newval) {
		super(StatType.SWAP);
		this.res = res;
		this.shared = shared;
		this.newval = newval;
	}

	public String toString()
	{
		return res + " = SWAP " + shared + ", " + newval;
	}

	public ArithValue getNewVal(State s, int pid) {
		return newval.evaluate(s, pid);
	}
	
	public String getShared(SBState s, int pid) {
		return shared.evalShared(s, pid);
	}

	public String getRes() {
		return res;
	}

	@Override
	public boolean isLocal() {
		return false;
	}	
}
