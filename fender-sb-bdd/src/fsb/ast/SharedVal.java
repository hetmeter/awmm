package fsb.ast;
import fsb.explore.SBState;

public interface SharedVal {
	public int evalShared(SBState s, int pid);
}
