package fsb.ast;

import java.util.HashSet;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

public class AssertLocal implements AssertAritExpr {
	int m_local;
	int m_pid;
	protected Set<Integer> m_involvingVariables = null;
	
	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + m_local;
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
		AssertLocal other = (AssertLocal) obj;
		if (m_local != other.m_local)
			return false;
		if (m_pid != other.m_pid)
			return false;
		return true;
	}

	public AssertLocal(String local, int pid) {
		int localNum = SymbolTable.getOrCreateLocalVariable(local);
		this.m_local = localNum;
		this.m_pid = pid;
	}


	public ArithValue evaluate(State s) {
		return s.getLocal(m_pid, m_local);
	}

	public String toString() {
		return SymbolTable.getLocalVariableName(m_local);
	}


	public Set<Integer> getVariablesInvolved() {
		if (null == m_involvingVariables) {
			m_involvingVariables = new HashSet<Integer>();
			m_involvingVariables.add(m_local);
		}
		return m_involvingVariables;
	}	
	
	public AssertAritExpr assignConcreteValuesFromState(State s) {
		ArithValue val = evaluate(s);
		if(val.isDetermined()){
			return new AssertConstExpr(((DetArithValue)val).getValue());
		}
		return this;
	}
	

}
