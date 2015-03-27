package fsb.ast.tvl;

public class NonDeterBoolValue extends BoolValue {

	/**
	 *  "*" and "*" is "*"<br>
	 *  otherwise using symmetry of conjunction and delegating
	 */
	@Override
	public  BoolValue and(BoolValue b) {
		if(b instanceof NonDeterBoolValue){
			return this;
		}
		return b.and(this);
	}

	/**
	 *  "*" or "*" is "*"<br>
	 *  otherwise using symmetry of disjunction and delegating
	 */
	@Override
	public  BoolValue or(BoolValue b) {
		if(b instanceof NonDeterBoolValue){
			return this;
		}
		return b.or(this);
		
	}
	
	@Override
	public BoolValue not() {
		// TODO Auto-generated method stub
		return this;
	}
	@Override
	public String toString() {
		return "*";
	}
	
	private static NonDeterBoolValue m_instance = new NonDeterBoolValue();
	
	private NonDeterBoolValue() {
		super();
	}
	
	public static NonDeterBoolValue getInstance(){
		return m_instance;
	}

	public boolean isTrue(){
		return false;
	}

}
