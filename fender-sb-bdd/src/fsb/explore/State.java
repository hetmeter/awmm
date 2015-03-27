package fsb.explore;

import java.util.List;
import java.util.Set;

import ags.constraints.Formula;
import fsb.ast.MProgram;
import fsb.ast.Statement;
import fsb.tvl.ArithValue;

public abstract class State {
	protected static int numProcs;
	public static MProgram program;

	protected boolean bad;
	protected int inAtomicSection;
	protected boolean assumeFailed;

	public static void setProgram(MProgram p) {
		program = p;
	}

	public static void setNumProcs(int np) {
		numProcs = np;
	}

	public State() {
		this.bad = false;
		this.inAtomicSection = -1;
		this.assumeFailed = false;
	}

	public State(State s) {
		this.bad = s.bad;
		this.inAtomicSection = s.inAtomicSection;
		this.assumeFailed = s.assumeFailed;
	}

	public State copy(){
		return null;
	}
	@Override
	public boolean equals(Object obj) {
		// Don't consider "bad".
		if (this == obj)
			return true;
		if (!(obj instanceof State))
			return false;

		return true;
	}

	@Override
	public int hashCode() {
		return numProcs;
	}

	public int hashCode_subs() {
		return numProcs;
	}
	public boolean isErr() {
		// A state for which an assume() statement failed
		// is by definition never an error state.
		if (assumeFailed())
			return false;

		return program.getValidator().isErr(this) || bad;
	}

	public int getProcs() {
		return numProcs;
	}

	public int getLabelIndex(int proc, int label) {
		// TODO: This should be a lookup...
		List<Statement> listing = program.getListing(proc);
		int i = 0;
		for (Statement st : listing) {
			if (st.getLabel() == label)
				return i;
			i++;
		}
		throw new RuntimeException("Label " + label + " not found!");
	}

	public void setBad() {
		bad = true;
	}

	// This is a horrible, horrible hack
	public boolean isJoinable(State other) {
		return false;
	}

	public boolean join(State other) {
		return false;
	}

	public abstract boolean isFinal();

	public abstract boolean isInitFinished();

	public abstract ArithValue getLocal(int pid, int res);

	public abstract void setLocal(int pid, int res, ArithValue val);

	public abstract Set<Formula> getPredicates(Action action);
	
	public abstract int getPC(int pid);

	// A priori, everything is avoidable. :)
	// Should be overridden by states that really need to support inference.
	public boolean isAvoidable(Action taken) {
		return true;
	}

	// Same
	public boolean disjPredicates() {
		return true;
	}

	public Formula getAvoidFormula(Action action) {
		throw new RuntimeException("getAvoidFormula() not implemented!");
	}

	// Returns -1 if no process is in an atomic section
	// Or the process number that is currently running atomically otherwise.
	public int inAtomicSection() {
		return inAtomicSection;
	}

	public void setAtomic(int proc) {
		inAtomicSection = proc;
	}

	public void unsetAtomic() {
		inAtomicSection = -1;
	}

	public boolean assumeFailed() {
		return assumeFailed;
	}

	public void setAssumeFailed() {
		assumeFailed = true;
	}

	public void registerAction(Action in) {
	}
	
	 /**
	  * this >= obj
	  */
	public boolean equals_subsumes(Object obj) {
		return equals(obj);
	}
	
	 /**
	  * this <= obj
	  */
	public boolean equals_subsumed(Object obj) {
		return equals(obj);
	}
	public String getKeyString(){
		return "";
	}
	
	public StateVariables[] getKeyBitSet(){
		return null;
	}
}
