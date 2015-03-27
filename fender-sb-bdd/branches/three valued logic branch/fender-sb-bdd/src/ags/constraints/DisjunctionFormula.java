package ags.constraints;

import java.util.HashSet;
import java.util.List;

/**
 * See note on {@link CompositeFormula} 
 * 
 */
public class DisjunctionFormula extends CompositeFormula {

	public DisjunctionFormula() {
		super(true);
		terms.add(Formula.falseValue());
	}

	public DisjunctionFormula(Formula l, Formula r) {
		super(true);
		terms.add(l);
		terms.add(r);
	}

	public DisjunctionFormula(List<Formula> fl) {
		super(true);
		terms.addAll(fl);
	}

	@Override
	public boolean implies(Formula other) {
		if (other instanceof TrueFormula)
		{
			//Anything implies true
			return true;
		}
		//If one of the terms does not imply the other formula, then the whole disjunction doesn't.
		for (Formula f : terms) {
			if (!f.implies(other)) {
				return false;
			}
		}
		return true;
	}
	
	/**
	 * Look at Conjunction for explanation	
	 */
	public void add(Formula ap) {
		if (ap instanceof DisjunctionFormula)
		{
			DisjunctionFormula dis = (DisjunctionFormula)ap;
			for (Formula f: dis.getTerms())
				add(f);
			return;
		}
		
		HashSet<Formula> removeSet = new HashSet<Formula>(); 
		for (Formula f: terms)
		{
			if (f.implies(ap))
				removeSet.add(f);

			if (ap.implies(f))
				return;

		}
		terms.removeAll(removeSet);
		//TODO: Debugging only!
		//if (terms.size() < 3)
			terms.add(ap);
	}
	
	public DisjunctionFormula deepCopy()
	{
		DisjunctionFormula copy = new DisjunctionFormula();
		for (Formula f: terms)
			copy.add(f.deepCopy());
		
		return copy;
	}

	@Override
	public Formula or(Formula other) {
		if (other instanceof ValueFormula) {
			if (other == Formula.falseValue()) {
				return this;
			} else if (other == Formula.trueValue()) {
				return other;
			}
		}
		Formula result = this.deepCopy();
		((DisjunctionFormula)result).add(other);
		return result;
	}

	@Override
	public Formula and(Formula other) {
		if (other == Formula.trueValue()) 
			return this;
		if (other == Formula.falseValue())
			return other;
		
		Formula result = FormulaUtil.distributeOrOverAnd(this, other);
		System.out.println("AND RESULT: " + result);
		return result;
	}
	
	@Override
	public Formula visit(FormulaTransformer t) {
		return t.accept(this);
	}
	
	public void setTrue() {
		throw new RuntimeException("should never happen");
	}
}
