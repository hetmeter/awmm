package fsb.ast;

import fsb.utils.Options;
import gnu.trove.set.hash.THashSet;

public class ChooseStore extends Statement {

	public BoolExpr cond1;
	public BoolExpr cond2;
	public SharedVal dst;
	
	public ChooseStore(BoolExpr cond1, BoolExpr cond2, SharedVal dst) {
		super(StatType.CHOOSE_STORE);
		this.cond1 = cond1;
		this.cond2 = cond2;
		this.dst = dst;
	}

	public String toString() {
		String ret = "STORE " + dst + " = CHOOSE(" + cond1 + ", " + cond2
				+ "); ";

		if (Options.PRINT_USED_CUBES_PER_STATEMENT) {
			m_unUsedCubes.clear();
			m_unUsedCubes.addAll(m_allCubes);
			m_unUsedCubes.removeAll(m_usefullCubes);
			ret = "\nSTORE " + dst + " = CHOOSE(" + cond1 + ", " + cond2
					+ "); " + "\nused: " + m_usefullCubes + "\nall: "
					+ m_allCubes + "\nunused:" + m_unUsedCubes + "\n";
		}

		return ret;

	}

	@Override
	public boolean isLocal() {
		return true;
	}

	public  THashSet<BoolExpr> m_usefullCubes = new THashSet<BoolExpr>();
	public  THashSet<BoolExpr> m_allCubes = new THashSet<BoolExpr>();
	public  THashSet<BoolExpr> m_unUsedCubes = new THashSet<BoolExpr>();
}
