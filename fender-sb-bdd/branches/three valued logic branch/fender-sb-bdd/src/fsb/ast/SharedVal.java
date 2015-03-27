package fsb.ast;
import fsb.explore.SBState;

public interface SharedVal {
	public String evalShared(SBState s, int pid);
}
