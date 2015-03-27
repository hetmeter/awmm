package ags.constraints;

import java.util.Collections;
import java.util.Set;

public abstract class ValueFormula extends AtomicPredicate {
	public abstract Formula value();
	public Set<Formula> getTerms() {
		return Collections.singleton(this.value());
	}
	
	@SuppressWarnings("unchecked")
	public Set<Formula> getPredicates() {
		return Collections.EMPTY_SET;
	}
	
	public void setTrue() {
		throw new RuntimeException("should never happen");
	}
}
