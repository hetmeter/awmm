package ags.constraints;

import java.util.Set;

/**
 * IF-THEN-ELSE formula (for ITE normal form coming out from BDD)
 * (!guard & low) | (guard & high) 
 * @author yahave
 *
 */
public class IfFormula extends Formula {

	protected Formula guard; 
	protected Formula high;
	protected Formula low;
	
	public IfFormula(Formula bF, Formula highF, Formula lowF) {
		guard = bF;
		high = highF;
		low = lowF;
	}

	@Override
	public Formula and(Formula other) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Formula deepCopy() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<Formula> getTerms() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean implies(Formula other) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public Formula or(Formula other) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int size() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<Formula> getPredicates() {
		// TODO Auto-generated method stub
		return null;
	}
	
	public String toString() {
		StringBuffer result = new StringBuffer();
		String gs = guard.toString();
		result.append(gs);
		result.append(" ? ");
		result.append(high);
		result.append(" : ");
		result.append(low);
		return result.toString();
	}

	public void setTrue() {
		throw new RuntimeException("should never happen");
	}
}
