package fsb.ast;

public class ChooseStore extends Statement {

	public BoolExpr cond1;
	public BoolExpr cond2;
	public SharedVal dst;
	
	public ChooseStore(BoolExpr cond1, BoolExpr cond2, SharedVal dst) {
		super(StatType.CHOOSE_STORE);
		this.cond1 = cond1;
		this.cond2 = cond2;
		this.dst = dst;
	}

	public String toString()
	{
		return "STORE " + dst + " = CHOOSE(" + cond1 + ", " + cond2 +"); ";
	}

	@Override
	public boolean isLocal() {
		return true;
	}
}
