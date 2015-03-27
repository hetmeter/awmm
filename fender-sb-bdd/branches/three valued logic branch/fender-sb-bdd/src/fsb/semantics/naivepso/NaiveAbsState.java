package fsb.semantics.naivepso;


import java.util.Set;

import ags.constraints.Formula;
import fsb.ast.MProgram;
import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.Validator;
import fsb.semantics.SharedResMap;

public class NaiveAbsState extends SBState {
	SharedResMap<SharedResVal> shared;

	protected Validator validator;
	
	public NaiveAbsState(MProgram program, Validator validator, int numProcs)
	{
		super(program, validator, numProcs);
		this.shared = new SharedResMap<SharedResVal>();
		for(String var : program.getShared())
			addShared(var, DeterArithValue.getInstance(0));
	}
	
	@SuppressWarnings("unchecked")
	public NaiveAbsState(NaiveAbsState s)
	{
		super(s);
		//this.shared = s.shared;		
		//if (!Options.LAZY_COPY)
		//{
			this.shared = (SharedResMap) s.shared.clone();		
		//}
	}
	
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		
		if (!super.equals(obj))
			return false;
		
		if (!(obj instanceof NaiveAbsState)) return false;	
		NaiveAbsState other = (NaiveAbsState) obj;
	
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
		for (int pid = 0; pid < numProcs; pid++)
		{
			if (program.getListing(pid).get(pc[pid]).isLast() == false)
				return false;
		}
		
		for (SharedResVal resval : shared.values())
		{
			for (int pid = 0; pid < numProcs; pid++)
				if (!resval.isEmptySet(pid))
					return false;
		}
		
		return true;
	}

	@Override
	public void addShared(String str, ArithValue initval) {
		shared.put(str, new SharedResVal(numProcs, 0));		
	}

	@Override
	public Set<Formula> getPredicates(Action taken) {
		// TODO Auto-generated method stub
		return null;
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

