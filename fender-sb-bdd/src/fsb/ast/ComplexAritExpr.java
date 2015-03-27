package fsb.ast;

import java.util.List;

import fsb.explore.State;
import fsb.tvl.ArithValue;

public class ComplexAritExpr implements AritExpr {
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
		ComplexAritExpr other = (ComplexAritExpr) obj;
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

	AritExpr left, right;
	AXType type;
	
	protected List<Integer> m_involvingVariables=null;
	
	public ComplexAritExpr(AritExpr l, AritExpr r, AXType type) {
		this.left = l;
		this.right = r;
		this.type = type; 
	}

	public ArithValue evaluate(State s, int pid) {
		ArithValue leftval = left.evaluate(s, pid);
		ArithValue rightval = right.evaluate(s, pid);
		
		return leftval.switchOp(rightval, type);
	}	
	
	public String toString()
	{
		String typeStr;
		switch (type)
		{
			case PLUS:
				typeStr = "+";
				break;
			case MINUS:
				typeStr = "-";
				break;
			case MUL:
				typeStr = "*";
				break;
			default:
				throw new RuntimeException("Invalid type for arithmetic expression!");
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
}
