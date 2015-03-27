package fsb.ast;

import fsb.explore.State;
import fsb.tvl.ArithValue;

public class Assign extends Statement {

	AritExpr src;
	int dst;
	
	public Assign(AritExpr src, String dst) {
		super(StatType.ASSIGN);
		this.src = src;
		this.dst = SymbolTable.getOrCreateLocalVariable(dst);
	}

	public String toString()
	{
		return SymbolTable.getLocalVariableName(dst) + " = " + src.toString();
	}

	public ArithValue getSrcVal(State s, int pid) {		
		return src.evaluate(s, pid);
	}

	public int getDst() {
		return dst;
	}

	@Override
	public boolean isLocal() {
		return true;
	}
}
