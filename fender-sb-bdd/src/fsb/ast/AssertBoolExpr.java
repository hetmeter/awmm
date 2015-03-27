package fsb.ast;

import java.util.Set;
import java.util.WeakHashMap;

import fsb.explore.State;
import fsb.semantics.Semantics;
import fsb.tvl.BoolValue;
import fsb.utils.Options;

abstract public class  AssertBoolExpr {
	/**
	 * a function to force all inheriting function to implement an evaluate
	 * and still have a general wrapper for it.
	 * used when one doesn't want to use the wrapper...
	 * @param s
	 * @return
	 */
	abstract public BoolValue evaluate_partVisb(State s)  ;
	abstract public Set<Integer> getVariablesInvolved();
	abstract public AssertBoolExpr assignConcreteValuesFromState(State s);
	abstract public Set<AssertBoolExpr> allSatisfiedCubes(State s);
	abstract public Set<AssertBoolExpr> allNegationSatisfiedCubes(State s);
	abstract public Set<AssertBoolExpr> allCubes();
	
	final public BoolValue evaluate(State s)  {
		BoolValue ret = evaluate_partVisb(s);
		if (Options.PRINT_USED_CUBES){
			AssertComplexBool.m_allCubesGlobal.addAll(allCubes());
			//if undetermined condition not interesting
			if(ret.isTrue()){
				Semantics.addAssertCubesToSet(allNegationSatisfiedCubes(s),
					allCubes(),Semantics.cubesAssertFalseTogetherMap);
				AssertComplexBool.m_usefullCubesGlobal.addAll(allNegationSatisfiedCubes(s));
			}
			if(ret.isFalse()){
				Semantics.addAssertCubesToSet(allNegationSatisfiedCubes(s),
					allCubes(),Semantics.cubesAssertFalseTogetherMap);
				AssertComplexBool.m_usefullCubesGlobal.addAll(allNegationSatisfiedCubes(s));
			}
			
		}
		return ret;
		
	}
	
	abstract public boolean isNegatedDNF();
	
	protected static WeakHashMap<Integer, AssertBoolExpr> m_hashed_evaluations = new WeakHashMap<Integer, AssertBoolExpr>();
	protected static WeakHashMap<AssertBoolExpr, AssertBoolExpr> m_hashed_bool_expr = new WeakHashMap<AssertBoolExpr, AssertBoolExpr>();
	
	/**
	 * @param b
	 * @return
	 */
	protected AssertBoolExpr returnGivenAssertBoolExprOrHashed(AssertBoolExpr b) {
//		AssertBoolExpr hashed = m_hashed_bool_expr.get(b);
//		if(hashed == null){
//			m_hashed_bool_expr.put(b, b);
//			return b;
//		}
//		if(!hashed.equals(b)){
//			throw new RuntimeException("hashed value wrong in assert bool expr");
//		}
//		return hashed;
		return b;
	}
}
