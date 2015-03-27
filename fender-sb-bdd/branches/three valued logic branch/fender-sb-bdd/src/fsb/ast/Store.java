package fsb.ast;

import com.sun.tools.javac.util.List;

import fsb.ast.tvl.ArithValue;
import fsb.explore.SBState;
import fsb.explore.State;

public class Store extends Statement {
	AritExpr src;
	SharedVal dst;
	
	public Store(AritExpr src, SharedVal dst) {
		super(StatType.STORE);
		this.src = src;
		this.dst = dst;
	}

	public String toString()
	{
		return "STORE " + dst + " = " + src;
	}
	

	public ArithValue getSrcVal(State s, int pid) {
		//TODO: Ugly, ugly hack for MV. no more?

		return src.evaluate(s, pid);
	}
	
	public String getDst(SBState s, int pid) {
		return dst.evalShared(s, pid);
	}

	@Override
	public boolean isLocal() {
		return false;
	}
}
