package fsb.ast;
import fsb.explore.SBState;

public class PrimitiveSharedVal implements SharedVal {

	String name;
	
	public PrimitiveSharedVal(String name)
	{
		this.name = name;
	}
	
	@Override
	public String evalShared(SBState s, int pid) {
		return name;
	}
	
	@Override
	public String toString()
	{
		return name;
	}
}
