package fsb.ast.tvl;

import fsb.ast.BoolExpr;

public abstract class ArithValue {

	//arithmetic operations
	public abstract ArithValue plus(ArithValue a);
	public abstract ArithValue sub(ArithValue a);
	public abstract ArithValue mul(ArithValue a);
	
	//though that I might need it
	public abstract ArithValue negate();
	
	//comparison
	public abstract BoolValue eq(ArithValue a);
	public abstract BoolValue neq(ArithValue a);
	public abstract BoolValue lessThen(ArithValue a);
	public abstract BoolValue greaterThen(ArithValue a);
	
	
	//shortcut
	public BoolValue compare(ArithValue rightval, BoolExpr.BXType type){
		switch (type)
		{
			case EQUAL:
				return eq(rightval);
			case NEQ:
				return neq(rightval);
			case GREATER:
				return greaterThen(rightval);
			case LESS:
				return lessThen(rightval);
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}
	}
	public BoolValue eq(int i) {
		return eq(DeterArithValue.getInstance(i));
	}

}
