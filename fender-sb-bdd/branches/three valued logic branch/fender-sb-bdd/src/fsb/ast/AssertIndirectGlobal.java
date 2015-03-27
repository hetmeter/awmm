package fsb.ast;
import fsb.ast.tvl.ArithValue;
import fsb.explore.SBState;
import fsb.explore.State;

public class AssertIndirectGlobal implements AssertAritExpr {

	AssertAritExpr expr;
	
	public AssertIndirectGlobal(AssertAritExpr expr)
	{
		this.expr = expr;
	}
	
	@Override
	public ArithValue evaluate(State s) {		
		return ((SBState)s).getShared(SBState.getMemResource(expr.evaluate(s)));
	}

	@Override
	public String toString()
	{
		return "[" + expr.toString() + "]";
	}
}
