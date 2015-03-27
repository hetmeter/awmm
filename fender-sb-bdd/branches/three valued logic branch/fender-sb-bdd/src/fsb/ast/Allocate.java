package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public class Allocate extends Statement {

	String dst;
	AritExpr num;
	
	public Allocate(String dst, AritExpr num) {
		super(StatType.ALLOCATE);
		this.num = num;
		this.dst = dst;
	}

	public String toString()
	{
		return "ALLOCATE " + dst.toString() + " = " + num.toString();
	}

	public ArithValue getNumVal(State s, int pid) {		
		return num.evaluate(s, pid);
	}

	public String getDst() {
		return dst;
	}

	@Override
	public boolean isLocal() {
		//TODO: This could conceivably be "true"
		return false;
	}
	
	
}
