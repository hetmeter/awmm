package fsb.ast;

import fsb.utils.Options;

public class Nop extends Statement {

	public Nop() {
		super(StatType.NOP);
	}

	public String toString()
	{
		return "NOP";
	}

	@Override
	public boolean isLocal() {
		return Options.LOCAL_NOP;
	}
}
