package fsb.ast;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.BoolValue;
import fsb.tvl.DetBoolValue;

public class ConstBool extends BoolExpr {
	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((val == null) ? 0 : val.hashCode());
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
		ConstBool other = (ConstBool) obj;
		if (val == null) {
			if (other.val != null)
				return false;
		} else if (!val.equals(other.val))
			return false;
		return true;
	}
	BoolValue val;
	
	public ConstBool(boolean val) 
	{
		this.val = DetBoolValue.getInstance(val);
	}

	@Override
	protected BoolValue evaluate_inner(State s, int pid) {
		return val;
	}
	
	public String toString()
	{
		return "" + val;
	}
	@Override
	public List<Integer> getVariablesInvolved() {
		return Collections.EMPTY_LIST;
	}
	@Override
	public Set<BoolExpr> allSatisfiedCubes(State s, int pid){
		
		if(evaluate_inner(s,pid).isTrue()){
			HashSet<BoolExpr> ret = new HashSet<BoolExpr>();
			ret.add(this);
			return ret;
		}
		return Collections.EMPTY_SET;
	}
	
	@Override
	public Set<BoolExpr> allNegationSatisfiedCubes(State s, int pid){
		if(evaluate_inner(s,pid).isFalse()){
			HashSet<BoolExpr> ret = new HashSet<BoolExpr>();
			ret.add(this);
			return ret;
		}
		return Collections.EMPTY_SET;
	}
	@Override
	public Set<BoolExpr> allCubes() {
		HashSet<BoolExpr> ret = new HashSet<BoolExpr>();
		ret.add(this);
		return ret;	
	}

}
