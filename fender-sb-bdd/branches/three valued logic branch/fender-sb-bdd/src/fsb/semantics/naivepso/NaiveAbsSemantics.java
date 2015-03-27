package fsb.semantics.naivepso;

import java.util.LinkedList;
import java.util.List;
import java.util.Map.Entry;

import fsb.explore.Action;
import fsb.explore.BadMemoryException;
import fsb.explore.SBState;
import fsb.explore.StateFactory;
import fsb.semantics.Semantics;

import fsb.ast.Allocate;
import fsb.ast.AtomicDec;
import fsb.ast.Branch;
import fsb.ast.CAS;
import fsb.ast.EndAtomic;
import fsb.ast.Load;
import fsb.ast.Statement;
import fsb.ast.Store;
import fsb.ast.Assign;
import fsb.ast.Swap;
import fsb.ast.tvl.DeterArithValue;

public class NaiveAbsSemantics extends Semantics {
	private StateFactory factory;
	
	public NaiveAbsSemantics()
	{
		this.factory = new NaiveAbsStateFactory();
	}
	
	protected List<Action> getFlushes(SBState s, int pid)
	{
		NaiveAbsState state = (NaiveAbsState) s;
		List<Action> succ = new LinkedList<Action>();
		//Iterate over all the shared variables
		for (String res : state.shared.keySet())
		{
			SharedResVal sharedval = state.shared.get(res);
			for(Integer flushedval : sharedval.buffer[pid].set)
			{
				//Non-Destructive
				NaiveAbsState newstate = (NaiveAbsState) factory.newState(s);
				SharedResVal newsharedval = newstate.shared.get(res);
				newsharedval.directSetValue(flushedval);
				succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH-ND " + res + 
						" = " + flushedval));		
				
				//Destructive
				newstate = (NaiveAbsState) factory.newState(s);
				newsharedval = newstate.shared.get(res);
				newsharedval.directSetValue(flushedval);
				newsharedval.flushDestructive(pid, flushedval);
				succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH-D " + res + 
						" = " + flushedval));	
			}							
		}	
		return succ;
	}	
	
	protected List<Action> applyAssign(SBState state, Assign assign, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
		
		//ASSIGN is local, just moves things inside L(p)...
		int val = ((DeterArithValue)assign.getSrcVal(state, pid)).getValue();
		String dst = assign.getDst();
		newstate.setLocal(pid, dst, DeterArithValue.getInstance(val));
		newstate.advancePC(pid);
		
		succ.add(new Action(state, newstate, assign, pid, "" + pid + 
				"." + assign.getLabel() + " " + assign.toString()));
		return succ;
	}
	
	protected List<Action> applyAllocate(SBState state, Allocate allocate, int pid) {	
		List<Action> succ = new LinkedList<Action>();			
		NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
		
		int num = ((DeterArithValue)allocate.getNumVal(state, pid)).getValue();
		int mem = newstate.allocate(num);
		String dst = allocate.getDst();
		newstate.setLocal(pid, dst, DeterArithValue.getInstance(mem));
		newstate.advancePC(pid);
		
		succ.add(new Action(state, newstate, allocate, pid, "" + pid + 
				"." + allocate.getLabel() + " " + allocate.toString()));
		return succ;
	}
	
	protected List<Action> applyStore(SBState state, Store store, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		//List<Integer> vals = store.getSrcVal(state, pid);
		int val = ((DeterArithValue)store.getSrcVal(state, pid)).getValue();
		//for (Integer val : vals) {
			NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
			String dst = store.getDst(state, pid);	
			
			//Decide whether we want to STORE-S or STORE-H.
			//The reason the real work is wrapped is because we need
			//to support lazy copying. So we never change things in the buffers
			//explicitly, but leave it to the buffer object's discretion. 
			try
			{
				SharedResVal newshared = newstate.shared.get(dst);
				newshared.buffer[pid].set.clear();
				newstate.advancePC(pid);	
				succ.add(new Action(state, newstate, store, pid, "" + pid + 
						"." + store.getLabel() + " " + store.toString()));						
			}
			catch (NullPointerException e)
			{
				throw new BadMemoryException(dst);
			}				
	//	}

		return succ;
	}
	
	protected List<Action> applyDirectStore(SBState state, Store store, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		//List<Integer> vals = store.getSrcVal(state, pid);
		int val = ((DeterArithValue)store.getSrcVal(state, pid)).getValue();
		//for (Integer val : vals) {
			NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
			String dst = store.getDst(state, pid);	
			 
			try
			{
				SharedResVal newshared = newstate.shared.get(dst);
				newshared.directSetValue(val);
				
				newstate.advancePC(pid);	
				succ.add(new Action(state, newstate, store, pid, "" + pid + 
						"." + store.getLabel() + " " + store.toString()));						
			}
			catch (NullPointerException e)
			{
				throw new BadMemoryException(dst);
			}				
	//	}

		return succ;
	}

	protected List<Action> applyLoad(SBState s, Load load, int pid) {	
		NaiveAbsState state = (NaiveAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		String src = load.getSrc(state, pid);
		String dst = load.getDst();		
		try
		{
			SharedResVal sharedval = state.shared.get(src);

			//Note that BOTH options here are possible, if
			//we have validity.TOP
			if (sharedval.isEmptySet(pid))
			{
				//LOAD-G
				int val = sharedval.getGlobalVal();
				NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
				newstate.setLocal(pid, dst, DeterArithValue.getInstance(val));
				newstate.advancePC(pid);
				succ.add(new Action(state, newstate, load, pid, "" + pid + 
						"." + load.getLabel() + " " + load.toString() + " //G"));	
			}			
			else
			{
				//LOAD-S				
				for(Integer val : sharedval.buffer[pid].set)
				{
					//Non-Destructive
					NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
					newstate.setLocal(pid, dst, DeterArithValue.getInstance(val));
					
					newstate.advancePC(pid);
					succ.add(new Action(state, newstate, load, pid, "" + pid + 
							"." + load.getLabel() + " " + load.toString() + " //" + val));	
				}							
			}
			
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(src);
		}				
		return succ;
	}	
		
	protected List<Action> applyCAS(SBState s, CAS cas, int pid) {
		NaiveAbsState state = (NaiveAbsState) s;
		List<Action> succ = new LinkedList<Action>();
		
		String sharedStr = cas.getShared(state, pid);
		SharedResVal sharedval;
		try
		{
			sharedval = state.shared.get(sharedStr);
			int val = sharedval.getGlobalVal();
			if (val == ((DeterArithValue)cas.getOldVal(state, pid)).getValue())
			{
				//CAS-T
				NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
				SharedResVal newsharedval = newstate.shared.get(sharedStr);
				newstate.advancePC(pid);
				newsharedval.directSetValue(((DeterArithValue)cas.getNewVal(state, pid)).getValue());
				newstate.setLocal(pid, cas.getRes(), DeterArithValue.getInstance(1));				
				succ.add(new Action(state, newstate, cas, pid, "" + pid + 
						"." + cas.getLabel() + " " + cas.toString() + " //G"));	
			}
			else
			{
				//CAS-F
				NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
				newstate.advancePC(pid);				
				newstate.setLocal(pid, cas.getRes(), DeterArithValue.getInstance(0));				
				succ.add(new Action(state, newstate, cas, pid, "" + pid + 
						"." + cas.getLabel() + " " + cas.toString() + " //G"));	
			}			
		} 
		catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}		
		
		return succ;
	}	
	
	
	protected List<Action> applySwap(SBState s, Swap swap, int pid) {	
		NaiveAbsState state = (NaiveAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		String sharedStr = swap.getShared(state, pid);
		String local = swap.getRes();		

		try
		{
			SharedResVal sharedval = state.shared.get(sharedStr);
			int val = sharedval.getGlobalVal();
			NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
			SharedResVal newsharedval = newstate.shared.get(sharedStr);
			newstate.advancePC(pid);

			newstate.setLocal(pid, local, DeterArithValue.getInstance(val));
			newsharedval.directSetValue(((DeterArithValue)swap.getNewVal(state, pid)).getValue());	
			
			succ.add(new Action(state, newstate, swap, pid, "" + pid + 
					"." + swap.getLabel() + " " + swap.toString() + " //G"));	
			
			
		} 
		catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}				
		return succ;
	}	
		
	
	protected List<Action> applyAtomicDec(SBState s, AtomicDec dec, int pid) {		
		NaiveAbsState state = (NaiveAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		String sharedStr = dec.getShared(state, pid);
		String local = dec.getRes();		

		try
		{
			SharedResVal sharedval = state.shared.get(sharedStr);
			int val = sharedval.getGlobalVal();
			
			NaiveAbsState newstate = (NaiveAbsState) factory.newState(state);
			SharedResVal newsharedval = newstate.shared.get(sharedStr);
			newstate.advancePC(pid);

			newstate.setLocal(pid, local, DeterArithValue.getInstance(val));
			newsharedval.directSetValue(val - 1);	
			
			succ.add(new Action(state, newstate, dec, pid, "" + pid + 
					"." + dec.getLabel() + " " + dec.toString() + " //G"));	
			
			
		} 
		catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}				
		return succ;
	}	
	
	protected List<Action> applyBarrier(SBState origState, Statement statement, int pid) {
		List<Action> succ = new LinkedList<Action>();		
		
		NaiveAbsState newState = (NaiveAbsState) factory.newState(origState);		
		if(!statement.isLast())			
			newState.advancePC(pid);
		
		succ.add(new Action(origState, newState, statement, pid, "" + pid + 
			"." + statement.getLabel() + " " + statement.toString()));
		return succ;
	}
	
	protected List<Action> applyEndAtomic(SBState state, EndAtomic statement, int pid)
	{
		throw new RuntimeException("Not supported under Naive Set Abstraction");
		//TODO: This requires computing the Cartesian product of possible values for all variables.
		//Later!
	}

	
	@Override
	public StateFactory getStateFactory() {		
		return factory;
	}
	
	public boolean isEnabled(SBState s, int pid, Statement statement) {
		NaiveAbsState state = (NaiveAbsState) s;
		String sharedStr = "BAD!";
		try
		{
			switch (statement.getType())
			{
			case CAS:
				{
					CAS cas = (CAS) statement;
					sharedStr = cas.getShared(state, pid);
					SharedResVal sharedval;
					sharedval = state.shared.get(sharedStr);
					return (sharedval.isEmptySet(pid));
				}
			case SWAP:
				{
					Swap swap = (Swap) statement;
					sharedStr = swap.getShared(state, pid);
					SharedResVal sharedval;
					sharedval = state.shared.get(sharedStr);
					return (sharedval.isEmptySet(pid));
				}
			case ATOMICDEC:
				{
					AtomicDec dec = (AtomicDec) statement;
					sharedStr = dec.getShared(state, pid);
					SharedResVal sharedval;
					sharedval = state.shared.get(sharedStr);
					return (sharedval.isEmptySet(pid));
				}				
			case BARRIER:
				{
					for (Entry<String, SharedResVal> entry : state.shared.entrySet())
					{
						SharedResVal sharedval = entry.getValue();
						if (!sharedval.isEmptySet(pid))
							return false;
					}
					return true;
				}
			default:
				return true;
			}
		}
		catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}	

	}
}
