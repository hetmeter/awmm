package fsb.ast;


import java.util.Set;

import fsb.ast.BoolExpr.BXType;
import fsb.explore.State;
import fsb.tvl.BoolValue;

public class AssertNotBool extends AssertBoolExpr {	
	private AssertBoolExpr m_expr;
	
	public AssertNotBool(AssertBoolExpr expr)
	{
		this.m_expr = expr;
	}
	
	public BoolValue evaluate_partVisb(State s)
	{
		BoolValue exprval = m_expr.evaluate_partVisb(s);
		return exprval.not();
	}
	
	public String toString()
	{
		return "!(" + m_expr.toString() + ")";
	}
	@Override
	public Set<Integer> getVariablesInvolved() {
		return m_expr.getVariablesInvolved();
	}
	@Override
	public boolean equals(Object obj) {
		if(! (obj instanceof AssertNotBool) ){
			return false;
		}

		AssertNotBool other = (AssertNotBool)obj;
		
		if(! other.m_expr.equals(m_expr))
		return false;

		
		return true;
	};
	@Override
	public int hashCode() {
		int ret = 17 + m_expr.hashCode();
		
		return ret;
	}

	@Override
	public AssertBoolExpr assignConcreteValuesFromState(State s) {
		// TODO Auto-generated method stub
		AssertBoolExpr newExpr=m_expr.assignConcreteValuesFromState(s);
		if(newExpr instanceof AssertConstBool){
			AssertBoolExpr b = new AssertConstBool(newExpr.evaluate_partVisb(s).not());; 
			return returnGivenAssertBoolExprOrHashed(b);
		}
		AssertBoolExpr b = new AssertNotBool(newExpr);
		return returnGivenAssertBoolExprOrHashed(b);
	}

	public boolean isAP() {
		if(m_expr instanceof AssertNotBool){
			return ((AssertNotBool)m_expr).isAP();
		}
		return ((m_expr instanceof AssertConstBool) || (m_expr instanceof AssertComparsionBool));
	}

	@Override
	public boolean isNegatedDNF() {
		if(m_expr instanceof AssertComplexBool){
			AssertComplexBool complexAssetExpr = (AssertComplexBool)m_expr;
			return (complexAssetExpr.isDNF() && complexAssetExpr.getType() == BXType.OR) ;
		}
		return false;
	}

	@Override
	public Set<AssertBoolExpr> allSatisfiedCubes(State s){
		
		return m_expr.allNegationSatisfiedCubes(s);
	}
	
	@Override
	public Set<AssertBoolExpr> allNegationSatisfiedCubes(State s){
		return m_expr.allSatisfiedCubes(s);
	}
	@Override
	public Set<AssertBoolExpr> allCubes(){
		return m_expr.allCubes();
	}
}
