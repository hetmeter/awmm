package ags.constraints;

import java.util.Collections;
import java.util.Set;

/**
 * 
 * A LocationAtomicPredicate is of the form [id1, id2].
 * 
 * @author yahave Abused by MK
 * 
 */
@SuppressWarnings("unchecked")
public class LocationAtomicPredicate extends AtomicPredicate implements
		Comparable {
	private int lhsID;
	private int rhsID;

	protected LocationAtomicPredicate(int lhsLabel, int rhsLabel) {
		this.lhsID = lhsLabel;
		this.rhsID = rhsLabel;

	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + lhsID;
		result = prime * result + rhsID;
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		LocationAtomicPredicate other = (LocationAtomicPredicate) obj;
		if (lhsID != other.lhsID)
			return false;
		if (rhsID != other.rhsID)
			return false;
		return true;
	}

	public String toString() {
		return "[" + lhsID + "<" + rhsID + "]";
	}

	@Override
	public int size() {
		return 1;
	}

	@Override
	public boolean implies(Formula other) {
		if (other.equals(Formula.falseValue())) {
			return false;
		}
		if (other.equals(Formula.trueValue())) {
			return true;
		}
		if (other instanceof LocationAtomicPredicate) {
			return other.equals(this);
		}
		if (other instanceof DisjunctionFormula) {
			DisjunctionFormula df = (DisjunctionFormula) other;
			// return df.terms.contains(this);.
			for (Formula term : df.getTerms())
				if (implies(term))
					return true;
			return false;
		}
		if (other instanceof BitDisjunctionFormula) {
			BitDisjunctionFormula df = (BitDisjunctionFormula) other;
			System.out.println("trying to get " + this);
			TermDictionary dict = BitCompositeFormula.termDict;
			System.out.println("dict=" + dict);
			int index = dict.get(this);
			System.out.println("got index " + index);
			return (df.terms.get(index));
		}
		if (other instanceof ConjunctionFormula) {
			ConjunctionFormula cf = (ConjunctionFormula) other;
			for (Formula term : cf.getTerms())
				if (!implies(term))
					return false;
			return true;
		}
		// ignore complicated cases and just return false
		return false;
	}

	public int getLhsID() {
		return lhsID;
	}

	public int getRhsID() {
		return rhsID;
	}

	@Override
	public int compareTo(Object o) {
		LocationAtomicPredicate other = (LocationAtomicPredicate) o;
		if (lhsID < other.lhsID) {
			return -1;
		} else if (lhsID == other.lhsID) {
			if (rhsID < other.rhsID)
				return -1;
			else if (rhsID == other.rhsID)
				return 0;
		}
		return 1;
	}

	@Override
	public Formula or(Formula other) {
		return Formula.makeDisjunction(this, other);
	}

	@Override
	public Formula and(Formula other) {
		if (other == Formula.trueValue()) {
			return this;
		}
		return Formula.makeConjunction(this, other);
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		return t.accept(this);
	}

	@Override
	public Set<Formula> getTerms() {
		return Collections.singleton((Formula) this);
	}

	public Set<Formula> getPredicates() {
		return Collections.singleton((Formula) this);
	}

	public void setTrue() {
		throw new RuntimeException("should never happen");
	}
}
