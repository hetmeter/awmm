package fsb.semantics.naivepso;

import fsb.ast.MProgram;
import fsb.explore.State;
import fsb.explore.StateFactory;
import fsb.explore.Validator;

public class NaiveAbsStateFactory implements StateFactory {

	@Override
	public State newState(State current) {
		return new NaiveAbsState((NaiveAbsState)current);
	}

	@Override
	public State newState(MProgram program, Validator validator, int numProcs) {
		// TODO Auto-generated method stub
		return new NaiveAbsState(program, validator, numProcs);
	}

}
