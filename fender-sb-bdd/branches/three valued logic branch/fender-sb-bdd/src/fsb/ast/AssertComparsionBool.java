package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public class AssertComparsionBool implements AssertBoolExpr {	
	private AssertAritExpr left, right;
	private BoolExpr.BXType type;
	
	public AssertComparsionBool(AssertAritExpr left, AssertAritExpr right, BoolExpr.BXType type)
	{
		this.left = left;
		this.right = right;
		this.type = type;
	}
	
	public boolean evaluate(State s)
	{
		ArithValue leftval = left.evaluate(s);
		ArithValue rightval = right.evaluate(s);
		
		return leftval.compare(rightval, type).isTrue();
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
