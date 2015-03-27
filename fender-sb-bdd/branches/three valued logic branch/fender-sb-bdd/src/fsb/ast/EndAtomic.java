package fsb.ast;

public class EndAtomic extends Statement {

	public EndAtomic() {
		super(StatType.ENDATOMIC);
	}

	public String toString()
	{
		return "END_ATOMIC";
	}

	@Override
	public boolean isLocal() {
		return false;
	}
}
