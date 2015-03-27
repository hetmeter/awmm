package fsb.tvl;

import java.util.WeakHashMap;

import fsb.utils.Options;

public class DetArithValue extends ArithValue {

	private Integer m_value;
	private static WeakHashMap<Integer, DetArithValue> m_instances =  new WeakHashMap<Integer, DetArithValue>();

	@Override
	public ArithValue plus(ArithValue a) {
		if(a instanceof NondetArithValue){
			return a;
		}
		DetArithValue other = (DetArithValue)a;
		
		return getInstance(m_value + other.m_value);
	}

	@Override
	public ArithValue minus(ArithValue a) {
		if(a instanceof NondetArithValue){
			return a;
		}
		DetArithValue other = (DetArithValue)a;
		
		return getInstance(m_value - other.m_value);	}

	@Override
	public ArithValue mul(ArithValue a) {
		if(a instanceof NondetArithValue){
			return a;
		}
		DetArithValue other = (DetArithValue)a;
		
		return getInstance(m_value * other.m_value);	
	}

	@Override
	public ArithValue unaryMinus() {
		return getInstance(-m_value);	}

	@Override
	public BoolValue eq(ArithValue a) {
		if(a instanceof NondetArithValue){
			return NondetBoolValue.getInstance();
		}
		DetArithValue other = (DetArithValue)a;
		
		return DetBoolValue.getInstance(m_value == other.m_value);	
	}

	@Override
	public BoolValue smallerThen(ArithValue a) {
		if(a instanceof NondetArithValue){
			return NondetBoolValue.getInstance();
		}
		DetArithValue other = (DetArithValue)a;
		
		return DetBoolValue.getInstance(m_value < other.m_value);	
	}

	@Override
	public BoolValue greaterThen(ArithValue a) {
		if(a instanceof NondetArithValue){
			return NondetBoolValue.getInstance();
		}
		DetArithValue other = (DetArithValue)a;
		
		return DetBoolValue.getInstance(m_value > other.m_value);	
	}
	
	//to avoid duplicate..
	private DetArithValue(Integer i){
		super();
		m_value =i;
	}

	
	public static DetArithValue getInstance(Integer i){
		if(! m_instances.containsKey(i)){
			m_instances.put(i, new DetArithValue(i));
		}
		return m_instances.get(i); 
	}


	@Override
	public String toString() {
		return m_value.toString();
	}

	
	public int getValue(){
		return m_value;
	}

	@Override
	public boolean isDetermined() {
		return true;
	}

	@Override
	public ArithValue not() {
		if(Options.IS_BOOLEAN_PROGRAM){
			if(m_value==0)return getInstance(1);
			return getInstance(0);
		}
		throw new RuntimeException("not() should not be on Arith Value unless we are in a boolean program");
	}

}