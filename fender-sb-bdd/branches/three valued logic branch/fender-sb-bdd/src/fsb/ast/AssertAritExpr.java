package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public interface AssertAritExpr {
	public ArithValue evaluate(State s);
}
