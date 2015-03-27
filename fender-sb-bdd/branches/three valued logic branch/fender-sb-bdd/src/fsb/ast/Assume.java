package fsb.ast;

public class Assume extends Statement {

	public BoolExpr cond;
	
	public Assume(BoolExpr cond) {
		super(StatType.ASSUME);
		this.cond = cond;
	}

	public String toString()
	{
		return "ASSUME " + cond;
	}

	@Override
	public boolean isLocal() {
		return true;
	}
}
