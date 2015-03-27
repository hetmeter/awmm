package fsb.tvl;


public class NondetArithValue extends ArithValue {
	
private static NondetArithValue m_instance = new NondetArithValue();

	@Override
	public ArithValue plus(ArithValue a) {
		return this;
	}

	@Override
	public ArithValue minus(ArithValue a) {
		return this;
	}

	@Override
	public ArithValue mul(ArithValue a) {
		return this;
	}

	@Override
	public ArithValue unaryMinus() {
		return this;
	}

	@Override
	public BoolValue eq(ArithValue a) {
		return NondetBoolValue.getInstance();
	}

	@Override
	public BoolValue smallerThen(ArithValue a) {
		return NondetBoolValue.getInstance();
	}

	@Override
	public BoolValue greaterThen(ArithValue a) {
		return NondetBoolValue.getInstance();
	}
	
	//singleton class
	private NondetArithValue(){
		super();
	}
	
	public static NondetArithValue getInstance(){
		return m_instance;
	}
	@Override
	public String toString(){
		return "*";
	}


/*	@Override
	public BoolValue toBoolValue() {
		// TODO Auto-generated method stub
		return NondetBoolValue.getInstance();
	}*/


/*	@Override
	public BoolValue toBoolValue() {
		// TODO Auto-generated method stub
		return NondetBoolValue.getInstance();
	}*/

	@Override
	public boolean isDetermined() {
		return false;
	}

	@Override
	public ArithValue not() {
		return this;
	}
}
