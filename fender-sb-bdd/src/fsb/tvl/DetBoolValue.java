package fsb.tvl;

import java.util.ArrayList;
import java.util.Collection;

public class DetBoolValue extends BoolValue {

	boolean m_value;
	public static DetBoolValue m_true_instance = new DetBoolValue(true);
	public static DetBoolValue m_false_instance = new DetBoolValue(false);
	
	@Override
	public BoolValue and(BoolValue b) {
		if(false == m_value){
			return this;
		}
		return b;
	}

	@Override
	public BoolValue or(BoolValue b) {
		if(true == m_value){
			return this;
		}
		return b;	}

	@Override
	public BoolValue not() {
		if(false == m_value){
			return m_true_instance;
		}
		return m_false_instance;	}

	@Override
	public boolean isTrue() {
		return m_value;
	}

	@Override
	public boolean isDetermined() {
		return true;
	}

	@Override
	public boolean isFalse() {
		return !m_value;
	}

	
	//sort of a singleton.
	/**
	 * This class should have only two instances.
	 * please use {@link #getFalseInstace()} or {@link #getTrueInstace()}
	 * @param val
	 */
	private DetBoolValue(boolean val) {
		super();
		m_value = val;
	}
	
	public static DetBoolValue getTrueInstace(){
		return m_true_instance;
	}
	
	public static DetBoolValue getFalseInstace(){
		return m_false_instance;
	}

	@Override
	public Collection<BoolValue> getConcreteValues() {
		ArrayList<BoolValue> ret = new ArrayList<BoolValue>();
		ret.add(this);
		return ret;
	}

	public static BoolValue getInstance(boolean b) {
		if(b) return getTrueInstace();
		return getFalseInstace();
	}

	@Override
	public ArithValue getArithEquvalent() {
		if(m_value) return DetArithValue.getInstance(1);
		return DetArithValue.getInstance(0);
	}
	
	
	
	@Override
	public String toString(){
		return (new Boolean (m_value)).toString();
	}
}
