package fsb.ast;

public class Branch extends Statement {

	public BoolExpr cond;
	public int toLabel;
	
	public Branch(BoolExpr cond, int toLabel) {
		super(StatType.BRANCH);
		this.cond = cond;
		this.toLabel = toLabel;
	}

	public String toString()
	{
		return "IF " + cond + " GOTO " + toLabel;
	}

	@Override
	public boolean isLocal() {
		return true;
	}
}
