package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public class ComplexAritExpr implements AritExpr {
	AritExpr left, right;
	AXType type;
	
	public ComplexAritExpr(AritExpr l, AritExpr r, AXType type) {
		this.left = l;
		this.right = r;
		this.type = type; 
	}

	public ArithValue evaluate(State s, int pid) {
		ArithValue leftval = left.evaluate(s, pid);
		ArithValue rightval = right.evaluate(s, pid);
		
		switch (type)
		{
			case PLUS:
				return leftval.plus(rightval);
			case MINUS:
				return leftval.sub(rightval);
			case MUL:
				return leftval.mul(rightval);
			default:
				throw new RuntimeException("Invalid type for arithmetic expression!");
		}
	}	
	
	public String toString()
	{
		String typeStr;
		switch (type)
		{
			case PLUS:
				typeStr = "+";
				break;
			case MINUS:
				typeStr = "-";
				break;
			case MUL:
				typeStr = "*";
				break;
			default:
				throw new RuntimeException("Invalid type for arithmetic expression!");
		}	
		
		return "(" + left.toString() + " " + typeStr + " " + right.toString() + ")";
	}
}
