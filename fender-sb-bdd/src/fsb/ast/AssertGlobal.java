package fsb.ast;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import fsb.explore.SBState;
import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

public class AssertGlobal implements AssertAritExpr {
	int m_global;
	protected Set<Integer> m_involvingVariables = null;

	public AssertGlobal(String global) {
		
		this.m_global = SymbolTable.getOrCreateGlobalVariable(global);
	}

	@Override
	public ArithValue evaluate(State s) {
		return ((SBState) s).getShared(m_global);
	}

	public String toString() {
		return SymbolTable.getGlobalVariableName(m_global);
	}

	@Override
	public Set<Integer> getVariablesInvolved() {
		if (null == m_involvingVariables) {
			m_involvingVariables = new HashSet<Integer>();
			m_involvingVariables.add(m_global);
		}
		return m_involvingVariables;
	}

	@Override
	public boolean equals(Object obj) {
		if(! (obj instanceof AssertGlobal) ){
			return false;
		}
		AssertGlobal other =(AssertGlobal) obj;
		
		return other.m_global == m_global;
	}
	
	@Override
	public int hashCode() {
		return m_global;
	}

	@Override
	public AssertAritExpr assignConcreteValuesFromState(State s) {
		
		ArithValue val = ((SBState) s).getShared(m_global);
		if(val.isDetermined()){
			return new AssertConstExpr(((DetArithValue)val).getValue());
		}
		return this;
	}
}
