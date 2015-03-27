package fsb.semantics.sc;

import fsb.ast.MProgram;
import fsb.explore.State;
import fsb.explore.StateFactory;
import fsb.explore.Validator;

public class SCStateFactory implements StateFactory {

	@Override
	public State newState(State current) {
		return new SCState((SCState)current);
	}
	
	@Override
	public State newState(MProgram program, Validator validator, int numProcs) {
		// TODO Auto-generated method stub
		return new SCState(program, validator, numProcs);
	}

}
