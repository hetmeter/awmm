package ags.constraints;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * Code written by Tal Lev Ami and Roman Manevich 
 * Adapted from TVLA code
 * EY: This class should not be open-sourced without permission!
 */
public class FormulaUtil {
	public static Formula distributeAndOverOr(Formula left, Formula right) {
		List<Formula> leftAnds = new ArrayList<Formula>();
		getAnds(left, leftAnds);
		List<Formula> rightAnds = new ArrayList<Formula>();
		getAnds(right, rightAnds);

		Formula result = null;
		for (Iterator<Formula> leftIt = leftAnds.iterator(); leftIt.hasNext();) {
			Formula leftFormula = leftIt.next();
			for (Iterator<Formula> rightIt = rightAnds.iterator(); rightIt
					.hasNext();) {
				Formula rightFormula = rightIt.next();
				if (result == null) {
					result = Formula.makeDisjunction(leftFormula, rightFormula);
				} else {
					result = Formula.makeConjunction(result, Formula
							.makeDisjunction(leftFormula, rightFormula));
				}
			}
		}
		return result;
	}

	public static void getAnds(Formula formula, Collection<Formula> ands) {
		LinkedList<Formula> workList = new LinkedList<Formula>();
		workList.add(formula);
		while (!workList.isEmpty()) {
			formula = workList.removeFirst();
			if (formula instanceof ConjunctionFormula) {
				ConjunctionFormula cformula = (ConjunctionFormula) formula;
				workList.addAll(cformula.getTerms());
			} else {
				ands.add(formula);
			}
		}
	}

	public static Formula distributeOrOverAnd(Formula left, Formula right) {
		List<Formula> leftOrs = new ArrayList<Formula>();
		getOrs(left, leftOrs);
		List<Formula> rightOrs = new ArrayList<Formula>();
		getOrs(right, rightOrs);
		Formula result = null;
		for (Iterator<Formula> leftIt = leftOrs.iterator(); leftIt.hasNext();) {
			Formula leftFormula = leftIt.next();
			for (Iterator<Formula> rightIt = rightOrs.iterator(); rightIt
					.hasNext();) {
				Formula rightFormula = rightIt.next();
				if (result == null) {
					result = Formula.makeConjunction(leftFormula, rightFormula);
				} else {
					Formula mc = Formula.makeConjunction(leftFormula,
							rightFormula);
					result = Formula.makeDisjunction(result, mc);
				}
			}
		}
		return result;
	}

	public static void getOrs(Formula formula, Collection<Formula> ors) {
		LinkedList<Formula> workList = new LinkedList<Formula>();
		workList.add(formula);
		while (!workList.isEmpty()) {
			formula = workList.removeFirst();
			if (formula instanceof DisjunctionFormula) {
				workList.addAll(formula.getTerms());
			} else {
				ors.add(formula);
			}
		}
	}
}
