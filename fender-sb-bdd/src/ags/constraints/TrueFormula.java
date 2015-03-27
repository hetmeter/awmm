package ags.constraints;


public class TrueFormula extends ValueFormula {

	@Override
	public int size() {
		return 1;
	}

	public String toString() {
		return "True";
	}

	@Override
	public boolean implies(Formula other) {
		if (!other.equals(this))
			return false;

		return true;
	}

	@Override
	public Formula and(Formula other) {
		return other;
	}

	@Override
	public Formula or(Formula other) {
		return this;
	}

	@Override
	public Formula value() {
		return Formula.trueValue();
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		return t.accept(this);
	}

}
