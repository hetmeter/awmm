package fsb.ast;


import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.BoolValue;

public class NotBool extends BoolExpr {	
	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((expr == null) ? 0 : expr.hashCode());
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
		NotBool other = (NotBool) obj;
		if (expr == null) {
			if (other.expr != null)
				return false;
		} else if (!expr.equals(other.expr))
			return false;
		return true;
	}
	private BoolExpr expr;
	
	public NotBool(BoolExpr expr)
	{
		this.expr = expr;
	}
	
	protected BoolValue evaluate_inner(State s, int pid)
	{
		BoolValue exprval = expr.evaluate_inner(s, pid);
		return exprval.not();
	}
	
	public String toString()
	{
		return "!(" + expr.toString() + ")";
	}
	@Override
	public List<Integer> getVariablesInvolved() {
		return expr.getVariablesInvolved();
	}
	@Override
	public Set<BoolExpr> allSatisfiedCubes(State s, int pid){
		
		return expr.allNegationSatisfiedCubes(s, pid);
	}
	
	@Override
	public Set<BoolExpr> allNegationSatisfiedCubes(State s, int pid){
		return expr.allSatisfiedCubes(s, pid);
	}


	@Override
	public Set<BoolExpr> allCubes() {
		// TODO Auto-generated method stub
		return expr.allCubes();
	}
}
