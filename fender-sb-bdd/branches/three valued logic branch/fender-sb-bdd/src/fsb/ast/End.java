package fsb.ast;

public class End extends Statement {

	public End() {
		super(StatType.END);
	}
	
	public boolean isLast() {
		return true;
	}
	
	public String toString()
	{
		return "END";
	}

	@Override
	public boolean isLocal() {
		return false;
	}
}
