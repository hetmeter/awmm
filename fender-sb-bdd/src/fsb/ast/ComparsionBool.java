package fsb.ast;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.BoolValue;

public class ComparsionBool extends BoolExpr {	
	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((left == null) ? 0 : left.hashCode());
		result = prime * result + ((right == null) ? 0 : right.hashCode());
		result = prime * result + ((type == null) ? 0 : type.hashCode());
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
		ComparsionBool other = (ComparsionBool) obj;
		if (left == null) {
			if (other.left != null)
				return false;
		} else if (!left.equals(other.left))
			return false;
		if (right == null) {
			if (other.right != null)
				return false;
		} else if (!right.equals(other.right))
			return false;
		if (type != other.type)
			return false;
		return true;
	}
	private AritExpr left, right;
	private BXType type;
	
	private List<Integer>  m_involvingVariables = null;
	public ComparsionBool(AritExpr left, AritExpr right, BXType type)
	{
		this.left = left;
		this.right = right;
		this.type = type;
	}
	
	protected BoolValue evaluate_inner(State s, int pid)
	{
		ArithValue leftval = left.evaluate(s, pid);
		ArithValue rightval = right.evaluate(s, pid);
		
		return leftval.switchCmpr(rightval, type);
	}
	
	public String toString()
	{
		String typeStr;
		switch (type)
		{
			case EQUAL:
				typeStr = "==";
				break;
			case NEQ:
				typeStr = "!=";
				break;
			case GREATER:
				typeStr = ">";
				break;
			case LESS:
				typeStr = "<";
				break;
			default:
				throw new RuntimeException("Invalid type for boolean expression!");
		}	
		
		return "(" + left.toString() + " " + typeStr + " " + right.toString() + ")";
	}
	@Override
	public List<Integer> getVariablesInvolved() {
		if(null == m_involvingVariables){
			m_involvingVariables = left.getVariablesInvolved();
			m_involvingVariables.addAll(right.getVariablesInvolved());
		}
		return m_involvingVariables;
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
