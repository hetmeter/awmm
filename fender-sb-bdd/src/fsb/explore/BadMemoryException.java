package fsb.explore;

//An exception used to indicate access to unallocated or uninitialized memory
public class BadMemoryException extends RuntimeException {

	private static final long serialVersionUID = -8389693898155427446L;

	public BadMemoryException(String str)
	{
		super("Unallocated memory at " + str);
	}
}
