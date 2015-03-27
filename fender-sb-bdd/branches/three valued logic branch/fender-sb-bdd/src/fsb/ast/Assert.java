package fsb.ast;

public class Assert {
	private AssertBoolExpr expr;
	private boolean alwaysActive; 
	
	public Assert(AssertBoolExpr assertExpr, boolean alwaysActive)
	{
		expr = assertExpr;
		this.alwaysActive = alwaysActive;
	}
	
	public AssertBoolExpr getExpr()
	{
		return expr;
	}
	
	public boolean alwaysActive()
	{
		return alwaysActive;
	}
}
