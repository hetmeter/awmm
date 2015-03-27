package fsb.ast.tvl;

import java.util.WeakHashMap;

public class DeterArithValue extends ArithValue {

	private static WeakHashMap<Integer, DeterArithValue> m_instnaces = new WeakHashMap<Integer, DeterArithValue>();

	private Integer m_value;	
	
	@Override
	public ArithValue plus(ArithValue a) {
		
		//perhaps redundant 
		if(0 == m_value)return a;
		
		if(a instanceof DeterArithValue){
			DeterArithValue other = (DeterArithValue)a;
			
			if(0 == other.m_value)return this;
			return getInstance(m_value + other.m_value);
		}
		
		return NonDeterArithValue.getInstance();
	}

	@Override
	public ArithValue sub(ArithValue a) {

		return plus(a.negate());	
	}

	@Override
	public ArithValue mul(ArithValue a) {

		//perhaps redundant 
		if(1 == m_value)return a;
		if(0 == m_value)return this;
		
		if(a instanceof DeterArithValue){
			DeterArithValue other = (DeterArithValue)a;
			
			if(1 == other.m_value)return this;
			if(0 == other.m_value)return other;
			return getInstance(m_value * other.m_value);
			
		}
		
		return NonDeterArithValue.getInstance();
	}

	@Override
	public ArithValue negate() {
		if(0 == m_value)return this;
	return getInstance(-m_value);
	}

/**
 * if both value known comparing them,
 * otherwise returning unknown; 
 */
	@Override
	public BoolValue eq(ArithValue a) {
		if(a instanceof ArithValue){
			DeterArithValue other = (DeterArithValue)a; 
			return DeterBoolValue.getInstance(m_value == other.m_value);
		}
		return NonDeterBoolValue.getInstance();
	}


	@Override
	public BoolValue neq(ArithValue a) {
		return eq(a).not();
	}


	@Override
	public BoolValue lessThen(ArithValue a) {
		if(a instanceof ArithValue){
			DeterArithValue other = (DeterArithValue)a; 
			return DeterBoolValue.getInstance(m_value < other.m_value);
		}
		return NonDeterBoolValue.getInstance();
	}


	@Override
	public BoolValue greaterThen(ArithValue a) {
		if(a instanceof ArithValue){
			DeterArithValue other = (DeterArithValue)a; 
			return DeterBoolValue.getInstance(m_value > other.m_value);
		}
		return NonDeterBoolValue.getInstance();
	}
	
	public int getValue(){
		return m_value;
	}

	@Override
	public String toString() {
		return m_value.toString();
	}

	// weird stuff
	
	public static DeterArithValue getInstance(int val){
		if(!m_instnaces.containsKey(val)){
			m_instnaces.put(val, new DeterArithValue(val));
		}
		return m_instnaces.get(val);
	}
	private DeterArithValue(int val) {
		m_value = val;
	}

}
