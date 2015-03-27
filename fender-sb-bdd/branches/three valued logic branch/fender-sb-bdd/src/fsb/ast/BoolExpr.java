package fsb.ast;

import fsb.ast.tvl.BoolValue;
import fsb.explore.State;

public interface BoolExpr {
	public enum BXType {EQUAL, NEQ, GREATER, LESS, AND, OR, NOT};
	public BoolValue evaluate(State s, int pid);
}
