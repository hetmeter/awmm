package fsb.ast;

import java.util.List;
import java.util.Collections;

import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

public class ConstExpr implements AritExpr {
	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((val == null) ? 0 : val.hashCode());
		return result;
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		ConstExpr other = (ConstExpr) obj;
		if (val == null) {
			if (other.val != null)
				return false;
		} else if (!val.equals(other.val))
			return false;
		return true;
	}

	ArithValue val;
	
	public ConstExpr(int val)
	{
		this.val = DetArithValue.getInstance(val);	
	}

	@Override
	public ArithValue evaluate(State s, int pid) {
		return val;
	}
	
	public String toString()
	{
		return "" + val;
	}
	
	@Override
	public List<Integer> getVariablesInvolved() {
		return Collections.EMPTY_LIST;
	}
}
