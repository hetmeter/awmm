package fsb.ast;

import java.util.Set;

import fsb.explore.State;
import fsb.tvl.ArithValue;

public interface AssertAritExpr {
	public ArithValue evaluate(State s);
	public Set<Integer> getVariablesInvolved() ;
	public AssertAritExpr assignConcreteValuesFromState(State s);
}
