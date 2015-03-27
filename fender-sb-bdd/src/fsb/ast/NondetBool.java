package fsb.ast;

import java.util.ArrayList;
import java.util.List;
import java.util.Collections;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.BoolValue;
import fsb.tvl.NondetBoolValue;

//This is a huge hack.
//The right thing to do would be to move from a boolean domain to {F, T, \top} and perform evaluations accordingly.
public class NondetBool extends BoolExpr {
	public NondetBool() { }

	@Override
	protected BoolValue evaluate_inner(State s, int pid) {
		return NondetBoolValue.getInstance();
		//throw new RuntimeException("Should not try to evaluate non-deterministic expressions");
	}
	
	public String toString()
	{
		return "*";
	}
	
	@Override
	public List<Integer> getVariablesInvolved() {
		return Collections.EMPTY_LIST;
	}
	
	@Override
	public Set<BoolExpr> allSatisfiedCubes(State s, int pid){
		return Collections.EMPTY_SET;
	}
	
	@Override
	public Set<BoolExpr> allNegationSatisfiedCubes(State s, int pid){
		return Collections.EMPTY_SET;
	}

	@Override
	public Set<BoolExpr> allCubes() {
		return Collections.EMPTY_SET;
	}
}
