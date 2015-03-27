package ags.constraints;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

//See note on BitCompositeFormula
public class BitDisjunctionFormula extends BitCompositeFormula {
	protected void addToTerms(Formula f) {
		int index;
		if (!(f instanceof LocationAtomicPredicate))
			throw new RuntimeException(
					"Can only add atomic terms to bitset disjunctions! " + f.getClass());
		if (termDict.isIn(f)) {
			index = termDict.get(f);
		} else {
			index = termDict.add(f);
		}
		terms.set(index);
	}

	protected BitDisjunctionFormula() {
		super(true);
		isFalse = true;
	}

	@Override
	public boolean implies(Formula other) {
		if ((other instanceof TrueFormula) || isFalse) {
			// Anything implies true
			return true;
		}

		if (other instanceof FalseFormula) {
			return false;
		}

		if (other instanceof LocationAtomicPredicate) {
			int index = termDict.get(other);
			return (terms.get(index));
		}

		if (other instanceof BitDisjunctionFormula) {
			BitDisjunctionFormula dis = (BitDisjunctionFormula) other;
			if (dis.isTrue)
				return true;
			return dis.terms.containsAll(terms);

			// BitSet termsCopy = (BitSet) terms.clone();
			// termsCopy.andNot(dis.terms);
			// return termsCopy.isEmpty();

		}

		throw new RuntimeException(
				"This is not the implication you are looking for!");
	}

	/**
	 * To falsify the model, all disjuncts must falsify it.
	 */
	public boolean falsified(Collection<AtomicPredicate> model) {
		throw new RuntimeException("Should not be used!");
	}

	/**
	 * Lots of ugly hacks
	 */
	public void add(Formula ap) {
		if (isTrue)
			return;

		if (ap instanceof TrueFormula) {
			isFalse = false;
			isTrue = true;
			return;
		}

		if (ap instanceof BitDisjunctionFormula) {
			BitDisjunctionFormula dis = (BitDisjunctionFormula) ap;
			if (dis.isTrue) {
				isFalse = false;
				isTrue = true;
				return;
			}

			if (dis.isFalse) {
				return;
			}

			isFalse = false;
			terms.or(dis.terms);
			return;
		}

		isFalse = false;
		addToTerms(ap);
	}

	@Override
	public BitDisjunctionFormula deepCopy() {
		BitDisjunctionFormula copy = new BitDisjunctionFormula();
		if (isFalse) {
			copy.setFalse();
			return copy;
		}

		copy.isFalse = false;
		if (isTrue)
			copy.setTrue();
		copy.terms.or(terms);
		return copy;
	}

	@Override
	public Formula or(Formula other) {
		System.out.println("BDF OR " + other);
		if (other instanceof ValueFormula) {
			if (other == Formula.falseValue()) {
				return this;
			} else if (other == Formula.trueValue()) {
				return other;
			}
		}
		Formula result = this.deepCopy();
		((BitDisjunctionFormula) result).add(other);
		return result;
	}

	@Override
	public Formula and(Formula other) {
		throw new RuntimeException("NYI" + other + "," + other.getClass());
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		return t.accept(this);
	}

	@Override
	public Set<Formula> getTerms() {
		Set<Formula> result = new HashSet<Formula>(3);
		for (int i = terms.nextSetBit(0); i >= 0; i = terms.nextSetBit(i + 1)) {
			result.add(termDict.reverseGet(i));
		}
		return result;
	}
}
