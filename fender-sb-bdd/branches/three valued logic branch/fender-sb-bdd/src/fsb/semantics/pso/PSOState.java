package fsb.semantics.pso;

import java.util.HashSet;
import java.util.Set;

import ags.constraints.AtomicPredicateDictionary;
import ags.constraints.Formula;
import fsb.ast.MProgram;
import fsb.ast.Statement;
import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.Validator;
import fsb.semantics.SharedResMap;

public class PSOState extends SBState {
	SharedResMap<SharedResVal> shared;
	
	public PSOState(MProgram program, Validator validator, int numProcs)
	{
		super(program, validator, numProcs);
		this.shared = new SharedResMap<SharedResVal>();
		for(String var : program.getShared())
			addShared(var, DeterArithValue.getInstance(0));			
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
	public void addShared(String str, ArithValue initval) {		
		shared.put(str, new SharedResVal(numProcs, ((DeterArithValue)initval).getValue()));
	}
	
	public Set<Formula> getPredicates(Action taken)
	{
		Set<Formula> model = new HashSet<Formula>();
		Statement st = taken.getStatement();
		int pid = taken.getPid();
		if(st == null || pid == -1 || st.isLocal() || inAtomicSection() != -1)
			return model;
	
		for (String res : shared.keySet())
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
	public ArithValue getShared(String res)
	{	
		SharedResVal val = shared.get(res);		
		if (val == null)
			throw new RuntimeException("Local " + res + " not found");
		
		return DeterArithValue.getInstance(val.getGlobalVal());
	}
}

