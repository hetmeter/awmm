package ags.constraints;

import java.util.Set;

public abstract class Formula {
	private static final Formula TRUE_FORMULA = new TrueFormula();
	private static final Formula FALSE_FORMULA = new FalseFormula();
	private static final Formula BDD_TRUE_FORMULA = new BDDFormula(
			Formula.TRUE_FORMULA);
	private static final Formula BDD_FALSE_FORMULA = new BDDFormula(
			Formula.FALSE_FORMULA);
	private static final boolean USE_BDD = true;

	public static final boolean DEBUG = false;

	public abstract boolean implies(Formula other);

	public abstract Formula deepCopy();

	public abstract int size();

	public abstract Formula or(Formula other);

	public abstract Formula and(Formula other);

	public abstract void setTrue();

	public abstract Formula visit(FormulaTransformer t);

	public abstract Set<Formula> getTerms();

	public abstract Set<Formula> getPredicates();

	public static Formula flatTrueValue() {
		return TRUE_FORMULA;
	}

	public static Formula flatFalseValue() {
		return FALSE_FORMULA;
	}

	public static Formula trueValue() {
		return USE_BDD ? BDD_TRUE_FORMULA : TRUE_FORMULA;
	}

	public static Formula falseValue() {
		return USE_BDD ? BDD_FALSE_FORMULA : FALSE_FORMULA;
	}

	public static Formula makeDisjunction() {
		if (USE_BDD) {
			return new BDDFormula(Formula.FALSE_FORMULA);
		} else {
			// return new BitDisjunctionFormula();
			return new DisjunctionFormula();
		}

	}

	public static Formula makeConjunction() {
		if (USE_BDD) {
			return new BDDFormula(Formula.TRUE_FORMULA);
		} else {
			return new ConjunctionFormula();
		}

	}

	public static boolean useBDD() {
		return USE_BDD;
	}

	public static Formula makeConjunction(Formula l, Formula r) {
		Formula result = makeConjunction();
		result = result.and(l);
		result = result.and(r);
		return result;
	}

	public static Formula makeDisjunction(Formula l, Formula r) {
		Formula result = makeDisjunction();
		result = result.or(l);
		result = result.or(r);
		return result;
	}
}
