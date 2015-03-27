package fsb.semantics.eagerpso;

import fsb.ast.MProgram;
import fsb.explore.State;
import fsb.explore.StateFactory;
import fsb.explore.Validator;

public class EagerAbsStateFactory implements StateFactory {

	@Override
	public State newState(State current) {
		return new EagerAbsState((EagerAbsState)current);
	}

	@Override
	public State newState(MProgram program, Validator validator, int numProcs) {
		// TODO Auto-generated method stub
		return new EagerAbsState(program, validator, numProcs);
	}

	@Override
	public State newState() {
		throw new RuntimeException("NYI");
	}

}
