package fsb.ast;

import java.util.List;
import java.util.Set;

import fsb.explore.SBState;
import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

public class AssertIndirectGlobal implements AssertAritExpr {

	AssertAritExpr m_expr;

	public AssertIndirectGlobal(AssertAritExpr expr) {
		this.m_expr = expr;
	}

	@Override
	public ArithValue evaluate(State s) {
		return ((SBState) s)
				.getShared(SBState.getMemResource(m_expr.evaluate(s)));
	}

	@Override
	public String toString() {
		return "[" + m_expr.toString() + "]";
	}

	@Override
	public Set<Integer> getVariablesInvolved() {
		return m_expr.getVariablesInvolved();
	}

	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof AssertIndirectGlobal)) {
			return false;
		}
		AssertIndirectGlobal other = (AssertIndirectGlobal) obj;

		return other.m_expr.equals(m_expr);
	}

	@Override
	public int hashCode() {
		// TODO Auto-generated method stub
		return m_expr.hashCode();
	}

	@Override
	public AssertAritExpr assignConcreteValuesFromState(State s) {
		ArithValue val = ((SBState) s)
				.getShared(SBState.getMemResource(m_expr.evaluate(s)));
		if(val.isDetermined()){
			return new AssertConstExpr(((DetArithValue)val).getValue());
		}
		return this;
	}
}
