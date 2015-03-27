package fsb.ast;

import fsb.explore.SBState;

public class AtomicDec extends Statement {
	String res;
	SharedVal shared;
	
	public AtomicDec(String res, SharedVal shared) {
		super(StatType.ATOMICDEC);
		this.res = res;
		this.shared = shared;
	}

	public String toString()
	{
		return res + " = DEC " + shared;
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
