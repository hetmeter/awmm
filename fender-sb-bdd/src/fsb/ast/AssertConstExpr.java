package fsb.ast;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import fsb.explore.State;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

public class AssertConstExpr implements AssertAritExpr {
	ArithValue val;

	public AssertConstExpr(int val) {
		this.val = DetArithValue.getInstance(val);
	}

	@Override
	public ArithValue evaluate(State s) {
		return val;
	}

	public String toString() {
		return "" + val;
	}

	@Override
	public Set<Integer> getVariablesInvolved() {
		return Collections.EMPTY_SET;
	}

	@Override
	public boolean equals(Object obj) {
		if(! (obj instanceof AssertConstExpr) ){
			return false;
		}
		AssertConstExpr other =(AssertConstExpr) obj;
		
		return other.val == val;
	}
	
	@Override
	public int hashCode() {
		return val.hashCode();
	}

	@Override
	public AssertAritExpr assignConcreteValuesFromState(State s) {
		// TODO Auto-generated method stub
		return this;
	}
}
