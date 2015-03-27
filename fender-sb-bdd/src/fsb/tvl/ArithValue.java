package fsb.tvl;

import fsb.ast.AritExpr.AXType;
import fsb.ast.BoolExpr;

/**
 * Hopefully will not be using it
 * @author user
 *
 */
public abstract class ArithValue {

	public abstract ArithValue plus(ArithValue a);
	public abstract ArithValue minus(ArithValue a);
	public abstract ArithValue mul(ArithValue a);
	public abstract ArithValue unaryMinus();
	
	public abstract BoolValue eq(ArithValue a);
	public abstract BoolValue smallerThen(ArithValue a);
	public abstract BoolValue greaterThen(ArithValue a);
	

	//shortcut 
	public BoolValue switchCmpr(ArithValue rightval, BoolExpr.BXType type){
		switch (type)
		{
			case EQUAL:
				return this.eq(rightval);
			case NEQ:
				return this.eq(rightval).not();
			case GREATER:
				return this.greaterThen(rightval);
			case LESS:
				return this.smallerThen(rightval);
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}

	}
	public ArithValue switchOp(ArithValue rightval, AXType type) {
		switch (type)
		{
			case PLUS:
				return this.plus(rightval);
			case MINUS:
				return this.minus(rightval);
			case MUL:
				return this.mul(rightval);
			default:
				throw new RuntimeException("Invalid type for arithmetic expression!");
		}
		}
	//public abstract BoolValue toBoolValue() ;
	public abstract boolean isDetermined();
	public abstract ArithValue not() ;

}
