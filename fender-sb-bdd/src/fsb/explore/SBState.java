package fsb.explore;

import java.util.Arrays;
import java.util.List;
import java.util.WeakHashMap;



import fsb.ast.Assume;
import fsb.ast.SymbolTable;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;
import fsb.utils.Options;
import gnu.trove.map.hash.THashMap;

public abstract class SBState extends State {

//protected class LocalResMap extends TreeMap<String, ArithValue> {}
//	@SuppressWarnings("serial")
//	protected class LocalResMap extends StateVariables{
//
//		protected LocalResMap(int sizeLocals) {
//			super(sizeLocals,false);
//			// TODO Auto-generated constructor stub
//		}
//
//		public LocalResMap(StateVariables other) {
//			// TODO Auto-generated constructor stub
//			super(other);
//		}
//		
//	}

	//private static THashMap<LocalResMap, LocalResMap> cachedLocals = new THashMap<LocalResMap, LocalResMap>();
	private static WeakHashMap<StateVariables, StateVariables> cachedLocals = new WeakHashMap<StateVariables, StateVariables>();
	
	protected final StateVariables locals[];
	protected int pc[];
	public SBState() {
		super();

		this.locals = new StateVariables[numProcs];
		this.pc = new int[numProcs];
		List<Integer> program_locals = program.getLocals();

		for (int i = 0; i < numProcs; i++) {
			this.locals[i] = new StateVariables(program_locals.size(),false);
			for (Integer var : program_locals) {
				this.locals[i].put(var, DetArithValue.getInstance(0));
			}
			this.pc[i] = 0;
		}
		//this.lastMem = 1;
	}

	public SBState(SBState s) {
		super(s);
		this.locals = new StateVariables[numProcs];
		this.pc = new int[numProcs];
		//this.lastMem = s.lastMem;

		for (int i = 0; i < numProcs; i++) {
			this.locals[i] = (StateVariables) s.locals[i];
			this.pc[i] = s.pc[i];
		}

		if (!Options.LAZY_COPY) {
			for (int i = 0; i < numProcs; i++) {
				this.locals[i] = new StateVariables(locals[i].size(),false);
				this.locals[i].putAll(s.locals[i]);
			}
		}
	}

	public void advancePC(int pid) {
		pc[pid]++;
	}

	public void setPC(int pid, int toLabel) {
		pc[pid] = getLabelIndex(pid, toLabel);
	}

	@Override
	public ArithValue getLocal(int pid, int res) {
		ArithValue val = locals[pid].get(res);
		if (val == null)
			throw new RuntimeException("Local " + res + " not found");

		return val;
	}

	public void setLocal(int pid, int res, ArithValue val) {
		if (!program.getLocals().contains(res))
			throw new RuntimeException("Local " + res + " not found");

		if (Options.LAZY_COPY){
			StateVariables tmp = new StateVariables(locals[pid].size(),false); 
			tmp.putAll(locals[pid]);
			locals[pid] = tmp;
		}

		locals[pid].put(res, val);
		StateVariables cached = cachedLocals.get(locals[pid]);
		if (cached == null)
		{
			cachedLocals.put(locals[pid], locals[pid]);
		} else {
			locals[pid] = cached;
		}

	}

	public int getPC(int pid) {
		return pc[pid];
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		//result = prime * result + lastMem;
		result = prime * result + Arrays.hashCode(locals);
		result = prime * result + Arrays.hashCode(pc);

		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (!super.equals(obj))
			return false;

		if (!(obj instanceof SBState))
			return false;
		SBState other = (SBState) obj;

//		if (lastMem != other.lastMem)
//			return false;
		if (!Arrays.equals(locals, other.locals))
			return false;
		if (!Arrays.equals(pc, other.pc))
			return false;

		return true;
	}
	 /**
	  * this >= obj
	  */
	@Override
	public boolean equals_subsumes(Object obj) {
		if (!super.equals(obj))
			return false;

		if (!(obj instanceof SBState))
			return false;
		SBState other = (SBState) obj;

//		if (lastMem != other.lastMem)
//			return false;
		if (!Arrays.equals(pc, other.pc))
			return false;
		if (!myLocalsEqOrSubsumeLocalsOf(other))
			return false;


		return true;
	}
	/**
	 * this <= obj
	 */
	@Override
	public boolean equals_subsumed(Object obj) {
		if (!super.equals(obj))
			return false;

		if (!(obj instanceof SBState))
			return false;
		SBState other = (SBState) obj;

//		if (lastMem != other.lastMem)
//			return false;
		if (!Arrays.equals(pc, other.pc))
			return false;
		if (!myLocalsEqOrSubsumedByLocalsOf(other))
			return false;
		

		return true;
	}
	/**
	 * this >= other
	 * @param other
	 * @return
	 */
	private boolean myLocalsEqOrSubsumeLocalsOf(SBState other) {
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL != -1){
			for(int i=0; i< locals.length; ++i){
				if(!locals[i].equals_subsumes(other.locals[i])){
					return false;
				}
			}
			return true;
		}else{
			return Arrays.equals(locals, other.locals);
		}
	}

	/**
	 * @param other
	 * this <= other
	 * @return
	 */
	private boolean myLocalsEqOrSubsumedByLocalsOf(SBState other) {
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL != -1){
			for(int i=0; i< locals.length; ++i){
				if(!locals[i].equals_subsumed(other.locals[i])){
					return false;
				}
			}
			return true;
		}else{
			return Arrays.equals(locals, other.locals);
		}
	}


	public int allocate(int num) {
//		int start = lastMem;
//		for (; lastMem < start + num; lastMem++) {
//			addShared(getMemResource(lastMem), DetArithValue.getInstance(0));
//		}
//		return start;
		return 0;
	}

	public boolean isInitFinished() {
		// return program.getInitListing().get(initpc).isLast();
		return program.getListing(0).get(pc[0]).isLast();
	}

	static public int getMemResource(ArithValue num) {
		return ((DetArithValue)num).getValue();
	}

	static public int getMemResource(int num) {
		return num;
	}

	public abstract void addShared(int str, ArithValue initval);

	public abstract ArithValue getShared(int res);

	public abstract List<SBState> focus(Assume assume, int pid);
	
	/**
	 * performing a join of the local variables
	 * if {@link Options#USE_STATE_SUBSUMPTIUON_LEVEL} is true 
	 */
	@Override
	public boolean join(State other_s) {
			// TODO Auto-generated method stub
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL == -1){
			return super.join(other_s);
		}
		if(! (other_s instanceof SBState)){
			throw new RuntimeException("joining an SBState with something that is not an SBState how did that happen?");
		}
		SBState other = (SBState)other_s; 
		
		Boolean ret = false;
		for(int i=0; i<locals.length; ++i){
			StateVariables joined = locals[i].join(other.locals[i]);
			if(joined != null){
				ret = true;
				locals[i] = new StateVariables(joined);
			}
			
		}
		
		return ret;
	}
	@Override
	public int hashCode_subs() {
		final int prime = 31;
		int result = super.hashCode_subs();
		//result = prime * result + lastMem;
		result = prime * result + Arrays.hashCode(pc);

		return result;
	}
	
	@Override
	public String getKeyString(){
		String ret = super.getKeyString();
		for(int i=0; i<locals.length;++i){
			ret += locals[i].getKeyString() + " ";
		}
		return ret;
	}
}
