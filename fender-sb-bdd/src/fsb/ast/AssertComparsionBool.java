package fsb.ast;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.BoolValue;

public class AssertComparsionBool extends AssertBoolExpr {	
	private AssertAritExpr left, right;
	private BoolExpr.BXType type;
	private Set<Integer>  m_involvingVariables = null;
	
	public AssertComparsionBool(AssertAritExpr left, AssertAritExpr right, BoolExpr.BXType type)
	{
		this.left = left;
		this.right = right;
		this.type = type;
	}
	
	public BoolValue evaluate_partVisb(State s)
	{
		ArithValue leftval = left.evaluate(s);
		ArithValue rightval = right.evaluate(s);
		
		return leftval.switchCmpr(rightval,type);
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
	public Set<Integer> getVariablesInvolved() {
		if(null == m_involvingVariables){
			m_involvingVariables = left.getVariablesInvolved();
			m_involvingVariables.addAll(right.getVariablesInvolved());
		}
		return m_involvingVariables;
	}
	@Override
	public boolean equals(Object obj) {
		if(! (obj instanceof AssertComparsionBool) ){
			return false;
		}

		AssertComparsionBool other = (AssertComparsionBool)obj;
		
		if(! other.left.equals(left))
		return false;
		
		if(! other.right.equals(right))
			return false;
		
		if(! other.type.equals(type))
			return false;
		
		return true;
	};
	@Override
	public int hashCode() {
		int ret = left.hashCode();
		
		ret = ret*17 + right.hashCode();
		
		ret = ret*17 + type.hashCode();
		
		return ret;
	}

	@Override
	public AssertBoolExpr assignConcreteValuesFromState(State s) {
		AssertAritExpr newLeft = left.assignConcreteValuesFromState(s);
		AssertAritExpr newRight = right.assignConcreteValuesFromState(s);
		
		if(newLeft instanceof AssertConstExpr && newRight instanceof AssertConstExpr){
			AssertConstExpr constValLeft = (AssertConstExpr)newLeft;
			AssertConstExpr constValRight = (AssertConstExpr)newRight;
			AssertBoolExpr b = new AssertConstBool(constValLeft.val.switchCmpr(constValRight.val, type));
			return returnGivenAssertBoolExprOrHashed(b);
		}
		AssertBoolExpr b = new AssertComparsionBool(newLeft, newRight, type);
		return returnGivenAssertBoolExprOrHashed(b);
	}
	
	@SuppressWarnings("unchecked")
	@Override
	public Set<AssertBoolExpr> allSatisfiedCubes(State s){
		
		if(evaluate_partVisb(s).isTrue()){
			HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
			ret.add(this);
			return ret;
		}
		return Collections.EMPTY_SET;
	}
	
	@SuppressWarnings("unchecked")
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
