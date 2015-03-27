package fsb.ast.tvl;

public class NonDeterArithValue extends ArithValue {

	
	/**
	 * addition of unknown to any value is unknown 
	 */
	@Override
	public ArithValue plus(ArithValue a) {
		return this;
	}

	/**
	 * subtraction of unknown from any value is unknown 
	 */
	@Override
	public ArithValue sub(ArithValue a) {
		return this;
	}
	/**
	 * if a is known then expecting it to handle the multiplication.
	 * otherwise unknown;
	 */
	@Override
	public ArithValue mul(ArithValue a) {
		if(a instanceof DeterArithValue){
			return a.mul(this);
		}
		return this;
	}

	@Override
	public ArithValue negate() {
		return this;
	}

	/**
	 * a singleton class
	 */
	private NonDeterArithValue(){
		super();
	}

	/**
	 * the singleton class instance
	 */
	private static NonDeterArithValue m_instance = new NonDeterArithValue();
	
	/**
	 * returning the singleton class instance
	 */
	public static NonDeterArithValue getInstance(){
		return m_instance;
	}

	@Override
	public BoolValue eq(ArithValue a) {
		return NonDeterBoolValue.getInstance();
	}

	@Override
	public BoolValue neq(ArithValue a) {
		// TODO Auto-generated method stub
		return NonDeterBoolValue.getInstance();
	}

	@Override
	public BoolValue lessThen(ArithValue a) {
		// TODO Auto-generated method stub
		return NonDeterBoolValue.getInstance();
	}

	@Override
	public BoolValue greaterThen(ArithValue a) {
		// TODO Auto-generated method stub
		return NonDeterBoolValue.getInstance();
	}
@Override
public String toString() {
	return "*";
}
}
