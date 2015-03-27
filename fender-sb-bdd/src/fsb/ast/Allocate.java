package fsb.ast;

import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;
import fsb.tvl.NondetArithValue;

public class Allocate extends Statement {

	int dst;
	AritExpr num;
	
	public Allocate(String dst, AritExpr num) {
		super(StatType.ALLOCATE);
		this.num = num;
		this.dst = SymbolTable.getOrCreateGlobalVariable(dst);
	}

	public String toString()
	{
		return "ALLOCATE " + SymbolTable.getGlobalVariableName(dst) + " = " + num.toString();
	}

	public int getNumVal(State s, int pid) {
		ArithValue arith = num.evaluate(s, pid);
		if(arith instanceof NondetArithValue){
			throw new RuntimeException("Cannont allocate *");
		}
		DetArithValue val = (DetArithValue)arith;
		return val.getValue();
	}

	public int getDst() {
		return dst;
	}

	@Override
	public boolean isLocal() {
		//TODO: This could conceivably be "true"
		return false;
	}
	
	
}
