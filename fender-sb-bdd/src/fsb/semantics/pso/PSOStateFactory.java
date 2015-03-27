package fsb.semantics.pso;

import javax.management.RuntimeErrorException;

import fsb.ast.MProgram;
import fsb.explore.State;
import fsb.explore.StateFactory;
import fsb.explore.Validator;

public class PSOStateFactory implements StateFactory {

	@Override
	public State newState(State current) {
		return new PSOState((PSOState)current);
	}

	@Override
	public State newState(MProgram program, Validator validator, int numProcs) {
		// TODO Auto-generated method stub
		return new PSOState(program, validator, numProcs);
	}

	@Override
	public State newState() {
		throw new RuntimeException("NYI");
	}

}
