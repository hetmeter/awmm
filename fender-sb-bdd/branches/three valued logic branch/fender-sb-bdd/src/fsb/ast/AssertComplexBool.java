package fsb.ast;

import fsb.explore.State;

public class AssertComplexBool implements AssertBoolExpr {	
	private AssertBoolExpr left, right;
	private BoolExpr.BXType type;
	
	public AssertComplexBool(AssertBoolExpr left, AssertBoolExpr right, BoolExpr.BXType type)
	{
		this.left = left;
		this.right = right;
		this.type = type;
	}
	
	public boolean evaluate(State s)
	{
		boolean leftval = left.evaluate(s);
		boolean rightval = right.evaluate(s);
		
		switch (type)
		{
			case AND:
				return leftval && rightval;
			case OR:
				return leftval || rightval;
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}
	}
	
	public String toString()
	{
		String typeStr;
		switch (type)
		{
			case AND:
				typeStr = "&&";
				break;
			case OR:
				typeStr = "||";
				break;
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}	
		
		return "(" + left.toString() + " " + typeStr + " " + right.toString() + ")";
	}
}
