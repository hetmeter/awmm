package fsb.ast;

import fsb.ast.tvl.ArithValue;
import fsb.explore.State;

public interface AritExpr {
	public enum AXType {PLUS, MINUS, MUL};
	public ArithValue evaluate(State s, int pid);
}
