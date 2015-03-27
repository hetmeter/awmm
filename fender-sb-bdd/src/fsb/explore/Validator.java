package fsb.explore;

/**
 * A construct to indicate if a state is an error state or not.
 * @author user
 *
 */
public abstract class Validator {
	abstract public boolean isErr(State s);
}
