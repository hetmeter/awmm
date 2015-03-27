package fsb.explore;

import fsb.ast.MProgram;

public interface StateFactory {
	public State newState();
	public State newState(State current);
	public State newState(MProgram program, Validator validator, int numProcs);	
}
