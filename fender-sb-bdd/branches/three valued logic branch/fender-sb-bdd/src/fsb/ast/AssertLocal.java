package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public class AssertLocal implements AssertAritExpr {
	String local;
	int pid;
	
	public AssertLocal(String local, int pid)
	{
		this.local = local;
		this.pid = pid;
	}
	
	@Override
	public ArithValue evaluate(State s) {		
		return s.getLocal(pid, local);
	}

	public String toString()
	{
		return local;
	}
}
