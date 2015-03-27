package fsb.semantics.pso;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import ags.constraints.AtomicPredicateDictionary;
import ags.constraints.Formula;
import fsb.ast.Assume;
import fsb.ast.MProgram;
import fsb.ast.Statement;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.Validator;
import fsb.semantics.SharedResMap;
import fsb.tvl.ArithValue;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

public class PSOState extends SBState {
	SharedResMap<SharedResVal> shared;
	/**
	 * this field shoulnd't be static,
	 * it's a hack to reduce statespace,
	 * because for s
	 */
	protected int lastMem;
	
	public PSOState(MProgram program, Validator validator, int numProcs)
	{
		super();
		this.shared = new SharedResMap<SharedResVal>();
		for(int var : program.getShared())
			addShared(var, DetArithValue.getInstance(0));			
	}
	
	@SuppressWarnings("unchecked")
	public PSOState(PSOState s)
	{
		super(s);
		this.shared = s.shared;
		
		//if (!Options.LAZY_COPY)
		{
			this.shared = (SharedResMap) s.shared.clone();		
		}
	}
	
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		
		if (!super.equals(obj))
			return false;
		
		if (!(obj instanceof PSOState)) return false;	
		PSOState other = (PSOState) obj;
	
		if (!shared.equals(other.shared))
			return false;	
		
		return true;
	}
	
	@Override
	public int hashCode()
	{
		final int prime = 31;
		int result = super.hashCode();
		
		result = prime * result + shared.hashCode();
		
		return result;
	}
		
	public String toString()
	{		
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < numProcs; i++)
		{
			sb.append(i);
			sb.append(" : ");
			sb.append("PC = " + (pc[i] + 1) + " ");
			sb.append(locals[i].toString());
			sb.append("\\n");
		}
		sb.append(shared.toString());
		
		return sb.toString();
	}
	
	public boolean isFinal()
	{
		for (int i = 0; i < numProcs; i++)
		{
			if (program.getListing(i).get(pc[i]).isLast() == false)
				return false;
		}
		
		for (SharedResVal resval : shared.values())
		{
			for (int i = 0; i < numProcs; i++)
				if (!resval.buffer[i].isEmpty())
					return false;
		}
		
		return true;
	}

	@Override
	public void addShared(int str, ArithValue initval) {		
		shared.put(str, new SharedResVal(numProcs, initval));
	}
	
	public Set<Formula> getPredicates(Action taken)
	{
		Set<Formula> model = new HashSet<Formula>();
		Statement st = taken.getStatement();
		int pid = taken.getPid();
		if(st == null || pid == -1 || st.isLocal() || inAtomicSection() != -1)
			return model;
	
		for (int res : shared.keySet())
		{
			SharedResVal sharedval = shared.get(res);
			for(fsb.semantics.BufVal buf : sharedval.buffer[pid])
			{
				model.add(AtomicPredicateDictionary.makeAtomicPredicate(buf.writingLabel, taken.getStatement().getLabel()));
			}
		}
		return model;
	}

	@Override
	public ArithValue getShared(int res)
	{	
		SharedResVal val = shared.get(res);		
		if (val == null)
			throw new RuntimeException("Local " + res + " not found");
		
		return val.getGlobalVal();
	}

	@Override
	public List<SBState> focus(Assume assume, int pid) {
		throw new RuntimeException("focus is not supported for this semantics");
	}
}

