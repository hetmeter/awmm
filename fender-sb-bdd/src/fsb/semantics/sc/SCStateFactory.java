package fsb.semantics.sc;

import fsb.ast.MProgram;
import fsb.explore.State;
import fsb.explore.StateFactory;
import fsb.explore.Validator;
import fsb.utils.Options;

public class SCStateFactory implements StateFactory {

	public SCStateFactory(MProgram program,  Validator validator, int numProcs) { 
		State.setNumProcs(numProcs);
		State.setProgram(program);
	}
	
	@Override
	public State newState(State current) {
		if(fsb.utils.Debug.DEBUG_LEVEL>0 &&  fsb.utils.Options.PRINT_FOCUS_STATISTICS){
			return new SCStateWithStatistics((SCState)current);
		}else{
			return new SCState((SCState)current);
		}
	}
	
	@Override
	public State newState() {
		// TODO Auto-generated method stub
		return new SCState();
	}
	
	public State newState(MProgram program, Validator validator, int numProcs) {
		throw new RuntimeException("Not supported");
	}

}
