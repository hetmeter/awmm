package fsb.ast;

import fsb.explore.SBState;
import fsb.explore.State;
import fsb.tvl.ArithValue;

public class Swap extends Statement {
	int res;
	SharedVal shared;
	AritExpr newval;
	
	public Swap(String res, SharedVal shared, AritExpr newval) {
		super(StatType.SWAP);
		this.res = SymbolTable.getOrCreateGlobalVariable(res);
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
	
	public int getShared(SBState s, int pid) {
		return shared.evalShared(s, pid);
	}

	public int getRes() {
		return res;
	}

	@Override
	public boolean isLocal() {
		return false;
	}	
}
