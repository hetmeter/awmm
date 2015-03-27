package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public class LocalExpr implements AritExpr {
	String local;
	
	public LocalExpr(String local)
	{
		this.local = local;
	}
	
	@Override
	public ArithValue evaluate(State s, int pid) {		
		return s.getLocal(pid, local);
	}

	public String toString()
	{
		return local;
	}
}
