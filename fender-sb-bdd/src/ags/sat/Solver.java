package ags.sat;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.sat4j.core.VecInt;
import org.sat4j.minisat.SolverFactory;
import org.sat4j.specs.ContradictionException;
import org.sat4j.specs.ISolver;
import org.sat4j.specs.TimeoutException;
import org.sat4j.tools.ModelIterator;

import ags.constraints.AtomicPredicate;
import ags.constraints.ConjunctionFormula;
import ags.constraints.DisjunctionFormula;
import ags.constraints.Formula;
import ags.constraints.TrueFormula;


public class Solver {

	private Map<AtomicPredicate, Integer> predToVar;
	private Map<Integer, AtomicPredicate> varToPred;

	public Solver() {
		predToVar = new HashMap<AtomicPredicate, Integer>();
		varToPred = new HashMap<Integer, AtomicPredicate>();
	}

	private void computeVarMaps(ConjunctionFormula cnf) {
		// start at 1 for sat4j disallowing 0
		int index = 1;
		for (Formula f : cnf.getTerms()) {
			if (f instanceof TrueFormula)
				continue;
			if (!(f instanceof DisjunctionFormula)) {
				throw new RuntimeException("Expected CNF");
			}
			DisjunctionFormula df = (DisjunctionFormula) f;
			for (Formula term : df.getTerms()) {
				if (!(term instanceof AtomicPredicate)) {
					throw new RuntimeException("Expected CNF");
				}
				AtomicPredicate ap = (AtomicPredicate) term;
				if (!predToVar.containsKey(term)) {
					Integer var = new Integer(index++);
					predToVar.put(ap, var);
					varToPred.put(var, ap);
				}
			}
		}
	}

	public List<Formula> solve(ConjunctionFormula cnf) {

		ISolver solver = SolverFactory.newDefault();

		computeVarMaps(cnf);

		int maxVar = predToVar.size();
		int nbClauses = cnf.size();
		solver.newVar(maxVar);
		solver.setExpectedNumberOfClauses(nbClauses);

		for (Formula f : cnf.getTerms()) {
			if (f instanceof TrueFormula)
				continue;
			if (!(f instanceof DisjunctionFormula)) {
				throw new RuntimeException("Expected CNF");
			}
			DisjunctionFormula df = (DisjunctionFormula) f;
			int[] vectorClause = getClause(df, predToVar);
			try {
				solver.addClause(new VecInt(vectorClause));
			} catch (ContradictionException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		int shortest = Integer.MAX_VALUE;
		
		System.out.println("--Enumerating Models--");
		Map<Integer,List<Model>> results = new HashMap<Integer,List<Model>>();
		ISolver mi = new ModelIterator(solver);
		try {
			boolean unsat = true;
			while (mi.isSatisfiable()) {
				unsat = false;
				int[] model = mi.model();
				Formula result = interpretModel(model);
				int length = result.size();
				List<Model> modelsPerLength = results.get(length);
				if (modelsPerLength == null) {
					modelsPerLength = new ArrayList<Model>();
					results.put(length, modelsPerLength);
				}
				modelsPerLength.add(new Model(model));

				//TODO: DEBUG, Remove
				if (length <= shortest)
				{
					System.out.println("Shortest length is " + length + " with " + modelsPerLength.size() + " assignments.");
					shortest = length;
				}
				
				if (unsat) {
					System.out.println("I see bad things arising - non satisfiable");
				}
			}
		} catch (TimeoutException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

		System.out.println("Map Size: " + results.size());
		int length = 0;
		List<Model> modelsPerLength = results.get(length);
		List<Formula> solutions = new LinkedList<Formula>();
		while(modelsPerLength == null) {
			length++;
			modelsPerLength = results.get(length);
		}
		for (Model m : modelsPerLength) {
			Formula result = interpretModel(m.model);
			System.out.println("Result: " + result);
			solutions.add(result);
		}
		
		return solutions;
	
		/*System.out.println("--All solutions--");
		for (List<Model> r : results.values()) {
			for (Model m : r ) {
				Formula result = interpretModel(m.model);
				System.out.println("(" + result.size() + ") " + "Result: " + result);
			}
		}*/

		
	}

	private int[] getClause(DisjunctionFormula df,
			Map<AtomicPredicate, Integer> varMap) {
		int index = 0;
		int[] result = new int[df.size()];
		for (Formula f : df.getTerms()) {
			if (!(f instanceof AtomicPredicate)) {
				throw new RuntimeException("Expected AtomicPredicate");
			}
			AtomicPredicate ap = (AtomicPredicate) f;
			int var = varMap.get(ap);
			result[index++] = var;
		}
		return result;
	}

	private Formula interpretModel(int[] model) {
		Formula cf = Formula.makeConjunction();
		for (int i = 0; i < model.length; i++) {
			int var = model[i];
			if (var > 0) {
				AtomicPredicate ap = varToPred.get(var);
				cf = cf.and(ap);
			}
			//System.out.print(model[i] + ",");
		}
		//System.out.println(cf);
		return cf;
	}

}
