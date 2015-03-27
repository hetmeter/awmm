package fsb.explore.statespace;

import java.util.Set;

import fsb.explore.State;

public interface StateSpaceInterface {

	public abstract int size();

	public abstract Set<State> putInSubSpaceAndRemoveSubsumed(State next);

	@Deprecated
	public abstract Set<State> removeFromSubSpaceAllSubsumedBy(State found);
	
	/**
	 * Finds an exact match to next in the state space 
	 * @param next
	 * @return
	 */
	public abstract State myFindStateExact(State next);
	
	/**
	 * Finds an exact match to next in the state space 
	 * if not found searches for a state that subsumes it.
	 * @param next
	 * @return
	 */
	public abstract State myFindStateExactOrSubsume(State next);

	/**
	 * Finds all the states that match to next in the state space <br>
	 * or subsume it (s\in space. s >= next). <br>
	 * bit wise (*,1,1)>=(1,1,1)..
	 * @param space
	 * @param next
	 * @return
	 */
	public abstract Set<State> myFindStateThatSubsume(State next);

	/**
	 * clears the state space
	 */
	public abstract void clear();
}