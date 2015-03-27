package fsb.ast.tvl;

public abstract class BoolValue {

	public abstract BoolValue and(BoolValue b);
	public abstract BoolValue or(BoolValue b);
	public abstract BoolValue not();
	
	public abstract boolean isTrue(); 
	//abstract BoolValue xor(BoolValue b);
	

}