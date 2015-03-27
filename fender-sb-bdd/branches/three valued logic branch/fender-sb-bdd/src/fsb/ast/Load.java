package fsb.ast;

import fsb.explore.SBState;

public class Load extends Statement {
	SharedVal src;
	String dst;
	
	public Load(SharedVal src, String dst) {
		super(StatType.LOAD);
		this.src = src;
		this.dst = dst;
	}

	public String toString()
	{
		return "LOAD " + dst + " = " + src;
	}

	public String getSrc(SBState s, int pid) {
		return src.evalShared(s, pid);
	}
	
	public String getDst() {
		return dst;
	}

	@Override
	public boolean isLocal() {
		return false;
	}

}
