package fsb.ast;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.BoolValue;
import fsb.tvl.DetBoolValue;

public class AssertConstBool extends AssertBoolExpr {
	BoolValue val;
	
	public AssertConstBool(boolean val) 
	{
		this.val = DetBoolValue.getInstance(val);
	}

	public AssertConstBool(BoolValue val) 
	{
		this.val = val;
	}
	@Override
	public BoolValue evaluate_partVisb(State s) {
		return val;
	}
	
	public String toString()
	{
		return "" + val;
	}
	@Override
	public Set<Integer> getVariablesInvolved() {
		return Collections.EMPTY_SET;
	}
	
	@Override
	public boolean equals(Object obj) {
		if(! (obj instanceof AssertConstBool) ){
			return false;
		}

		AssertConstBool other = (AssertConstBool)obj;
		
		if(! other.val.equals(val))
		return false;

		
		return true;
	};
	@Override
	public int hashCode() {
		int ret = 13 + val.hashCode();
		
		return ret;
	}
	@Override
	public AssertBoolExpr assignConcreteValuesFromState(State s) {

		return this;
	}
	@Override
	public Set<AssertBoolExpr> allSatisfiedCubes(State s){
		
		if(evaluate_partVisb(s).isTrue()){
			HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
			ret.add(this);
			return ret;
		}
		return Collections.EMPTY_SET;
	}
	
	@Override
	public Set<AssertBoolExpr> allNegationSatisfiedCubes(State s){
		if(evaluate_partVisb(s).isFalse()){
			HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
			ret.add(this);
			return ret;
		}
		return Collections.EMPTY_SET;
	}
	@Override
	public Set<AssertBoolExpr> allCubes() {
		HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
		ret.add(this);
		return ret;
	}
	

	@Override
	public boolean isNegatedDNF() {
		// TODO Auto-generated method stub
		return false;
	}


}
