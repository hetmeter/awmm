package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public class Assign extends Statement {

	AritExpr src;
	String dst;
	
	public Assign(AritExpr src, String dst) {
		super(StatType.ASSIGN);
		this.src = src;
		this.dst = dst;
	}

	public String toString()
	{
		return dst.toString() + " = " + src.toString();
	}

	public ArithValue getSrcVal(State s, int pid) {		
		return src.evaluate(s, pid);
	}

	public String getDst() {
		return dst;
	}

	@Override
	public boolean isLocal() {
		return true;
	}
}
