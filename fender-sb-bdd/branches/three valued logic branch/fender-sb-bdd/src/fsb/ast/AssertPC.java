package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.SBState;
import fsb.explore.State;

public class AssertPC implements AssertAritExpr {
	int pid;
	
	public AssertPC(int pid)
	{
		this.pid = pid;
	}
	
	@Override
	//The naming is misleading - "pc" evaluates to the label, not the actual pc.
	public ArithValue evaluate(State s) {		
		int currRealPC = ((SBState) s).getPC(pid);
		return DeterArithValue.getInstance(s.program.getListing(pid).get(currRealPC).getLabel());
	}

	public String toString()
	{
		return "pc";
	}
}
