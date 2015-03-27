package ags.constraints;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;

import net.sf.javabdd.BDD;
import net.sf.javabdd.BDDFactory;

/**
 * Represent a formula as BDD Adapted with permission from Roman Manevich
 * 
 * @author Roman Manevich
 * @author Eran Yahav
 */
public class BDDFormula extends Formula {

	private static BDDFactory factory;

	private static Map<Formula, BDD> atomToVar = new LinkedHashMap<Formula, BDD>();
	private static Map<BDD, Formula> varToAtom = new LinkedHashMap<BDD, Formula>();
	private BDD bdd;
	public static boolean lock;

	static {
		init();
	}

	private static void init() {
		factory = BDDFactory.init("j", 10000, 10000);
	}

	@SuppressWarnings("unchecked")
	public List<Set> getSolutions()
	{
		Object[] varArr = varToAtom.entrySet().toArray();
		List<Set> solutions = new LinkedList<Set>();
		List<byte[]> l = bdd.allsat();
		for (byte[] arr : l)
		{
			Set sol = new HashSet<Formula>();
			for(int i = 0; i < arr.length; i++)
			{
				if (arr[i] == 1)
					sol.add(((Entry<BDD,Formula>)varArr[i]).getValue());
			}
			solutions.add(sol);
		}
		return solutions;
	}
	
	@Override
	public Formula and(Formula other) {
		if (!(other instanceof BDDFormula))
			throw new RuntimeException("Oh, didn't see that coming");
		BDD otherBDD = ((BDDFormula) other).bdd;
		return new BDDFormula(bdd.and(otherBDD));
	}

	@Override
	public Formula deepCopy() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean implies(Formula other) {
		if (!(other instanceof BDDFormula))
			throw new RuntimeException("Oh, didn't see that coming");
		BDD otherBDD = ((BDDFormula) other).bdd;
		BDD imp = bdd.imp(otherBDD);
		boolean result = imp.isOne();
		System.out.println("Implies " + this + " =?=> " + other + " = " + result);
		return result;
	}

	@Override
	public Formula or(Formula other) {
		if (!(other instanceof BDDFormula))
			throw new RuntimeException("Oh, didn't see that coming " + other
					+ "," + other.getClass());
		BDD otherBDD = ((BDDFormula) other).bdd;
		return new BDDFormula(bdd.or(otherBDD));
	}

	@Override
	public int size() {
		// TODO Auto-generated method stub
		return 0;
	}

	/**
	 * Returns the set of atoms, i.e., formulae corresponding to BDD variables,
	 * that were registered so far. The order of iteration corresponds to the
	 * BDD variable ordering.
	 * 
	 * @return A set of formulae.
	 */
	public static Collection<Formula> getAtoms() {
		return Collections.unmodifiableCollection(atomToVar.keySet());
	}

	/**
	 * Registers this formula as a BDD variable. If this formula is a Boolean
	 * combination of simpler formulae they all get registered in some order.
	 * This method can be used to control the ordering of BDD variables by
	 * registering predicate formulae in the desired order.
	 * 
	 * @return true if new atoms were added.
	 */
	public static boolean addAtom(Formula f) {
		// f = FormulaHashCons.get(f);
		boolean change = !atomToVar.containsKey(f);
		if (change) {
			if (lock)
				throw new RuntimeException(
						"Attempting to add an un-registered formula to the BDD pool "
								+ f);
			int varNum = factory.varNum();
			factory.setVarNum(varNum + 1);
			BDD var = factory.ithVar(varNum);
			atomToVar.put(f, var);
			varToAtom.put(var, f);
		}
		return change;
	}

	public List<Formula> getCubes() {
		return getCubes(bdd);
	}

	public List<Formula> getRestrictedCubes(BDD b, BDD r, boolean polarity) {
		List<Formula> result = new ArrayList<Formula>();
		if (b.isZero()) {
		} else if (b.isOne()) {
			result.add(Formula.trueValue());
		} else {
			BDD bddVar = factory.ithVar(b.var());

			List<Formula> highCubes;
			if (bddVar != r || polarity) {
				BDD high = b.high();
				highCubes = getCubes(high);
				high.free();
			} else {
				highCubes = new ArrayList<Formula>();
			}

			List<Formula> lowCubes;
			if (bddVar != r || !polarity) {
				BDD low = b.low();
				lowCubes = getCubes(low);
				low.free();
			} else {
				lowCubes = new ArrayList<Formula>();
			}

			Formula atom = varToAtom.get(bddVar);
			Formula negAtom = new NotFormula(atom);
			for (int i = 0; i < highCubes.size(); ++i) {
				Formula cube = highCubes.get(i);
				if (bddVar != r) // skip over the restricted variable
					cube = and(atom, cube);
				result.add(cube);
			}
			for (int i = 0; i < lowCubes.size(); ++i) {
				Formula cube = lowCubes.get(i);
				if (bddVar != r) // skip over the restricted variable
					cube = and(negAtom, cube);
				result.add(cube);
			}
		}
		return result;
	}

	public List<Formula> getCubes(BDD b) {
		List<Formula> result = new ArrayList<Formula>();
		if (b.isZero()) {
		} else if (b.isOne()) {
			result.add(Formula.trueValue());
		} else {
			BDD low = b.low();
			List<Formula> lowCubes = getCubes(low);
			low.free();
			BDD high = b.high();
			List<Formula> highCubes = getCubes(high);
			high.free();
			BDD bddVar = factory.ithVar(b.var());
			Formula atom = varToAtom.get(bddVar);
			Formula negAtom = new NotFormula(atom);
			for (int i = 0; i < highCubes.size(); ++i) {
				Formula cube = highCubes.get(i);
				cube = and(atom, cube);
				result.add(cube);
			}
			for (int i = 0; i < lowCubes.size(); ++i) {
				Formula cube = lowCubes.get(i);
				cube = and(negAtom, cube);
				result.add(cube);
			}
		}
		return result;
	}

	public BDD getBDD() {
		return bdd;
	}

	public static BDD getBDD(Formula f) {
		BDDFormula bddF = new BDDFormula(f);
		return bddF.bdd;
	}

	// TODO: write a "deep"-simplification procedure that
	// recursively simplifies "complex" formulae bottom-up

	// public static Formula restructureHashCons(Formula f) {
	// Formula fHashConsed = FormulaHashCons.get(f, false);
	// Formula fSimplified = BDDFormula.simplify(fHashConsed);
	// fSimplified = FormulaHashCons.get(fSimplified, false);
	// return fSimplified;
	// }

	public static Formula simplify(Formula f) {
		BDDFormula bddF = new BDDFormula(f);
		// factory.reorder(BDDFactory.REORDER_RANDOM);
		// factory.reorder(BDDFactory.REORDER_WIN3ITE);
		Formula result = bddF.toFormula();
		return result;
	}

	public static BDD toBDD(Formula f) {
		BDDFormula bddF = new BDDFormula(f);
		return bddF.getBDD();
	}

	public void negate() {
		BDD orig = bdd;
		bdd = orig.not();
		orig.free();
	}

	@SuppressWarnings("unused")
	public BDDFormula restrict(Formula r, boolean polarity1, boolean polarity2) {
		BDD var = atomToVar.get(r);
		if (var == null) {
			throw new RuntimeException("Formula " + r
					+ " was not registered in the BDD pool!");
		}

		BDD arg1 = bdd;
		if (!polarity1) {
			BDD origArg1 = arg1;
			arg1 = arg1.not();
			// origArg1.free();
		}
		if (!polarity2) {
			BDD origVar = var;
			var = var.not();
			// origVar.free();
		}
		BDD origArg1 = arg1;
		BDD restricted = arg1.restrict(var);
		// origArg1.free();

		BDDFormula result = new BDDFormula(restricted);
		return result;
	}

	protected BDDFormula(BDD bdd) {
		this.bdd = bdd;
	}

	/**
	 * Constructs an internal BDD representation from the given formula.
	 * 
	 * @param f
	 *            An FOTC formula.
	 */
	public BDDFormula(Formula f) {
		// f = FormulaHashCons.get(f);
		Formula[] atoms = getAtoms(f);
		// Arrays.sort(atoms, AtomComplexityComparator.instance);
		for (Formula atom : atoms)
			addAtom(atom);
		bdd = constructBDD(f);
	}

	/**
	 * Converts the internal BDD in this object to a formula using an
	 * if-then-else style.
	 * 
	 * @return An FOTC formula.
	 */
	public Formula toFormula() {
		Formula result = getFormula(bdd);
		return result;
		// List<Formula> cubes = getCubes();
		// return Formula.balancedDisjunction(cubes);
	}

	/**
	 * Converts the internal BDD in this object to a DNF formula or 0, if the
	 * formula is false.
	 * 
	 * @return An FOTC formula.
	 */
	public Formula toDNFFormula() {
		List<Formula> cubes = getCubes();
		return new DisjunctionFormula(cubes);
	}

	/**
	 * Returns a text representation of the formula represented by the internal
	 * BDD object.
	 */
	@Override
	public String toString() {
		return toFormula().toString();
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((bdd == null) ? 0 : bdd.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof BDDFormula))
			return false;
		BDDFormula other = (BDDFormula) obj;
		return bdd.equals(other.bdd);
	}

	/**
	 * Returns an array of sub-formulae that are Boolean operations
	 * (and/or/not).
	 * 
	 * @param f
	 *            A formula.
	 * @return An array of formula that do not correspond to Boolean operations.
	 */
	protected static Formula[] getAtoms(Formula f) {
		final Set<Formula> atomSet = f.getPredicates();
		Formula[] fArr = new Formula[0];
		Formula[] result = atomSet.toArray(fArr);
		return result;
	}

	/**
	 * Converts a given BDD to an FOTC formula.
	 */
	public static Formula getFormula(BDD b) {
		Formula result;
		if (b.isOne()) {
			result = Formula.flatTrueValue();
		} else if (b.isZero()) {
			result = Formula.flatFalseValue();
		} else {
			BDD bVar = factory.ithVar(b.var());
			Formula bF = varToAtom.get(bVar);

			BDD l = b.low();
			BDD h = b.high();
			Formula lowF = getFormula(l);
			Formula highF = getFormula(h);
			l.free();
			h.free();

			if (lowF instanceof ValueFormula || highF instanceof ValueFormula) {
				// Construct (!bf & lowF) | (bf & highF)
				// optimizing away constant values.
				lowF = and(not(bF), lowF);
				highF = and(bF, highF);
				result = or(highF, lowF);
			} else {
				result = new IfFormula(bF, highF, lowF);
				// result = FormulaHashCons.get(result);
			}
		}
		// result = FormulaHashCons.get(result);
		return result;
	}

	protected static Formula not(Formula f) {
		if (f == Formula.falseValue())
			return Formula.trueValue();
		else if (f == Formula.trueValue())
			return Formula.falseValue();
		else if (f instanceof NotFormula)
			return ((NotFormula) f).subFormula();
		else
			return new NotFormula(f);
	}

	protected static Formula and(Formula l, Formula r) {
		if (l == Formula.falseValue() || r == Formula.falseValue())
			return Formula.falseValue();
		else if (l == Formula.trueValue())
			return r;
		else if (r == Formula.trueValue())
			return l;
		else
			return new ConjunctionFormula(l, r);
	}

	protected static Formula or(Formula l, Formula r) {
		if (l == Formula.trueValue() || r == Formula.trueValue())
			return Formula.trueValue();
		else if (l == Formula.falseValue())
			return r;
		else if (r == Formula.falseValue())
			return l;
		else
			return new DisjunctionFormula(l, r);
	}

	protected BDD constructBDD(Formula f) {
		BDD result = new FormulaTransformer() {
			private BDD temp;

			public BDD go(Formula f) {
				transform(f);
				return temp;
			}

			@Override
			public Formula accept(AtomicPredicate f) {
				temp = atomToVar.get(f);
				return f;
			}

			@Override
			public Formula accept(ConjunctionFormula f) {
				BDD left = factory.one();
				temp = left;
				for (Formula term : f.getTerms()) {
					term.visit(this);
					BDD right = temp;
					temp = left.and(right);
					left = temp;
				}
				return f;
			}

			@Override
			public Formula accept(DisjunctionFormula f) {
				BDD left = factory.zero();
				temp = left;
				for (Formula term : f.getTerms()) {
					term.visit(this);
					BDD right = temp;
					temp = left.or(right);
					left = temp;
				}
				return f;
			}

			@Override
			public Formula accept(NotFormula f) {
				f.subFormula().visit(this);
				temp = temp.not();
				return f;
			}

			@Override
			public Formula accept(ValueFormula f) {
				if (f.value() == Formula.trueValue())
					temp = factory.one();
				else if (f.value() == Formula.falseValue())
					temp = factory.zero();
				else {
					temp = atomToVar.get(f);
				}
				return f;
			}

			@Override
			public Formula accept(BitDisjunctionFormula f) {
				BDD left = factory.zero();
				temp = left;
				for (Formula term : f.getTerms()) {
					term.visit(this);
					BDD right = temp;
					temp = left.or(right);
					left = temp;
				}
				return f;
			}

		}.go(f);

		return result;
	}

	public static class AtomComplexityComparator implements Comparator<Formula> {
		public static final AtomComplexityComparator instance = new AtomComplexityComparator();

		public static final int NULLARY_ABS_PRED = 0;
		public static final int UNARY_UNIQUE_PRED = 0;
		public static final int UNARY_PRED = 0;
		public static final int EQUALITY = 0;
		public static final int BINARY_FUNCTION_PRED = 0;
		public static final int BINARY_PRED = 0;
		public static final int EXISTS_FORMULA = 0;
		public static final int FORALL_FORMULA = 0;
		public static final int TRANSITIVE_FORMULA = 0;
		public static final int UNKNOWN_KLEENE_CONST = 0;

		@Override
		public int compare(Formula arg0, Formula arg1) {
			return complexity(arg1) - complexity(arg0);
		}

		public int complexity(Formula f) {
			return 0;
		}
	}

	@Override
	public Set<Formula> getTerms() {
		throw new RuntimeException("NYI");
	}

	@Override
	public Formula visit(FormulaTransformer t) {
		throw new RuntimeException("NYI");
	}

	@Override
	public Set<Formula> getPredicates() {
		throw new RuntimeException("NYI");
	}

	public void setTrue() {
		System.out.println("setTrue on BDD " + this);
	}
}
