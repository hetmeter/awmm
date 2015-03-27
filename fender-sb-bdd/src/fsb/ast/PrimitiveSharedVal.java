package fsb.ast;
import fsb.explore.SBState;

public class PrimitiveSharedVal implements SharedVal {

	int name;
	
	public PrimitiveSharedVal(String name)
	{
		this.name = SymbolTable.getOrCreateGlobalVariable(name);
	}
	
	@Override
	public int evalShared(SBState s, int pid) {
		return name;
	}
	
	@Override
	public String toString()
	{
		return SymbolTable.getGlobalVariableName(name);
	}
}
