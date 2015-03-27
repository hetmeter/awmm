package fsb.tvl;

import java.util.ArrayList;
import java.util.Collection;

public class NondetBoolValue extends BoolValue {

	//singleton class
	private static NondetBoolValue m_instance = new NondetBoolValue();

	@Override
	public BoolValue and(BoolValue b) {
		if(b instanceof NondetBoolValue)
			return this;
		return b.and(this);
	}

	@Override
	public BoolValue or(BoolValue b) {
		if(b instanceof NondetBoolValue)
			return this;
		return b.or(this);
	}

	@Override
	public BoolValue not() {
		return this;
	}

	@Override
	public boolean isTrue() {
		return false;
	}

	@Override
	public boolean isDetermined() {
		return false;
	}

	@Override
	public boolean isFalse() {
		return false;
	}
	/**
	 * Singleton class, please use 
	 * {@link #getInstance()}
	 */
	private NondetBoolValue() {
		super();
	}
		
	/**
	 * @return the single existent instance of
	 * {@link NondetBoolValue} 
	 */
	public static NondetBoolValue getInstance() {
		return m_instance;
	}
	public String toString(){
		return "*b";
	}

//	@Override
//	public Collection<BoolValue> getConcreteValues() {
//		ArrayList<BoolValue> ret = new ArrayList<BoolValue>();
//		ret.add(DetBoolValue.getFalseInstace());
//		ret.add(DetBoolValue.getTrueInstace());
//		return ret;
//	}
	static public String getMemResource(ArithValue arithValue)
	{
		return "MEM" + arithValue;
	}

//	@Override
//	public ArithValue getArithEquvalent() {
//		// TODO Auto-generated method stub
//		return NondetArithValue.getInstance();
//	}


	@Override
	public Collection<BoolValue> getConcreteValues() {
		ArrayList<BoolValue> ret = new ArrayList<BoolValue>();
		ret.add(DetBoolValue.getFalseInstace());
		ret.add(DetBoolValue.getTrueInstace());
		return ret;
	}

	@Override
	public ArithValue getArithEquvalent() {
		// TODO Auto-generated method stub
		return NondetArithValue.getInstance();
	}
	
}
