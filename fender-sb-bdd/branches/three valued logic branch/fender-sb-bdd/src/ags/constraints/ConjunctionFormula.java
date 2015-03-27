package ags.constraints;

import java.util.HashSet;
import java.util.Set;

/**
 * See note on {@link CompositeFormula} 
 * 
 */
public class ConjunctionFormula extends CompositeFormula {

	/**
	 * An empty conjunction is true
	 */
	protected ConjunctionFormula() {
		super(false);
		//terms.add(Formula.trueValue());
	}

	/**
	 * Build a conjunction out of a set of conjuncts
	 */
	protected ConjunctionFormula(Set<Formula> t) {
		super(false);
		assert t.size() > 1;
		for (Formula f : t)
			terms.add(f);
	}

	protected ConjunctionFormula(Formula l, Formula r) {
		super(false);
		terms.add(l);
		terms.add(r);
	}

	/**
	 * Does not do anything smart - if one of the conjuncts implies the other
	 * formula, then the whole conjunction does.
	 */
	@Override
	public boolean implies(Formula other) {
		for (Formula f : terms) {
			if (f.implies(other)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Add a new term to the conjunction.
	 */
	private void add(Formula ap) {
		// If the new term is itself a conjunction, try to add each of the terms
		// individually - we ant (A & B) & (C & D) to become (A & B & C & D),
		// plus this will remove implied terms.
		if (ap instanceof ConjunctionFormula) {
			ConjunctionFormula con = (ConjunctionFormula) ap;
			for (Formula f : con.getTerms())
				add(f);
			return;
		}

		// The formula is either an atomic one or a disjunction.
		// Add it, and remove implied terms.
		HashSet<Formula> removeSet = new HashSet<Formula>();
		for (Formula f : terms) {
			// Unless of course the new formula is implied by one of the
			// existing terms
			if (f.implies(ap))
				return;

			if (ap.implies(f))
				removeSet.add(f);

		}
		terms.removeAll(removeSet);
		terms.add(ap);
	}

	private ConjunctionFormula orAP(AtomicPredicate ap) {
		ConjunctionFormula result = this.deepCopy();
		Formula newdis = Formula.makeDisjunction();
		newdis = newdis.or(ap);
		System.out.println("NEWDIS: " + newdis);
		result.add(newdis);
		return result;
	}

	private ConjunctionFormula orDF(DisjunctionFormula dis) {
		ConjunctionFormula result = this.deepCopy();
		System.out.println("orDF: " + result.getTerms().size() + ":" + result.getTerms());
		for (Formula t : result.getTerms()) {
			if (t instanceof AtomicPredicate) {
				Formula newdis = dis.deepCopy();
				newdis = newdis.or(t);
				result.add(newdis);
			} else {
				DisjunctionFormula dt = (DisjunctionFormula) t;
				dt.add(dis);
			}
		}
		return result;
	}

	private ConjunctionFormula orBDF(BitDisjunctionFormula dis) {
		ConjunctionFormula result = this.deepCopy();
		for (Formula t : result.getTerms()) {
			if (t instanceof AtomicPredicate) {
				Formula newdis = dis.deepCopy();
				newdis = newdis.or(t);
				result.add(newdis);
			} else {
				BitDisjunctionFormula dt = (BitDisjunctionFormula) t;
				dt.add(dis);
			}
		}
		return result;
	}

	public ConjunctionFormula deepCopy() {
		ConjunctionFormula copy = new ConjunctionFormula();
		copy.terms.clear();
		for (Formula f : terms)
			copy.terms.add(f.deepCopy());

		/*
		 * for (Formula f: terms) copy.add(f.deepCopy());
		 */
		return copy;

	}

	public ConjunctionFormula orConj(ConjunctionFormula conj) {
		ConjunctionFormula newFormula = new ConjunctionFormula();

		// Optimize the common case

		if (isFalse())
			return conj.deepCopy();
		if (conj.isFalse())
			return deepCopy();

		for (Formula f1 : terms) {
			for (Formula f2 : conj.terms) {
				if (f1 == Formula.trueValue()) {
					newFormula.add(f2);
				} else {
					Formula df1 = f1.deepCopy();
					df1.or(f2);
					newFormula.add(df1);
				}
			}
		}
		return newFormula;
	}

	boolean isFalse() {
		if (terms.size() != 1)
			return false;

		Formula f = terms.iterator().next();
		if (f instanceof FalseFormula)
			return true;

		if (f instanceof BitDisjunctionFormula)
			return ((BitDisjunctionFormula) f).isFalse;

		return false;
	}

	@Override
	public Formula and(Formula other) {
		Formula result = this.deepCopy();
		if (other == Formula.trueValue()) {
			return result;
		} else if (other == Formula.falseValue()) {
			return other;
		}

		((ConjunctionFormula) result).add(other);
		return result;
	}

	@Override
	public Formula or(Formula other) {
		if (other instanceof ValueFormula) {
			if (other == Formula.trueValue())
				return other;
			else if (other == Formula.falseValue())
				return this;
		} else if (other instanceof ConjunctionFormula) {
			return orConj((ConjunctionFormula) other);
		} else if (other instanceof DisjunctionFormula) {
			return orDF((DisjunctionFormula) other);
		} else if (other instanceof BitDisjunctionFormula) {
			return orBDF((BitDisjunctionFormula) other);
		} else if (other instanceof AtomicPredicate) {
			return orAP((AtomicPredicate) other);
		}

		throw new RuntimeException("NYI " + other.getClass());
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		return t.accept(this);
	}

	public void setTrue() {
		throw new RuntimeException("should never happen");
	}
}
