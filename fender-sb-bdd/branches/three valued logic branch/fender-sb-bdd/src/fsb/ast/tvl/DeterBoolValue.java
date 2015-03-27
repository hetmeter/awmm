package fsb.ast.tvl;

public class DeterBoolValue extends BoolValue {

	Boolean m_value;
	
	/**
	 * short-circuit conjunction implementation
	 * if true returning the other
	 * otherwise returning false
	 * @param b
	 * @return
	 */
	@Override
	public  BoolValue and(BoolValue b) {
		
		if(m_value) return b;
		
		return this;
	}

	/**
	 * short-circuit disjunction implementation
	 * if false returning the other
	 * otherwise returning true
	 * @param b
	 * @return
	 */
	@Override
	public BoolValue or(BoolValue b) {
		
		if(!m_value) return b;
		
		return this;		
	}

	
	@Override
	public BoolValue not() {
		
		
		return getInstance(!m_value);		
	}
	
	@Override
	public String toString() {
		return m_value.toString();
	}
	
	
	private static DeterBoolValue m_trueInstance = new DeterBoolValue(true);
	private static DeterBoolValue m_falseInstance = new DeterBoolValue(false);
	
	public static DeterBoolValue getInstance(boolean b) {
		if(b) return m_trueInstance;
		else return m_falseInstance;
	}
	
	private DeterBoolValue(boolean val) {
		m_value = val;
	}
	public boolean isTrue(){
		return m_value;
	}
}
