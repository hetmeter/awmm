package fsb.semantics.sc;


import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.WeakHashMap;

import ags.constraints.AtomicPredicateDictionary;
import ags.constraints.BDDFormula;
import ags.constraints.ConjunctionFormula;
import ags.constraints.FalseFormula;
import ags.constraints.Formula;
import fsb.ast.MProgram;
import fsb.ast.Statement;
import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.Validator;
import fsb.semantics.pso.SharedResVal;

public class SCState extends SBState {
	@SuppressWarnings("serial")
	protected class SharedResMap extends TreeMap<String, ArithValue> {}	
	private static WeakHashMap<SharedResMap, SharedResMap> cachedShared = new WeakHashMap<SharedResMap, SharedResMap>();
	private Set<Action> inComing = new HashSet<Action>();
	private Set<Action> out_going = new HashSet<Action>();
	private SharedResMap shared;
	private Formula avoidFormula = null;

	protected Validator validator;
	
	public SCState(MProgram program, Validator validator, int numProcs)
	{
		super(program, validator, numProcs);
		this.shared = new SharedResMap();	
		for(String var : program.getShared())
			addShared(var, DeterArithValue.getInstance(0));
	}
	
	public void registerAction(Action in){
		this.inComing.add(in);
		((SCState)(in.getSource())).out_going.add(in);
	}
		
		
	public SCState(SCState s)
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
		if (!super.equals(obj))
			return false;
		
		if (!(obj instanceof SCState)) return false;	
		SCState other = (SCState) obj;

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
		sb.append("<Shared:" + shared.toString());
		for (int i = 0; i < numProcs; i++)
		{
			sb.append(",<pid" + i + ", " + "PC=" + program.getListing(i).get(pc[i]).getLabel() + ", ");			
			sb.append(locals[i].toString() + "> ");
		}
		sb.append(">\n");
	
		return sb.toString();
	}
	
	public boolean isFinal()
	{
		for (int i = 0; i < numProcs; i++)
			if (program.getListing(i).get(pc[i]).isLast() == false)
				return false;
		
		return true;
	}

	@Override
	public void addShared(String str, ArithValue initval) {
		shared.put(str, initval);		
		SharedResMap cached = cachedShared.get(shared);
		if (cached == null)
			cachedShared.put(shared, shared);
		else
			shared = cached;
	}
	
	@Override
	public ArithValue getShared(String res)
	{
		return shared.get(res);
	}
	
	public void setShared(String res, ArithValue val)
	{
		shared.put(res, val);
		
		SharedResMap cached = cachedShared.get(shared);
		if (cached == null)
			cachedShared.put(shared, shared);
		else
			shared = cached;
	}

	@Override
	public boolean disjPredicates() {
		
		return false;
	};
	/**
	 * given an action coming <b>out</b> of the current state
	 * returns a formula to avoid it. 
	 */
	@Override
	public Formula getAvoidFormula(Action taken) {
	
		if(avoidFormula != null) return avoidFormula;
		
		Formula model = Formula.makeConjunction();

		Map<Action,Action> alternatives = CalculateAlternatives(taken);
		Statement st = taken.getStatement();
		int taken_pid = taken.getPid();
		
		if(st == null || taken_pid == -1  || inAtomicSection() != -1)//inAtomicSection != -1 since if in atomic section not possible to prevent
			return avoidFormula=model;
		if(alternatives.isEmpty()){
			return avoidFormula=Formula.falseValue();
		}
		
		for (Map.Entry<Action, Action> entrya : alternatives.entrySet())
		{
			Action prev_action = entrya.getKey();
			Action couple_with_action = entrya.getValue();
			SCState predecessor = (SCState)prev_action.getSource(); 
					
			Formula avoid_predecessor_formula = predecessor.getAvoidFormula(prev_action);
			if(couple_with_action == null){
				//entering arc with same pid as then one to avoid
				//only option is to try to avoid the predessesor
				model = model.and(avoid_predecessor_formula);
			}else{
				Formula atomicity_pred = AtomicPredicateDictionary.makeAtomicPredicate(prev_action.getStatement().getLabel(),
						couple_with_action.getStatement().getLabel());
				
				// avoiding the transition to this state(atomicity_pred) or the previous state.
				model = model.and(atomicity_pred.or(avoid_predecessor_formula));
			}
			//model.add(AtomicPredicateDictionary.makeAtomicPredicate(buf.writingLabel, taken.getStatement().getLabel()));
			
		}
	
	 
		return avoidFormula=model;
	}
	@Override
	public Set<Formula> getPredicates(Action taken) {
		return null;
	}
	
	
/**
 * calculates all the possible coupling(atomic section placements)
 *  of action that could of been done to prevent the current transition.<br>
 * if one of the incoming actions belongs to a thread with the same pid as the one to be prevented,
 * it is coupled with null ! 
 * @param predState
 * @param taken
 * @return
 */
 private Map<Action, Action> CalculateAlternatives(Action taken) {
		// TODO Auto-generated method stub
		Map<Action,Action> coupling = new HashMap<Action, Action>();
		
		
		for(Action into_predecessor : inComing){
			Set<Action> coupleWith = new HashSet<Action>();
			int out_action_pid = into_predecessor.getPid();
			if (out_action_pid != taken.getPid()) {
				coupleWith = getActionsWithSamePid(out_going, out_action_pid);

				for (Action partner : coupleWith) {
					coupling.put(into_predecessor, partner);
				}
			}else{ // couples the outFromPredecessor with null
				coupling.put(into_predecessor, null);
			}
		}
		return coupling;
	}

private Set<Action> getActionsWithSamePid(
		Set<Action> potential_to_couple_with, int pid) {
	Set<Action> mates = new HashSet<Action>();
	
	for(Action a : potential_to_couple_with){
		if(a.getPid() == pid){
			mates.add(a);
		}
	}
	return mates;
}
}

