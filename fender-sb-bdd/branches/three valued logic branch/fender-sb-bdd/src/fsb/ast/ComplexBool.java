package fsb.ast;

import fsb.ast.tvl.BoolValue;
import fsb.explore.State;

public class ComplexBool implements BoolExpr {	
	private BoolExpr left, right;
	private BXType type;
	
	public ComplexBool(BoolExpr left, BoolExpr right, BXType type)
	{
		this.left = left;
		this.right = right;
		this.type = type;
	}
	
	public BoolValue evaluate(State s, int pid)
	{
		BoolValue leftval = left.evaluate(s, pid);
		BoolValue rightval = right.evaluate(s, pid);
		
		switch (type)
		{
			case AND:
				return leftval.and(rightval);
			case OR:
				return leftval.or(rightval);
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
