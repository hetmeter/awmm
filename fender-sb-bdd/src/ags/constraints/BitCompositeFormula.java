package ags.constraints;

import java.util.HashSet;
import java.util.Set;

/**
 * @author MK
 * This class was not, as such, designed, but rather evolved from an earlier
 * version by EY. It is not consistent, and does not necessarily do what you'd expect
 * The same applies to its child classes. 
 * If you ever run into it - please, do not use and write a new one instead.
 */
public abstract class BitCompositeFormula extends Formula {
	protected BitSet terms;
	static protected TermDictionary termDict = new TermDictionary();

	boolean isFalse;
	boolean isTrue;
	
	private boolean isDisjunction;

	protected BitCompositeFormula(boolean isDisjunction) {
		terms = new BitSet();
		this.isDisjunction = isDisjunction;
	}
	
	public abstract void add(Formula ap);
	
	//Dumb remove
	public void remove(Formula f) {		
		terms.clear(termDict.get(f));
	}

	
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (isDisjunction ? 1231 : 1237);
		result = prime * result + terms.hashCode();
		return result;
	}

	//Note this checks equality, not equivalence.
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		BitCompositeFormula other = (BitCompositeFormula) obj;
		if (isDisjunction != other.isDisjunction)
			return false;
		if (terms == null) {
			if (other.terms != null)
				return false;
		} else if (!terms.equals(other.terms))
			return false;
		return true;
	}

	public String toString() {
		StringBuffer sb = new StringBuffer();

		sb.append("(");
		if (isFalse)
			sb.append("False");
		else if (isTrue)
			sb.append("True");
		else for (int i = terms.nextSetBit(0); i >= 0; i = terms.nextSetBit(i+1)) 
		{
			Formula f = termDict.reverseGet(i);
			sb.append(f);
			if (terms.nextSetBit(i+1) >= 0) {
				sb.append(isDisjunction ? "|" : "&");
			}
		}
		sb.append(")");
		return sb.toString();
	}


	@Override
	public int size() {
		return terms.size();
	}
	
	public boolean isEmpty() {
		return terms.isEmpty();
	}	
	
	
	public void setFalse()
	{
		isTrue = false;
		isFalse = true;
	}
	
	public void setTrue()
	{
		isTrue = true;
		isFalse = false;
	}
	
	public Set<Formula> getPredicates() {
		Set<Formula> result = new HashSet<Formula>();
		Set<Formula> ts = getTerms();
		for (Formula term : ts) {
			result.addAll(term.getPredicates());
		}
		return result;
	}
}
