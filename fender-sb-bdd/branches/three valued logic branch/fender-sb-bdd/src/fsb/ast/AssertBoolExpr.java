package fsb.ast;

import fsb.explore.State;

public interface AssertBoolExpr {
	public boolean evaluate(State s);
}
