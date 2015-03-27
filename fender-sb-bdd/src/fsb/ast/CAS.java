package fsb.ast;

import fsb.explore.SBState;
import fsb.explore.State;
import fsb.tvl.ArithValue;

public class CAS extends Statement {
	int res;
	SharedVal shared;
	AritExpr oldval;
	AritExpr newval;
	
	public CAS(String res, SharedVal shared, AritExpr oldval, AritExpr newval) {
		super(StatType.CAS);
		this.res = SymbolTable.getOrCreateGlobalVariable(res);
		this.shared = shared;
		this.oldval = oldval;
		this.newval = newval;
	}

	public String toString()
	{
		return res + " = CAS " + shared + " == " + oldval + ", " + newval;
	}

	public ArithValue getOldVal(State s, int pid) {
		return oldval.evaluate(s, pid);
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
