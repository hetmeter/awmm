package fsb.semantics.empeagerpso;

import fsb.ast.MProgram;
import fsb.explore.State;
import fsb.explore.StateFactory;
import fsb.explore.Validator;

public class EmpEagerAbsStateFactory implements StateFactory {

	@Override
	public State newState(State current) {
		return new EmpEagerAbsState((EmpEagerAbsState)current);
	}

	@Override
	public State newState(MProgram program, Validator validator, int numProcs) {
		// TODO Auto-generated method stub
		return new EmpEagerAbsState(program, validator, numProcs);
	}

}
