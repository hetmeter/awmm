package fsb.semantics.eagerpso;


import java.util.HashSet;
import java.util.Set;

import ags.constraints.AtomicPredicateDictionary;
import ags.constraints.Formula;
import fsb.ast.MProgram;
import fsb.ast.Statement;
import fsb.ast.Statement.StatType;
import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.Validator;
import fsb.semantics.BufVal;
import fsb.semantics.SharedResMap;

public class EagerAbsState extends SBState {
	SharedResMap<SharedResVal> shared;

	protected Validator validator;
	
	public EagerAbsState(MProgram program, Validator validator, int numProcs)
	{
		super(program, validator, numProcs);
		this.shared = new SharedResMap<SharedResVal>();
		for(String var : program.getShared())
			addShared(var, DeterArithValue.getInstance(0));
	}
	
	@SuppressWarnings("unchecked")
	public EagerAbsState(EagerAbsState s)
	{
		super(s);
		//this.shared = s.shared;		
		//if (!Options.LAZY_COPY)
		//{
			this.shared = (SharedResMap<SharedResVal>) s.shared.clone();		
		//}
	}
	
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		
		if (!super.equals(obj))
			return false;
		
		if (!(obj instanceof EagerAbsState)) return false;	
		EagerAbsState other = (EagerAbsState) obj;
	
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
				if (resval.nonEmpty(pid))
					return false;
		}
		
		return true;
	}

	@Override
	public void addShared(String str, ArithValue initval) {
		shared.put(str, new SharedResVal(numProcs, 0));		
	}
	
	@Override
	//The avoid formula for a transition depends on two things:
	//a) Properties of the source state (this)
	//b) Properties of the outgoing transition (taken)
	//
	//A formula returned by getPredicates() is by definition a disjunction.
	//We represent this disjunction by simply returning a set of disjuncts.
	//An empty set represents the empty disjunction, that is, "false".
	public Set<Formula> getPredicates(Action taken)
	{		
		Set<Formula> model = new HashSet<Formula>();
		//If the transition is, unavoidable, the avoid formula is false.
		if (!isAvoidableType(taken))			
			return model;
		
		Statement st = taken.getStatement();
		int pid = taken.getPid();

		//The transition might be avoidable. 
		//Determine how we can avoid it by going through all values potentially
		//in the buffer for current process.
		//Each such value adds a single atomic predicate to the disjunction.
		//Note that we may still return an empty set, if all buffers are empty.
		for (String res : shared.keySet())
		{
			SharedResVal sharedval = shared.get(res);
			for(BufVal buf : sharedval.buffer[pid].head)
			{
				addPredicate(model, buf, st);
			}
			for(BufVal buf : sharedval.buffer[pid].set)
			{
				addPredicate(model, buf, st);
			}
		}
		return model;
	}
	
	protected boolean isAvoidableType(Action taken)
	{
		Statement st = taken.getStatement();
		//Which transitions are unavoidable?
		//1) If there is no statement associated with the transition.
		//2) The transition is part of the init section.
		//3) The associated statement is local (not a memory operation).
		//4) The associated statement is inside an atomic section.
		if (st == null || taken.getPid() == -1 || st.isLocal() || st.getType() == StatType.END || inAtomicSection() != -1)
			return false;
		
		return true;
	}
	
	@Override
	public boolean isAvoidable(Action taken)
	{
		//The transition may be unavoidable for some "trivial" reason.
		//Check that first.
		if(!isAvoidableType(taken))
			return false;
		
		//Ok, now check that it has at least one non-empty buffer.
		//This isn't strictly necessary, but helps debugging.
		int pid = taken.getPid();
		for (String res : shared.keySet())
		{
			SharedResVal sharedval = shared.get(res);
			if (sharedval.nonEmpty(pid))
				return true;
		}
		
		return false;
	}
	
	protected void addPredicate(Set<Formula> model, BufVal buf, Statement st)
	{
		model.add(AtomicPredicateDictionary.makeAtomicPredicate(buf.writingLabel, st.getLabel()));
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

