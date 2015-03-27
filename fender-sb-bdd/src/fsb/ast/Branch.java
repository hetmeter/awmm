package fsb.ast;

import java.util.HashSet;

import fsb.utils.Options;
import gnu.trove.set.hash.THashSet;

public class Branch extends Statement {

	public BoolExpr cond;
	public int toLabel;
	
	public Branch(BoolExpr cond, int toLabel) {
		super(StatType.BRANCH);
		this.cond = cond;
		this.toLabel = toLabel;
	}

	public String toString()
	{
		String ret ="IF " + cond + " GOTO " + toLabel ;
		if (Options.PRINT_USED_CUBES_PER_STATEMENT) {
			m_unUsedCubes.clear();
			m_unUsedCubes.addAll(m_allCubes);
			m_unUsedCubes.removeAll(m_usefullCubes);
			ret = "\nIF " + cond + " GOTO " + toLabel +"\nused: "+ m_usefullCubes +"\nall: "+ m_allCubes + "\nunused:" + m_unUsedCubes + "\n" ;
		}

		return ret;

	}

	@Override
	public boolean isLocal() {
		return true;
	}
	
	public  THashSet<BoolExpr> m_usefullCubes = new THashSet<BoolExpr>();
	public  THashSet<BoolExpr> m_unUsedCubes = new THashSet<BoolExpr>();
	public  THashSet<BoolExpr> m_allCubes = new THashSet<BoolExpr>();

}
