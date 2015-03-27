package fsb.explore.statespace;

import fsb.explore.State;

public interface StatesToExploreInterface {

	public abstract boolean isEmpty();

	public abstract State pop();

	public abstract int numStateOnStackForProcess(int i);

	public abstract int size();

	public abstract void push(State state);

}