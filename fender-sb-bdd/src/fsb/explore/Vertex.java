package fsb.explore;

import java.util.WeakHashMap;

import ags.constraints.Formula;

public class Vertex {
	private State state;
	private int weight;
	private boolean isVisited = false;
	private boolean isSafe = false;
	public int formulaChanged = 0;

	private static WeakHashMap<Formula, Formula> FormulaDictionary = new WeakHashMap<Formula, Formula>();
	public static Formula initStateFormula;
	private static Formula InitialFormula; // = Formula.makeConjunction();
	// private static Formula trueDisjunction = Formula.makeDisjunction();
	// private static DisjunctionFormula falseDisjunction = new
	// DisjunctionFormula();
	// private static DisjunctionFormula trueDisjunction = new
	// DisjunctionFormula();

	static {
		initStateFormula = Formula.falseValue(); // initStateFormula.and(falseDisjunction);
		// trueDisjunction.add(Formula.TRUE_FORMULA);
		// trueDisjunction.setTrue();
		// InitialFormula = InitialFormula.and(trueDisjunction);
		InitialFormula = Formula.trueValue();
	}

	protected Vertex(State state, int weight) {
		this.state = state;
		this.weight = weight;
		f = InitialFormula;
	}

	public static void clearCaches() {
		FormulaDictionary.clear();
	}

	public State getState() {
		return state;
	}

	@Override
	public String toString() {
		return state.toString() + f.toString();
	}

	public int getWeight() {
		return weight;
	}

	protected void reduceWeight(int newWeight) {
		if (weight > newWeight)
			weight = newWeight;
	}

	private Formula f;

	public Formula getFormula() {
		return f;
	}

	public void setFormula(Formula f) {
		formulaChanged++;
		Formula inDictionary = FormulaDictionary.get(f);
		if (inDictionary != null) {
			this.f = inDictionary;
		} else {
			FormulaDictionary.put(f, f);
			this.f = f;
		}
	}

	public void setVisited(boolean isVisited) {
		this.isVisited = isVisited;
	}

	public boolean isVisited() {
		return isVisited;
	}

	public void setSafe(boolean isSafe) {
		this.isSafe = isSafe;
	}

	public boolean isSafe() {
		return isSafe;
	}
}
