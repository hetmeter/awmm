package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.SBState;
import fsb.explore.State;

public class CAS extends Statement {
	String res;
	SharedVal shared;
	AritExpr oldval;
	AritExpr newval;
	
	public CAS(String res, SharedVal shared, AritExpr oldval, AritExpr newval) {
		super(StatType.CAS);
		this.res = res;
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
