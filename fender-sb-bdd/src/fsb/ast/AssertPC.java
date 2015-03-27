package fsb.ast;

import java.util.Collections;
import java.util.Set;

import fsb.explore.SBState;
import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

public class AssertPC implements AssertAritExpr {
	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + m_pid;
		return result;
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		AssertPC other = (AssertPC) obj;
		if (m_pid != other.m_pid)
			return false;
		return true;
	}

	int m_pid;

	public AssertPC(int pid) {
		this.m_pid = pid;
	}

	@Override
	// The naming is misleading - "pc" evaluates to the label, not the actual
	// pc.
	public ArithValue evaluate(State s) {
		int currRealPC = ((SBState) s).getPC(m_pid);
		int label = State.program.getListing(m_pid).get(currRealPC).getLabel();
		/*
		 * System.out.println("process " + pid +" currRealPC="+((SBState)
		 * s).getPC(pid) +" label="+label +" instruction " +
		 * s.program.getListing(pid).get(currRealPC) );
		 */
		return DetArithValue.getInstance(label);
	}

	public String toString() {
		return "pc";
	}

	@Override
	public Set<Integer> getVariablesInvolved() {
		return Collections.EMPTY_SET;
	}

	@Override
	public AssertAritExpr assignConcreteValuesFromState(State s) {
		ArithValue val = evaluate(s);
		if(val.isDetermined()){
			return new AssertConstExpr(((DetArithValue)val).getValue());
		}
		return this;
	}


}
