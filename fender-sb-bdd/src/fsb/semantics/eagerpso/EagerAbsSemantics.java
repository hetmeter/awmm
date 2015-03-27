package fsb.semantics.eagerpso;

import java.util.LinkedList;
import java.util.List;
import java.util.Map.Entry;

import fsb.explore.Action;
import fsb.explore.BadMemoryException;
import fsb.explore.SBState;
import fsb.explore.StateFactory;
import fsb.semantics.BufVal;
import fsb.semantics.Semantics;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;

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
import fsb.ast.SymbolTable;

public class EagerAbsSemantics extends Semantics {
	private StateFactory factory;
	
	//TODO: Should be external
	static int headlen = 0;

	public EagerAbsSemantics(int len)
	{
		this.factory = new EagerAbsStateFactory();
		headlen = len;
	}
	
	protected List<Action> getFlushes(SBState s, int pid)
	{
		EagerAbsState state = (EagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();
		//Iterate over all the shared variables
		for (int res: state.shared.keySet())
		{
			SharedResVal sharedval = state.shared.get(res);
			if (!sharedval.isEmptyHead(pid))
			{
				//Flush from head, since it's non-empty
				EagerAbsState newstate = (EagerAbsState) factory.newState(s);
				SharedResVal newsharedval = newstate.shared.get(res);
				newsharedval.flushFromHead(pid);
				succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH-H " + res + 
						" = " + newsharedval.getGlobalVal()));		
			}
			else
			{
				//Flush from set
				for(BufVal bufval : sharedval.buffer[pid].set)
				{
					//Non-Destructive
					ArithValue flushedval = bufval.contents;
					EagerAbsState newstate = (EagerAbsState) factory.newState(s);
					SharedResVal newsharedval = newstate.shared.get(res);
					newsharedval.directSetValue(flushedval);
					succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH-SN " + res + 
							" = " + flushedval));		
					
					
					//Destructive
					if (sharedval.getLast(pid) != flushedval || sharedval.isSingleton(pid))
					{
						newstate = (EagerAbsState) factory.newState(s);
						newsharedval = newstate.shared.get(res);
						newsharedval.flushDestructive(pid, bufval);
						succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH-SD " + res + 
								" = " + flushedval));
					}					
				}			
			}
		}	
		return succ;
	}
	
	protected List<Action> applyAssign(SBState state, Assign assign, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		EagerAbsState newstate = (EagerAbsState) factory.newState(state);
		
		//ASSIGN is local, just moves things inside L(p)...
		ArithValue val = assign.getSrcVal(state, pid);
		int dst = assign.getDst();
		newstate.setLocal(pid, dst, val);
		newstate.advancePC(pid);
		
		succ.add(new Action(state, newstate, assign, pid, "" + pid + 
				"." + assign.getLabel() + " " + assign.toString()));
		return succ;
	}
	
	protected List<Action> applyAllocate(SBState state, Allocate allocate, int pid) {	
		List<Action> succ = new LinkedList<Action>();			
		EagerAbsState newstate = (EagerAbsState) factory.newState(state);
		
		int num = allocate.getNumVal(state, pid);
		int mem = newstate.allocate(num);
		int dst = allocate.getDst();
		newstate.setLocal(pid, dst, DetArithValue.getInstance(mem));
		newstate.advancePC(pid);
		
		succ.add(new Action(state, newstate, allocate, pid, "" + pid + 
				"." + allocate.getLabel() + " " + allocate.toString()));
		return succ;
	}
	
	protected List<Action> applyStore(SBState state, Store store, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		ArithValue val = store.getSrcVal(state, pid);
		
		//for (ArithValue val : vals) {
			EagerAbsState newstate = (EagerAbsState) factory.newState(state);
			int dst = store.getDst(state, pid);	
			//Decide whether we want to STORE-S or STORE-H.
			//The reason the real work is wrapped is because we need
			//to support lazy copying. So we never change things in the buffers
			//explicitly, but leave it to the buffer object's discretion. 
			try
			{
				SharedResVal newshared = newstate.shared.get(dst);
				newshared.store(pid, val, store.getLabel());
				newstate.advancePC(pid);
				succ.add(new Action(state, newstate, store, pid, "" + pid + 
						"." + store.getLabel() + " " + store.toString()));
		
				
			}
			catch (NullPointerException e)
			{
				throw new  BadMemoryException(SymbolTable.getGlobalVariableName(dst));
			}				
		//}

		return succ;
	}

	protected List<Action> applyDirectStore(SBState state, Store store, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		ArithValue val = store.getSrcVal(state, pid);
		
		//for (ArithValue val : vals) {
			EagerAbsState newstate = (EagerAbsState) factory.newState(state);
			int dst = store.getDst(state, pid);	
			try
			{
				SharedResVal newshared = newstate.shared.get(dst);
				
				newshared.clearBuffer(pid);
				newshared.directSetValue(val);
				
				newstate.advancePC(pid);
				succ.add(new Action(state, newstate, store, pid, "" + pid + 
						"." + store.getLabel() + " " + store.toString()));
		
				
			}
			catch (NullPointerException e)
			{
				throw new  BadMemoryException(SymbolTable.getGlobalVariableName(dst));
			}				
		//}

		return succ;
	}
	
	protected List<Action> applyLoad(SBState s, Load load, int pid) {		
		EagerAbsState state = (EagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		int src = load.getSrc(state, pid);
		int dst = load.getDst();		
		try
		{
			SharedResVal sharedval = state.shared.get(src);

			if (!sharedval.nonEmpty(pid))
			{
				//LOAD-G
				ArithValue val = sharedval.getGlobalVal();
				EagerAbsState newstate = (EagerAbsState) factory.newState(state);
				newstate.setLocal(pid, dst, val);
				newstate.advancePC(pid);
				succ.add(new Action(state, newstate, load, pid, "" + pid + 
						"." + load.getLabel() + " " + load.toString() + " //G " + val));	
			}			
			else
			{
				//LOAD-S				
				EagerAbsState newstate = (EagerAbsState) factory.newState(state);
				ArithValue val = sharedval.getLast(pid);
				newstate.setLocal(pid, dst, val);
				
				newstate.advancePC(pid);
				succ.add(new Action(state, newstate, load, pid, "" + pid + 
						"." + load.getLabel() + " " + load.toString() + " //B " + val));										
			}
			
		} catch (NullPointerException e)
		{
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(src));
		}				
		return succ;
	}	
		
	protected List<Action> applyCAS(SBState s, CAS cas, int pid) {
		EagerAbsState state = (EagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();
		
		int sharedStr = cas.getShared(state, pid);
		SharedResVal sharedval;
		try
		{
			sharedval = state.shared.get(sharedStr);
			ArithValue val = sharedval.getGlobalVal();
			if (val == cas.getOldVal(state, pid))
			{
				//CAS-T
				EagerAbsState newstate = (EagerAbsState) factory.newState(state);
				SharedResVal newsharedval = newstate.shared.get(sharedStr);
				newstate.advancePC(pid);
				newsharedval.directSetValue(cas.getNewVal(state, pid));
				newstate.setLocal(pid, cas.getRes(), DetArithValue.getInstance(1));				
				succ.add(new Action(state, newstate, cas, pid, "" + pid + 
						"." + cas.getLabel() + " " + cas.toString() + " //G"));	
			}
			else
			{
				//CAS-F
				EagerAbsState newstate = (EagerAbsState) factory.newState(state);
				newstate.advancePC(pid);				
				newstate.setLocal(pid, cas.getRes(), DetArithValue.getInstance(0));				
				succ.add(new Action(state, newstate, cas, pid, "" + pid + 
						"." + cas.getLabel() + " " + cas.toString() + " //G"));	
			}			
		} 
		catch (NullPointerException e)
		{
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(sharedStr));
		}		
		
		return succ;
	}	
	
	
	protected List<Action> applySwap(SBState s, Swap swap, int pid) {
		EagerAbsState state = (EagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		int sharedStr = swap.getShared(state, pid);
		int local = swap.getRes();		

		try
		{
			SharedResVal sharedval = state.shared.get(sharedStr);
			ArithValue val = sharedval.getGlobalVal();
			EagerAbsState newstate = (EagerAbsState) factory.newState(state);
			SharedResVal newsharedval = newstate.shared.get(sharedStr);
			newstate.advancePC(pid);

			newstate.setLocal(pid, local, val);
			newsharedval.directSetValue(swap.getNewVal(state, pid));	
			
			succ.add(new Action(state, newstate, swap, pid, "" + pid + 
					"." + swap.getLabel() + " " + swap.toString() + " //G"));	
			
			
		} 
		catch (NullPointerException e)
		{
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(sharedStr));
		}				
		return succ;
	}	
	
	protected List<Action> applyAtomicDec(SBState s, AtomicDec dec, int pid) {
		EagerAbsState state = (EagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		int sharedStr = dec.getShared(state, pid);
		int local = dec.getRes();		

		try
		{
			SharedResVal sharedval = state.shared.get(sharedStr);
			ArithValue val = sharedval.getGlobalVal();
			EagerAbsState newstate = (EagerAbsState) factory.newState(state);
			SharedResVal newsharedval = newstate.shared.get(sharedStr);
			newstate.advancePC(pid);

			newstate.setLocal(pid, local, val);
			newsharedval.directSetValue(val.minus(DetArithValue.getInstance(1)));	
			
			succ.add(new Action(state, newstate, dec, pid, "" + pid + 
					"." + dec.getLabel() + " " + dec.toString() + " //G"));	
			
			
		} 
		catch (NullPointerException e)
		{
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(sharedStr));
		}				
		return succ;
	}
		
	protected List<Action> applyBarrier(SBState origState, Statement statement, int pid) {
		List<Action> succ = new LinkedList<Action>();		
		
		EagerAbsState newState = (EagerAbsState) factory.newState(origState);		
		if(!statement.isLast())			
			newState.advancePC(pid);
		
		succ.add(new Action(origState, newState, statement, pid, "" + pid + 
			"." + statement.getLabel() + " " + statement.toString()));
		return succ;
	}
	
	protected List<Action> applyEndAtomic(SBState state, EndAtomic statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		EagerAbsState newstate = (EagerAbsState) factory.newState(state);
		
		for (int res: newstate.shared.keySet())
		{
			SharedResVal sharedval = newstate.shared.get(res);
			if (sharedval.nonEmpty(pid))
			{
				ArithValue val = sharedval.getLast(pid);
				sharedval.clearBuffer(pid);
				sharedval.directSetValue(val);
			}
		}		
		newstate.unsetAtomic();
		
		newstate.advancePC(pid);
		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
				"." + statement.getLabel() + " " + statement.toString()));
		return succ;
	}


	
	@Override
	public StateFactory getStateFactory() {		
		return factory;
	}
	
	public boolean isEnabled(SBState s, int pid, Statement statement) {
		EagerAbsState state = (EagerAbsState) s;
		int sharedStr = SymbolTable.getOrCreateGlobalVariable("BAD!");
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
					return (!sharedval.nonEmpty(pid));
				}
			case SWAP:
				{
					Swap swap = (Swap) statement;
					sharedStr = swap.getShared(state, pid);
					SharedResVal sharedval;
					sharedval = state.shared.get(sharedStr);
					return (!sharedval.nonEmpty(pid));
				}
			case ATOMICDEC:
				{
					AtomicDec dec = (AtomicDec) statement;
					sharedStr = dec.getShared(state, pid);
					SharedResVal sharedval;
					sharedval = state.shared.get(sharedStr);
					return (!sharedval.nonEmpty(pid));
				}					
			case BARRIER:
				{
					for (Entry<Integer, SharedResVal> entry : state.shared.entrySet())
					{
						SharedResVal sharedval = entry.getValue();
						if (sharedval.nonEmpty(pid))
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
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(sharedStr));
		}	

	}
}
