package fsb.tvl;

import java.util.Collection;

import fsb.ast.BoolExpr;

public abstract class BoolValue {

	public abstract BoolValue and(BoolValue b);
	public abstract BoolValue or(BoolValue b);
	public abstract BoolValue not();
	
	public abstract boolean isTrue();
	public abstract boolean isDetermined();
	public abstract boolean isFalse();
	
	public abstract Collection<BoolValue> getConcreteValues();
	
	/**
	 * shortcut for boolean operations
	 * @param rightval
	 * @param type
	 * @return
	 */
	public BoolValue switchBooolAction(BoolValue rightval, BoolExpr.BXType type){
		switch (type)
		{
			case AND:
				return this.and(rightval);
			case OR:
				return this.or(rightval);
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}
	}
	
	//shortcut 
	public BoolValue switchCmpr(BoolValue rightval, BoolExpr.BXType type){
		switch (type)
		{
			case EQUAL:
				return DetBoolValue.getInstance(this == rightval);
			case NEQ:
				return DetBoolValue.getInstance(this != rightval);
			case GREATER:
				throw new RuntimeException("Invalid type for boolean expression!");
			case LESS:
				throw new RuntimeException("Invalid type for boolean expression!");
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}

	}
	/**
	 * Hack 1 since insisting to keep variables boolean while allowing them to be compared to Arith.
	 * @return
	 */
	public abstract ArithValue getArithEquvalent();
}
