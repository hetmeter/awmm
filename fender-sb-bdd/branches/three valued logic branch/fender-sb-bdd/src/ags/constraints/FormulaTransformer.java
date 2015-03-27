package ags.constraints;

public abstract class FormulaTransformer {

	public void transform(Formula f) {
		f.visit(this);
	}

	public abstract Formula accept(AtomicPredicate f);

	public abstract Formula accept(ConjunctionFormula f);

	public abstract Formula accept(DisjunctionFormula f);

	public abstract Formula accept(BitDisjunctionFormula f);

	public abstract Formula accept(NotFormula f);

	public abstract Formula accept(ValueFormula f);

}
