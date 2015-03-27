package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.BoolValue;
import fsb.explore.State;

public class ComparsionBool implements BoolExpr {	
	private AritExpr left, right;
	private BXType type;
	
	public ComparsionBool(AritExpr left, AritExpr right, BXType type)
	{
		this.left = left;
		this.right = right;
		this.type = type;
	}
	
	public BoolValue evaluate(State s, int pid)
	{
		ArithValue leftval = left.evaluate(s, pid);
		ArithValue rightval = right.evaluate(s, pid);
		
		switch (type)
		{
			case EQUAL:
				return leftval.eq(rightval);
			case NEQ:
				return leftval.neq(rightval);
			case GREATER:
				return leftval.greaterThen(rightval);
			case LESS:
				return leftval.lessThen(rightval);
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}
	}
	
	public String toString()
	{
		String typeStr;
		switch (type)
		{
			case EQUAL:
				typeStr = "==";
				break;
			case NEQ:
				typeStr = "!=";
				break;
			case GREATER:
				typeStr = ">";
				break;
			case LESS:
				typeStr = "<";
				break;
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}	
		
		return "(" + left.toString() + " " + typeStr + " " + right.toString() + ")";
	}
}
