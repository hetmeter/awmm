package ags.constraints;


public class FalseFormula extends ValueFormula {

	@Override
	public int size() {
		return 1;
	}

	public String toString() {
		return "False";
	}

	@Override
	public boolean implies(Formula other) {
		return true;
	}

	@Override
	public Formula and(Formula other) {
		return this;
	}

	@Override
	public Formula or(Formula other) {
		return other;
	}

	@Override
	public Formula value() {
		return Formula.falseValue();
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		return t.accept(this);
	}
}
