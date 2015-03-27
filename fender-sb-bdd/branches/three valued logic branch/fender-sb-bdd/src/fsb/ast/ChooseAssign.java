package fsb.ast;

public class ChooseAssign extends Statement {

	public BoolExpr cond1;
	public BoolExpr cond2;
	public String dst;
	
	public ChooseAssign(BoolExpr cond1, BoolExpr cond2, String dst) {
		super(StatType.CHOOSE_ASSIGN);
		this.cond1 = cond1;
		this.cond2 = cond1;
		this.dst = dst;
	}

	public String toString()
	{
		return dst + "= CHOOSE(" + cond1 + ", " + cond2 +"); ";
	}

	public BoolExpr getCond1() {
		return cond1;
	}

	public BoolExpr getCond2() {
		return cond2;
	}

	public String getDst() {
		return dst;
	}

	@Override
	public boolean isLocal() {
		return true;
	}
}
