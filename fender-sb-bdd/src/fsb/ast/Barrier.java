package fsb.ast;

public class Barrier extends Statement {

	public Barrier() {
		super(StatType.BARRIER);
	}

	public String toString()
	{
		return "BARRIER";
	}

	@Override
	public boolean isLocal() {
		return false;
	}
}
