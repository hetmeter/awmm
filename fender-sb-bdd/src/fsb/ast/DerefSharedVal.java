package fsb.ast;
import fsb.explore.SBState;

public class DerefSharedVal implements SharedVal {

	AritExpr expr;
	
	public DerefSharedVal(AritExpr expr)
	{
		this.expr = expr;
	}
	
	@Override
	public int evalShared(SBState s, int pid) {
		return SBState.getMemResource(expr.evaluate(s, pid));
	}

	@Override
	public String toString()
	{
		return "[" + expr.toString() + "]";
	}
}
