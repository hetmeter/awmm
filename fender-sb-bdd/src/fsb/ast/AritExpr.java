package fsb.ast;

import java.util.List;

import fsb.explore.State;
import fsb.tvl.ArithValue;

public interface AritExpr {
	public enum AXType {PLUS, MINUS, MUL};
	public ArithValue evaluate(State s, int pid);
	public List<Integer> getVariablesInvolved() ;
}
