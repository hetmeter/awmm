package ags.constraints;

import java.util.HashSet;
import java.util.Set;

/**
 * @author MK
 * This class was not, as such, designed, but rather evolved from an earlier
 * version by EY. It is not consistent, and does not necessarily do what you'd expect
 * The same applies to its children. 
 * If you ever run into it - please, do not use and write a new one instead.
 */
public abstract class CompositeFormula extends Formula {
	protected Set<Formula> terms;
	private boolean isDisjunction;

	protected CompositeFormula(boolean isDisjunction) {
		terms = new HashSet<Formula>();
		this.isDisjunction = isDisjunction;
	}
	
	//Dumb remove
	public void remove(Formula f) {
		terms.remove(f);
	}

	
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (isDisjunction ? 1231 : 1237);
		result = prime * result + ((terms == null) ? 0 : terms.hashCode());
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
		CompositeFormula other = (CompositeFormula) obj;
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

		int size = terms.size();
		int i = 1;
		sb.append("(");
		for (Formula f : terms) {
			sb.append(f);
			if (i++ < size) {
				sb.append(isDisjunction ? "|" : "&");
			}
		}
		sb.append(")");
		return sb.toString();
	}

	public Set<Formula> getTerms() {
		return terms;
	}
	

	@Override
	public int size() {
		return terms.size();
	}
	
	public boolean isEmpty() {
		return terms.isEmpty();
	}	
	
	public Set<Formula> getPredicates() {
		Set<Formula> result = new HashSet<Formula>();
		for (Formula term : terms) {
			result.addAll(term.getPredicates());
		}
		return result;
	}
}
