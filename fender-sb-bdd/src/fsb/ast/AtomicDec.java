package fsb.ast;

import fsb.explore.SBState;

public class AtomicDec extends Statement {
	int res;
	SharedVal shared;
	
	public AtomicDec(String res, SharedVal shared) {
		super(StatType.ATOMICDEC);
		this.res = SymbolTable.getOrCreateGlobalVariable(res);
		this.shared = shared;
	}

	public String toString()
	{
		return res + " = DEC " + shared;
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
