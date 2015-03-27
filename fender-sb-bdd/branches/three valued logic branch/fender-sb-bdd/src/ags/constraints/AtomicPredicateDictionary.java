package ags.constraints;

import java.util.TreeMap;


//Yes, this is ugly. Should be a singleton, etc...
//As usual, to conserve memory, we use a cache of atomic predicate objects.
public class AtomicPredicateDictionary {

	//A service class, each AP is identified by a left hand side and a right hand side.
	private static class Coords implements Comparable<Coords>
	{
		public Coords(int lhs, int rhs)
		{
			this.lhs = lhs;
			this.rhs = rhs;
		}	
		
		public int lhs;
		public int rhs;
		
		@Override
		public boolean equals(Object o) 
		{
		     if (this == o) return true;
		     if (!(o instanceof Coords)) return false;
		     
		     Coords other = (Coords) o;
		     return (other.lhs == lhs && other.rhs == rhs);
		}
		     
		     
		@Override
		public int compareTo(Coords o) {
			if (lhs < o.lhs)
				return -1;
			
			if (lhs > o.lhs)
				return 1;
			
			if (rhs < o.rhs)
				return -1;
			
			if (rhs > o.rhs)
				return 1;
			
			return 0;
		}
		
	}	
	
	//TODO: Override hashCode, if you want to use a hashmap instead of a treemap for the cache.
	private static TreeMap<Coords, AtomicPredicate> predicates;

	static
	{
		predicates = new TreeMap<Coords, AtomicPredicate>();
	}

	public static Formula makeAtomicPredicate(int lhsLabel, int rhsLabel) {
		Coords coords = new Coords(lhsLabel, rhsLabel);
		Formula ap = predicates.get(coords);
		if(ap == null)
		{
			ap = AtomicPredicateFactory.makeAtomicPredicate(lhsLabel, rhsLabel);
			predicates.put(coords, (AtomicPredicate)ap);
		}

		if (Formula.useBDD()) {
			return new BDDFormula(ap);
		} else {
			return ap;
		}

	}
}
