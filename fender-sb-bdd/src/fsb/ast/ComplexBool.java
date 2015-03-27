package fsb.ast;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import fsb.ast.BoolExpr.BXType;
import fsb.explore.State;
import fsb.tvl.BoolValue;
import fsb.utils.Options;
import gnu.trove.set.hash.THashSet;

public class ComplexBool extends BoolExpr {
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
		ComplexBool other = (ComplexBool) obj;
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

	private BoolExpr left, right;
	private BXType type;
	protected List<Integer> m_involvingVariables = null;

	public static THashSet<BoolExpr> m_usefullCubesGlobal = new THashSet<BoolExpr>();
	/**
	 * stores all cubes for evaluated conditions
	 */
	public static THashSet<BoolExpr> m_allCubesGlobal = new THashSet<BoolExpr>();
//	public static THashSet<BoolExpr> m_usefullCubesLocal = new THashSet<BoolExpr>();
//	public static THashSet<BoolExpr> m_allCubesLocal = new THashSet<BoolExpr>();

	public ComplexBool(BoolExpr left, BoolExpr right, BXType type) {
		this.left = left;
		this.right = right;
		this.type = type;
	}

	@Override
	protected BoolValue evaluate_inner(State s, int pid) {
		BoolValue leftval = left.evaluate_inner(s, pid);
		BoolValue rightval = right.evaluate_inner(s, pid);
//		if (Options.PRINT_USED_CUBES) {
//			switch (type) {
//			case AND: {
//				/*
//				 * if(leftval.isFalse()){ m_usefullCubes.add(left);}
//				 * if(rightval.isFalse()){ m_usefullCubes.add(right);}
//				 */}
//			case OR: {
//					if ((!(left instanceof ComplexBool))
//							|| ((ComplexBool) left).type != BXType.OR){
//						m_allCubesGlobal.add(left);
//						m_allCubesLocal.add(left);
//						if (leftval.isTrue()){
//							m_usefullCubesGlobal.add(left);
//							m_usefullCubesLocal.add(left);
//						}
//				}
//					if ((!(right instanceof ComplexBool))
//							|| ((ComplexBool) right).type != BXType.OR){
//						m_allCubesGlobal.add(right);
//						m_allCubesLocal.add(right);
//						if (rightval.isTrue()) {
//									m_usefullCubesGlobal.add(right);
//									m_usefullCubesLocal.add(right);
//					}
//				}
//				break;
//			}
//			default:
//				throw new RuntimeException(
//						"Invalid type for boolean expression!");
//			}
//		}
		return leftval.switchBooolAction(rightval, type);
	}

	public String toString() {
		String typeStr;
		switch (type) {
		case AND:
			typeStr = "&&";
			break;
		case OR:
			typeStr = "||";
			break;
		default:
			throw new RuntimeException("Invalid type for boolean expression!");
		}

		return "(" + left.toString() + " " + typeStr + " " + right.toString()
				+ ")";
	}

	@Override
	public List<Integer> getVariablesInvolved() {
		if (null == m_involvingVariables) {
			m_involvingVariables = left.getVariablesInvolved();
			m_involvingVariables.addAll(right.getVariablesInvolved());
		}
		return m_involvingVariables;
	}

	@Override
	public Set<BoolExpr> allSatisfiedCubes(State s, int pid) {
		if(isAND()){
			if(evaluate_inner(s,pid).isTrue()){
				HashSet<BoolExpr> ret = new HashSet<BoolExpr>();
				ret.add(this);
				return ret;
			}
			return Collections.EMPTY_SET;
		}else{
			HashSet<BoolExpr> ret = new HashSet<BoolExpr>();
			ret.addAll(left.allSatisfiedCubes(s, pid));
			ret.addAll(right.allSatisfiedCubes(s, pid));
			return ret;
		}
	}


	@Override
	public Set<BoolExpr> allNegationSatisfiedCubes(State s, int pid) {
		if(isAND()){
			if(evaluate_inner(s,pid).isFalse()){
				Set<BoolExpr> ret = new HashSet<BoolExpr>();
				ret.add(this);
				return ret;
			}
			return Collections.EMPTY_SET;
		}else{
			HashSet<BoolExpr> ret = new HashSet<BoolExpr>();
			ret.addAll(left.allNegationSatisfiedCubes(s, pid));
			ret.addAll(right.allNegationSatisfiedCubes(s, pid));
			return ret;
		}	
	}
	

	@Override
	public Set<BoolExpr> allCubes() {
		HashSet<BoolExpr> ret = new HashSet<BoolExpr>();
		if(isAND()){
			ret.add(this);
		}else{
			ret.addAll(left.allCubes());
			ret.addAll(right.allCubes());
		}
		return ret;
	}
	
	/**
	 * The proper name might be is conjunction. 
	 * but this is easier and faster to understand.
	 * 
	 * @return
	 */
private boolean isAND(){
	switch (type) {
	case AND:
		return true;
	case OR:
		return false;
	default:
		throw new RuntimeException("Invalid type for boolean expression!");
	}
}

}
