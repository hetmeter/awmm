package fsb.ast;

public class BeginAtomic extends Statement {

	public BeginAtomic() {
		super(StatType.BEGINATOMIC);
	}

	public String toString()
	{
		return "BEGIN_ATOMIC";
	}

	@Override
	public boolean isLocal() {
		return false;
	}
}
