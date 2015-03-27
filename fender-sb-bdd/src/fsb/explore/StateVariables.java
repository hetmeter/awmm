package fsb.explore;




import ags.constraints.BitSet;

import fsb.ast.SymbolTable;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;
import fsb.tvl.NondetArithValue;

public class StateVariables {

	BitSet m_true_values;
	BitSet m_false_values;
	static int m_num_local_variables;
	static int m_num_global_variables;
	boolean m_global;



	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result
				+ ((m_false_values == null) ? 0 : m_false_values.hashCode());
		result = prime * result + (m_global ? 1231 : 1237);
		//result = prime * result + m_num_variables;
		result = prime * result
				+ ((m_true_values == null) ? 0 : m_true_values.hashCode());
		return result;
	}

	public boolean subsetOf(StateVariables other) {                                                                                                                           
	    for(int variable=0;variable<size();++variable){
	    		ArithValue otherVar = other.get(variable);
	    		ArithValue thisVar = this.get(variable);
	    		if(!otherVar.isDetermined()){ continue;}
	    		if(! (thisVar == otherVar) ){
	    			return false;
	    		}
	    }
	    return true;                                                                                                                                     
		                                                                                                                             
	}                                                                                                                                
	
	private boolean subsetOrSubsumes(StateVariables other) {                                                                                                                           
	    for(int variable=0;variable<size();++variable){
	    		ArithValue otherVar = other.get(variable);
	    		ArithValue thisVar = this.get(variable);
	    		if((!otherVar.isDetermined()) || (!thisVar.isDetermined())){ continue;}
	    		if(! (thisVar == otherVar) ){
	    			return false;
	    		}
	    }
	    return true;                                                                                                                                     
		                                                                                                                             
	}
	/**
	 * does a join - non optimized<br>
	 * 1 U * = *<br>
	 * 1 U 0 = * <br>
	 * @param other
	 * @return the new value if joined differs from current and null otherwise.
	 */
	public StateVariables join(StateVariables other) {
		boolean ret = false;
		StateVariables newSTV = new StateVariables(this);
		
	    for(int variable=0;variable<size();++variable){
	    		ArithValue otherVar = other.get(variable);
	    		ArithValue thisVar = this.get(variable);
	    		if(!thisVar.isDetermined()){
	    			continue;
	    		}
	    		if((!otherVar.isDetermined()) || (thisVar != otherVar)){ 
	    			newSTV.put(variable, NondetArithValue.getInstance());
	    			ret = true;
	    		}
	    }
	    if(ret){
	    		return newSTV;
	    }
	    else{
	    		return null;
	    }
	    	
		                                                                                                                             
	}
	/* (non-Javadoc)
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {

		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof StateVariables)) {
			return false;
		}
		StateVariables other = (StateVariables) obj;
		if (m_false_values == null) {
			if (other.m_false_values != null) {
				return false;
			}
		} else if (!m_false_values.equals(other.m_false_values)) {
			return false;
		}
		if (m_global != other.m_global) {
			return false;
		}
//		if (m_num_variables != other.m_num_variables) {
//			return false;
//		}
		if (m_true_values == null) {
			if (other.m_true_values != null) {
				return false;
			}
		} else if (!m_true_values.equals(other.m_true_values)) {
			return false;
		}
		return true;
	}
	/**
	 * this >= obj? 
	 * @param obj
	 * @return
	 */
	public boolean equals_subsumes(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof StateVariables)) {
			return false;
		}
		StateVariables other = (StateVariables) obj;
			
		return other.subsetOf(this);
	}
	/**
	 * this <= obj?
	 * @param obj
	 * @return
	 */
	public boolean equals_subsumed(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof StateVariables)) {
			return false;
		}
		StateVariables other = (StateVariables) obj;
			
		return this.subsetOf(other);
	}
	
	public StateVariables(int numVars, boolean is_global) {
		m_true_values = new BitSet(numVars);
		m_false_values = new BitSet(numVars);
		if(is_global){
			m_num_global_variables = numVars;
		}else{
			m_num_local_variables = numVars;
		}
		m_global = is_global;
	}

	public StateVariables(StateVariables other) {
		m_global = other.m_global;
		m_num_local_variables = other.m_num_local_variables;
		m_num_global_variables = other.m_num_global_variables;
		m_true_values = (BitSet)other.m_true_values.clone();
		m_false_values = (BitSet)other.m_false_values.clone();
	}

	@Override
	public StateVariables clone() {
		StateVariables other = new StateVariables(this);
		return other;
	}
	
	private  ArithValue getValue(int variable)
	{   
	    if(m_true_values.get(variable)){
	    	return DetArithValue.getInstance(1);
	    }
	    if(m_false_values.get(variable)){
	    	return DetArithValue.getInstance(0);
	    }
	    
	    return NondetArithValue.getInstance();
	    
	    
	}
	
	public void putAll(StateVariables other) {
		m_true_values = (BitSet)other.m_true_values.clone();
		m_false_values= (BitSet)other.m_false_values.clone();
	}

	public void put(int var, ArithValue initval) {

		if(initval.isDetermined()){
			DetArithValue val = (DetArithValue)initval;
			if(val.getValue() == 1){
				m_true_values.set(var,true);
				m_false_values.set(var, false);
				return;
			}
			m_true_values.set(var,false);
			m_false_values.set(var, true);
			return;
		}
		m_true_values.set(var,false);
		m_false_values.set(var, false);
		return;

	}

	public ArithValue get(int var) {
		return getValue(var);

	}

	
	
			/*int[] m_trueFalse;
		int[] m_determined;
		int m_num_variables;
		boolean m_global;
		final int m_magic_num_bits_in_int =32;

		private  Boolean isBitSet(int b, int bit)
		{
		    return (b & (1 << bit)) != 0;
		}
		
		private  int setBit(int b, int bit)
		{

				return  (b | (1 << bit));		    

		}
		private  int unSetBit(int b, int bit)
		{

				return b & ~(1 << bit);		    
		}		

		private  ArithValue getValue(int variable)
		{
		    int cell = variable / m_magic_num_bits_in_int;
		    int bit = variable % m_magic_num_bits_in_int;
		    
		    if(!isBitSet(m_determined[cell], bit)){
		    	return NondetArithValue.getInstance();
		    }
		    if(isBitSet(m_trueFalse[cell], bit)){
		    	return DetArithValue.getInstance(1);
		    } 
		    return DetArithValue.getInstance(0);
		    
		    
		}
*/
		public String toString() {
			String res = "[";
			boolean first = true;
			for (int i = 0; i < size(); ++i) {
				if (!first) {
					res += ", ";
				} else {
					first = false;
				}
				if(m_global){
				res += SymbolTable.getGlobalVariableName(i) + "=" + getValue(i);
				}else{
					res += SymbolTable.getLocalVariableName(i) + "=" + getValue(i);
				}
			}
			return res + "]";
		}
/*
		protected StateVariables(int sizeLocals, boolean is_global) {
			m_trueFalse = new int[sizeLocals / m_magic_num_bits_in_int + 1];
			m_determined = new int[sizeLocals / m_magic_num_bits_in_int +1 ];
			m_num_variables = sizeLocals;
			m_global = is_global;
		}
*/
		public int size() {
			if(m_global){
				return m_num_global_variables;
			}
			return m_num_local_variables;
		}
/*
		public void putAll(StateVariables other) {
			m_trueFalse = Arrays.copyOfRange(other.m_trueFalse, 0, m_trueFalse.length);
			m_determined = Arrays.copyOfRange(other.m_determined, 0, m_determined.length);

		}

		public void put(int var, ArithValue initval) {
		    int cell = var / m_magic_num_bits_in_int;
		    int bit = var % m_magic_num_bits_in_int;

			if(!initval.isDetermined()){
				m_determined[cell] = unSetBit(m_determined[cell], bit);
				return;
			}

			m_determined[cell] = setBit(m_determined[cell], bit);
			
			DetArithValue val = (DetArithValue)initval;
			if(val.getValue() == 1){
				m_trueFalse[cell] = setBit(m_trueFalse[cell], var);
				return;
			}
			m_trueFalse[cell] = unSetBit(m_trueFalse[cell], var);
			return;

		}

		public ArithValue get(int var) {
			return getValue(var);

		}

		@Override
		public int hashCode() {
			int random_prime = 17;
			int ret = Arrays.hashCode(m_determined); 
			return ret*random_prime + Arrays.hashCode(m_trueFalse);  
		}
		@Override
		public boolean equals(Object obj) {
			if (!(obj instanceof StateVariables))
				return false;
			StateVariables other = (StateVariables) obj;

			if (!Arrays.equals(other.m_determined, m_determined) || !Arrays.equals(other.m_trueFalse, m_trueFalse)) {
				return false;
			}
			return true;
		}
	*/

		public String getKeyString(){
			String ret = "";
			for(int i=0;i<this.size();++i){
				ret += this.get(i).toString();
			}
			return ret;
		}

		public void reset() {
			m_true_values.clear();
			m_false_values.set(0, size());
			
		}
}
