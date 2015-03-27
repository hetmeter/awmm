package fsb.semantics.empeagerpso;


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
import fsb.explore.State;
import fsb.explore.Validator;
import fsb.semantics.BufVal;
import fsb.semantics.SharedResMap;

public class EmpEagerAbsState extends SBState {
	SharedResMap<SharedResVal> shared;

	protected Validator validator;
	
	public EmpEagerAbsState(MProgram program, Validator validator, int numProcs)
	{
		super(program, validator, numProcs);
		this.shared = new SharedResMap<SharedResVal>();
		for(String var : program.getShared())
			addShared(var, DeterArithValue.getInstance(0));
	}
	
	@SuppressWarnings("unchecked")
	public EmpEagerAbsState(EmpEagerAbsState s)
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
		
		if (!(obj instanceof EmpEagerAbsState)) return false;	
		EmpEagerAbsState other = (EmpEagerAbsState) obj;
	
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
				
		//TODO: Is this required here? I think not, because END is only enabled when the buffer is empty...
		for (SharedResVal resval : shared.values())
		{
			for (int pid = 0; pid < numProcs; pid++)
				if (!resval.isEmpty(pid))
					return false;
		}
		
		return true;
	}

	@Override
	public void addShared(String str, ArithValue initval) {
		shared.put(str, new SharedResVal(numProcs, 0));		
	}

	@Override
	public boolean join(State s)
	{
		if(!equals(s))
			throw new RuntimeException("Trying to join inappropriate states!");
		
		EmpEagerAbsState other = (EmpEagerAbsState) s;
		boolean changed = false;
		for (String res : shared.keySet())
		{
			SharedResVal resval = shared.get(res);
			changed |= resval.join(other.shared.get(res));
		}
		return changed;
	}
	
	@Override
	public Set<Formula> getPredicates(Action taken)
	{
		throw new RuntimeException("Should not be used for this type of state, use getAvoidFormula() instead!");
	}
	
	protected boolean isAvoidableType(Action taken)
	{
		Statement st = taken.getStatement();
		if (st == null || taken.getPid() == -1 || st.isLocal() || st.getType() == StatType.END || inAtomicSection() != -1)
			return false;
		
		return true;
	}
	
	@Override
	public boolean isAvoidable(Action taken)
	{
		if(!isAvoidableType(taken))
			return false;
		
		int pid = taken.getPid();
		for (String res : shared.keySet())
		{
			SharedResVal sharedval = shared.get(res);
			if (!(sharedval.isEmpty(pid)))
				return true;
		}
		return false;
	}
	
	protected void addPredicate(Set<Formula> model, BufVal buf, Statement st)
	{
		model.add(AtomicPredicateDictionary.makeAtomicPredicate(buf.writingLabel, st.getLabel()));
	}
	
	public boolean disjPredicates() {
		return false;
	}
	
	@Override
	public Formula getAvoidFormula(Action taken)
	{
		Formula avoidFormula = Formula.falseValue();
		//avoidFormula = avoidFormula.and(Formula.makeDisjunction());
		if (!isAvoidableType(taken))
			return avoidFormula;
		
		Statement st = taken.getStatement();
		int pid = taken.getPid();
		
		Formula common = Formula.falseValue();
		for (String res : shared.keySet())
		{
			SharedResVal sharedval = shared.get(res);
			for(BufVal buf : sharedval.buffer[pid].head)
			{
				common = common.or(AtomicPredicateDictionary.makeAtomicPredicate(buf.writingLabel, st.getLabel()));
			}
		}
		
		Formula allSets = Formula.falseValue();
		//allSets = allSets.and(Formula.makeDisjunction());
		for (String res : shared.keySet())
		{
			//Unfortunately, within a set we need to avoid everything.
			SharedResVal sharedval = shared.get(res);
			if (!sharedval.buffer[pid].set.isEmpty())
			{
				Formula setFormula = Formula.trueValue();
				for(BufVal buf : sharedval.buffer[pid].set)
				{
					Formula disj = Formula.falseValue();
					Formula ap = AtomicPredicateDictionary.makeAtomicPredicate(buf.writingLabel, st.getLabel());
					disj = disj.or(ap);
					//System.out.println("DISJ : " + disj);
					setFormula = setFormula.and(disj);
				}
				//System.out.println("setFormula : " + setFormula + " --- " + setFormula.getClass());
				//System.out.println("ALLSETS : " + allSets + " --- " + allSets.getClass());
				allSets = allSets.or(setFormula);
			}
		}
		
		avoidFormula = avoidFormula.or(common);
		avoidFormula = avoidFormula.or(allSets);
		return avoidFormula;
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

