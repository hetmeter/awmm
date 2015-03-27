package fsb.ast;

public class Get extends Statement {
	
	SharedVal src;
	SharedVal dst;

	public Get(SharedVal dst, SharedVal src) {
		super(Statement.StatType.GET);
		this.src = src;
		this.dst = dst;
	}
	
	public String toString()
	{
		return "GET " + dst + " = " + src;
	}
	
	@Override
	public boolean isLocal() {
		// TODO Auto-generated method stub
		return false;
	}

}
