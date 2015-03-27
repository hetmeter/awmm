package fsb.ast;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.management.RuntimeErrorException;

import fsb.ast.BoolExpr.BXType;
import fsb.explore.State;
import fsb.tvl.BoolValue;
import fsb.tvl.DetBoolValue;
import fsb.utils.Options;
import gnu.trove.set.hash.THashSet;

public class AssertComplexBool extends AssertBoolExpr {
	/**
	 * @return the type
	 */
	public BoolExpr.BXType getType() {
		return type;
	}
	private AssertBoolExpr left, right;
	private BoolExpr.BXType type;
	protected Set<Integer> m_involvingVariables = null;

	public static THashSet<AssertBoolExpr> m_usefullCubesGlobal = new THashSet<AssertBoolExpr>();
	/**
	 * stores all cubes for evaluated conditions
	 */
	public static THashSet<AssertBoolExpr> m_allCubesGlobal = new THashSet<AssertBoolExpr>();
//	public static THashSet<AssertBoolExpr> m_usefullCubesLocal = new THashSet<AssertBoolExpr>();
//	public static THashSet<AssertBoolExpr> m_allCubesLocal = new THashSet<AssertBoolExpr>();
	public AssertComplexBool(AssertBoolExpr left, AssertBoolExpr right,
			BoolExpr.BXType type) {
		this.left = left;
		this.right = right;
		this.type = type;
	}

@Override
public boolean isNegatedDNF() {
	return false;
}

public boolean isDNF(){
	boolean rightOk =false;
	boolean leftOK =false;
		switch (type) {
		case AND:
			
			rightOk = isConjPartOkForDNF(right);// if it is AssertConstBool or AssertComparisonBool or AssertComplexBool with a disjunciton 
			if(!rightOk){
				return false;
			}
			
			leftOK = isConjPartOkForDNF(left);		
			 return leftOK;
		case OR:
			rightOk = isDisjPartOkForDNF(right);// if it is AssertConstBool or AssertComparisonBool or AssertComplexBool with a disjunciton 
			if(!rightOk){
				return false;
			}
			
			leftOK =    isDisjPartOkForDNF(left);		
			 return leftOK;
		default:
			throw new RuntimeException("Invalid type for boolean expression!");
		}
	}

/**
 * @return
 */
private boolean isConjPartOkForDNF(AssertBoolExpr part) {
	// if left(or right) is a disjunction it is not a DNF
	// if it is a negation of something other then a const(or equality) it is not a DNF
	// if it is a conjunction it is ok only if it's part follow those rools
	// if it is a const or an equality it's ok
	boolean partOk = true;
	if(part instanceof AssertNotBool){
		partOk = ((AssertNotBool)part).isAP();
	}else if(part instanceof AssertComplexBool){
		AssertComplexBool assertComplexBoolPart = (AssertComplexBool)part;
		if(assertComplexBoolPart.type == BXType.OR){
			partOk = false;	
		}else{
			partOk = assertComplexBoolPart.isDNF();
		}
	
		
	}
	return partOk;
}
/**
 * @return
 */
private boolean isDisjPartOkForDNF(AssertBoolExpr part) {
	// if it is a negation of something other then a const(or equality) it is not a DNF
	// if it is a disjunction or a conjunction it is ok only if it's part follow those rools
	// if it is a const or an equality it's ok
	boolean partOk = true;
	if(part instanceof AssertNotBool){
		partOk = ((AssertNotBool)part).isAP();
		
	}else if(part instanceof AssertComplexBool){
		AssertComplexBool assertComplexBoolPart = (AssertComplexBool)part;
		partOk = assertComplexBoolPart.isDNF();
		
	}
	return partOk;
}
	@Override
	public boolean equals(Object obj) {
		if(! (obj instanceof AssertComplexBool) ){
			return false;
		}

		AssertComplexBool other = (AssertComplexBool)obj;
		
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
	public BoolValue evaluate_partVisb(State s) {
		BoolValue leftval = left.evaluate_partVisb(s);
		BoolValue rightval = right.evaluate_partVisb(s);
//		if (Options.PRINT_USED_CUBES) {
//			switch (type) {
//			case AND: {
//				/*
//				 * if(leftval.isFalse()){ m_usefullCubes.add(left);}
//				 * if(rightval.isFalse()){ m_usefullCubes.add(right);}
//				 */break;
//				 }
//			case OR: {
//					if ((!(left instanceof AssertComplexBool))
//							|| ((AssertComplexBool) left).type != BXType.OR){
//						m_allCubesGlobal.add(left);
//						m_allCubesLocal.add(left);
//						if (leftval.isTrue()) {
//									m_usefullCubesGlobal.add(left);
//									m_usefullCubesLocal.add(left);
//						}
//					}
//					if ((!(right instanceof AssertComplexBool))
//							|| ((AssertComplexBool) right).type != BXType.OR){
//						m_allCubesGlobal.add(right);
//						m_allCubesLocal.add(right);
//						if (rightval.isTrue()) {
//							m_usefullCubesGlobal.add(right);
//							m_usefullCubesLocal.add(right);
//						}
//					
//					
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
	public Set<Integer> getVariablesInvolved() {
		if (null == m_involvingVariables) {
			m_involvingVariables = left.getVariablesInvolved();
			m_involvingVariables.addAll(right.getVariablesInvolved());
		}
		return m_involvingVariables;
	}

	@Override
	public AssertBoolExpr assignConcreteValuesFromState(State s) {
//		int hash_val = this.hashCode()*17 + s.hashCode();
//		if(this.m_hashed_evaluations.containsKey(hash_val)){
//			return m_hashed_evaluations.get(hash_val);
//		}
		AssertBoolExpr newLeft = left.assignConcreteValuesFromState(s);
		AssertBoolExpr newRight = right.assignConcreteValuesFromState(s);
		if(newLeft instanceof AssertConstBool && newRight instanceof AssertConstBool){
			AssertBoolExpr b = new AssertConstBool(evaluate_partVisb(s));
			return returnGivenAssertBoolExprOrHashed(b);
		}
		switch (type) {
		case AND: {
			boolean isConjunction = true;
			if(newLeft instanceof AssertConstBool){
				return returnAccordingToConst(s, newRight, newLeft, isConjunction);
			}
			if(newRight instanceof AssertConstBool){
				return returnAccordingToConst(s, newLeft, newRight, isConjunction);
			}
			break;
			}
		case OR: {
			boolean isConjunction = false;
			if(newLeft instanceof AssertConstBool){
			return returnAccordingToConst(s, newRight, newLeft, isConjunction);
		}
		if(newRight instanceof AssertConstBool){
			return returnAccordingToConst(s, newLeft, newRight, isConjunction);
		}
		break;
		
		}
		default:
			throw new RuntimeException(
					"Invalid type for boolean expression!");
		}
		AssertBoolExpr b = new AssertComplexBool(newLeft, newRight, type);
		return returnGivenAssertBoolExprOrHashed(b);
	}


/**
 * performs short circuit evaluation<br>
 * if isDisjunction is true then checks if the constant is different from true and if so returns it.<br>
 * (so for true&&B will be returned B,<br> for *&&B - * will be returned <br>and for false&&B false returned.<br>
 * if isDisjunction is false then checks if the constant is different from false and if so returns it.<br>
 * (so for false||B - B will be returned,<br> for *||B - * will be returned <br>and for true||B -false returned.<br> 
 * @param s
 * @param secondPart
 * @param constantPart
 * @return
 */
private AssertBoolExpr returnAccordingToConst(State s,
		AssertBoolExpr secondPart, AssertBoolExpr constantPart, boolean iConjunction) {
	BoolValue constVal = constantPart.evaluate_partVisb(s);
	if(!constVal.equals(DetBoolValue.getInstance(iConjunction))){
		// returns the constant if non-determined
		// or it is a disjunction and the constant is true
		//or it is a conjunction and the constant is false
		return constantPart;
	}

	return secondPart;
}


@Override
public Set<AssertBoolExpr> allSatisfiedCubes(State s) {
	if(isAND()){
		if(evaluate_partVisb(s).isTrue()){
			HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
			ret.add(this);
			return ret;
		}
		return Collections.EMPTY_SET;
	}else{
		HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
		ret.addAll(left.allSatisfiedCubes(s));
		ret.addAll(right.allSatisfiedCubes(s));
		return ret;
	}
}

@Override
public Set<AssertBoolExpr> allNegationSatisfiedCubes(State s) {
	if(isAND()){
		if(evaluate_partVisb(s).isFalse()){
			HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
			ret.add(this);
			return ret;
		}
		return Collections.EMPTY_SET;
	}else{
		HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
		ret.addAll(left.allNegationSatisfiedCubes(s));
		ret.addAll(right.allNegationSatisfiedCubes(s));
		return ret;
	}	
}

@Override
public Set<AssertBoolExpr> allCubes() {
	HashSet<AssertBoolExpr> ret = new HashSet<AssertBoolExpr>();
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
	 * 
	 * @return
	 */
	public boolean isAND() {
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
