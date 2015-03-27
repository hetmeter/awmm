package fsb.ast;


import java.util.List;
import java.util.Set;

import fsb.explore.State;
import fsb.semantics.Semantics;
import fsb.tvl.BoolValue;
import fsb.utils.Options;

public abstract class BoolExpr {
	public enum BXType {EQUAL, NEQ, GREATER, LESS, AND, OR, NOT};
	abstract protected BoolValue evaluate_inner(State s, int pid); 
	abstract public List<Integer> getVariablesInvolved();
	
	/**
	 * This function assumes the expression is in DNF form(or negated DNF)
	 */
	abstract public Set<BoolExpr> allSatisfiedCubes(State s, int pid);
	/**
	 * This function assumes the expression is in DNF form(or negated DNF)
	 */
	abstract public Set<BoolExpr> allNegationSatisfiedCubes(State s, int pid);
	/**
	 * This function assumes the expression is in DNF form(or negated DNF)
	 */
	abstract public Set<BoolExpr> allCubes();
	

	 final public BoolValue evaluate(State s, int pid) {
		 BoolValue ret = evaluate_inner(s, pid);
			if (Options.PRINT_USED_CUBES){
				ComplexBool.m_allCubesGlobal.addAll(allCubes());
				//if undetermined condition not interesting
				if(ret.isTrue()){
					Semantics.addCubesToSet(allSatisfiedCubes(s,pid),
						allCubes(),Semantics.cubesTrueTogetherMap);
					ComplexBool.m_usefullCubesGlobal.addAll(allSatisfiedCubes(s,pid));
				}
				if(ret.isFalse()){
					Semantics.addCubesToSet(allNegationSatisfiedCubes(s,pid),
						allCubes(),Semantics.cubesFalseTogetherMap);
					ComplexBool.m_usefullCubesGlobal.addAll(allNegationSatisfiedCubes(s,pid));
				}
				
			}
		 return ret;
	}
	
}
