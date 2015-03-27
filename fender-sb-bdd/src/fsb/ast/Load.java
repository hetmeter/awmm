package fsb.ast;

import fsb.explore.SBState;

public class Load extends Statement {
	SharedVal src;
	int dst;
	
	public Load(SharedVal src, String dst) {
		super(StatType.LOAD);
		this.src = src;
		this.dst = SymbolTable.getOrCreateLocalVariable(dst);
	}

	public String toString()
	{
		return "LOAD " + SymbolTable.getLocalVariableName(dst) + " = " + src;
	}

	public int getSrc(SBState s, int pid) {
		return src.evalShared(s, pid);
	}
	
	public int getDst() {
		return dst;
	}

	@Override
	public boolean isLocal() {
		return false;
	}

}
