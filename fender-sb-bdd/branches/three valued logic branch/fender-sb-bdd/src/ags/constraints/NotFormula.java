package ags.constraints;

import java.util.Collections;
import java.util.Set;

public class NotFormula extends Formula {

	protected Formula subFormula;
	
	public NotFormula(Formula f) {
		subFormula = f;
	}

	@Override
	public Formula and(Formula other) {
		throw new RuntimeException("NYI");
	}

	@Override
	public Formula deepCopy() {
		throw new RuntimeException("NYI");
	}

	@Override
	public boolean implies(Formula other) {
		throw new RuntimeException("NYI");
	}

	@Override
	public Formula or(Formula other) {
		throw new RuntimeException("NYI");
	}

	@Override
	public int size() {
		throw new RuntimeException("NYI");
	}

	public Formula subFormula() {
		return subFormula;
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		return t.accept(this);
	}

	@Override
	public Set<Formula> getTerms() {
		return Collections.singleton(subFormula);
	}

	@Override
	public Set<Formula> getPredicates() {
		return subFormula.getPredicates();
	}

	public String toString() {
		return "!" + subFormula.toString();
	}
	
	public void setTrue() {
		throw new RuntimeException("should never happen");
	}
}
