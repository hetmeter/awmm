package fsb.ast;

import java.util.Set;

import fsb.explore.State;
import fsb.utils.Options;
import gnu.trove.set.hash.THashSet;

public class Assume extends Statement {

	public AssertBoolExpr cond;
	
	public Assume(AssertBoolExpr cond) {
		super(StatType.ASSUME);
		this.cond = cond;
	}

	public String toString()
	{
		String ret = "ASSUME " + cond; 
		
		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
			m_unUsedCubes.clear();
			m_unUsedCubes.addAll(m_allCubes);
			m_unUsedCubes.removeAll(m_usefullCubes);
			ret= "\nASSUME " + cond +"\nused: "+ m_usefullCubes +"\nall: "+ m_allCubes + "\nunused:" + m_unUsedCubes + "\n" ;
		}
		
		return ret;
	}

	@Override
	public boolean isLocal() {
		return false;
	}
	
	public Set<Integer> getVariablesInvolved() {
		return cond.getVariablesInvolved();
	}

	public Assume assignConcreteValuesFromState(State s){
		return new Assume(cond.assignConcreteValuesFromState(s));
	}
	public  THashSet<AssertBoolExpr> m_usefullCubes = new THashSet<AssertBoolExpr>();
	public  THashSet<AssertBoolExpr> m_allCubes = new THashSet<AssertBoolExpr>();
	public  THashSet<AssertBoolExpr> m_unUsedCubes = new THashSet<AssertBoolExpr>();

}
