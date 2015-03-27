package fsb.semantics.empeagerpso;

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

public class EmpEagerAbsSemantics extends Semantics {
	private StateFactory factory;
	
	//TODO: Should be external
	static int headlen = 0;

	public EmpEagerAbsSemantics(int len)
	{
		this.factory = new EmpEagerAbsStateFactory();
		headlen = len;
	}
	
	protected List<Action> getFlushes(SBState s, int pid)
	{
		EmpEagerAbsState state = (EmpEagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();
		//Iterate over all the shared variables
		for (int res : state.shared.keySet())
		{
			SharedResVal sharedval = state.shared.get(res);
			if (!sharedval.isEmptyHead(pid))
			{
				//Flush from head, since it's non-empty
				EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(s);
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
					EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(s);
					SharedResVal newsharedval = newstate.shared.get(res);
					newsharedval.directSetValue(flushedval);
					succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH-SN " + res + 
							" = " + flushedval));								
				}					

				//Very Destructive!
				if (!sharedval.isEmpty(pid))
				{
					EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(s);
					SharedResVal newsharedval = newstate.shared.get(res);
					ArithValue last = sharedval.getLast(pid);
					newsharedval.directSetValue(last);
					newsharedval.clearBuffer(pid);
					succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH-SD " + res + 
								" = " + last));
				}
			}			
		}	
		return succ;
	}
	
	protected List<Action> applyAssign(SBState state, Assign assign, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
		
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
		EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
		
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
		
		//for (Integer val : vals) {
			EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
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
				throw new BadMemoryException(SymbolTable.getGlobalVariableName(dst));
			}	
		//}
		
		return succ;
	}
	
	protected List<Action> applyDirectStore(SBState state, Store store, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		ArithValue val = store.getSrcVal(state, pid);
		
		//for (Integer val : vals) {
			EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
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
				throw new BadMemoryException(SymbolTable.getGlobalVariableName(dst));
			}				
		//}

		return succ;
	}

	protected List<Action> applyLoad(SBState s, Load load, int pid) {		
		EmpEagerAbsState state = (EmpEagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		int src = load.getSrc(state, pid);
		int dst = load.getDst();
		SharedResVal sharedval = state.shared.get(src);

		try
		{			
			EmpEagerAbsState newstate;
			ArithValue val;
			if (sharedval.isEmpty(pid))
			{
				//LOAD-G
				newstate = (EmpEagerAbsState) factory.newState(state);
				val = sharedval.getGlobalVal();			
				newstate.setLocal(pid, dst, val);
				newstate.advancePC(pid);
				succ.add(new Action(state, newstate, load, pid, "" + pid + 
						"." + load.getLabel() + " " + load.toString() + " //G " + val));	
			}			
			else
			{
				//LOAD-B
				newstate = (EmpEagerAbsState) factory.newState(state);
				val = sharedval.getLast(pid);
				newstate.setLocal(pid, dst, val);
				
				newstate.advancePC(pid);
				succ.add(new Action(state, newstate, load, pid, "" + pid + 
						"." + load.getLabel() + " " + load.toString() + " //B " + val));										
			}
			
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(SymbolTable.getGlobalVariableName(src));
		}				
		return succ;
	}	
		
	protected List<Action> applyCAS(SBState state, CAS cas, int pid) {
		List<Action> succ = new LinkedList<Action>();
		
		int sharedStr = cas.getShared(state, pid);
		try
		{
			EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
			SharedResVal newsharedval = newstate.shared.get(sharedStr);
			/* Unneeded
			if (!newsharedval.isEmpty(pid))
				newsharedval.directSetValue(newsharedval.getLast(pid));
			*/			
			ArithValue val = newsharedval.getGlobalVal();
			
			if (val == cas.getOldVal(state, pid))
			{
				//CAS-T
				newstate.advancePC(pid);
				newsharedval.directSetValue(cas.getNewVal(state, pid));
				newstate.setLocal(pid, cas.getRes(), DetArithValue.getInstance(1));			
				newsharedval.clearBuffer(pid);
				succ.add(new Action(state, newstate, cas, pid, "" + pid + 
						"." + cas.getLabel() + " " + cas.toString() + " //T"));	
			}
			else
			{
				//CAS-F
				newstate.advancePC(pid);				
				newstate.setLocal(pid, cas.getRes(), DetArithValue.getInstance(0));	
				newsharedval.clearBuffer(pid);
				succ.add(new Action(state, newstate, cas, pid, "" + pid + 
						"." + cas.getLabel() + " " + cas.toString() + " //F"));	
			}			
		} 
		catch (NullPointerException e)
		{
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(sharedStr));
		}		
		
		return succ;
	}	
	
	
	protected List<Action> applySwap(SBState state, Swap swap, int pid) {		
		List<Action> succ = new LinkedList<Action>();		
		int sharedStr = swap.getShared(state, pid);
		int local = swap.getRes();		

		try
		{
			EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
			SharedResVal newsharedval = newstate.shared.get(sharedStr);		
			ArithValue val = newsharedval.getGlobalVal();

			newstate.advancePC(pid);
			newstate.setLocal(pid, local, val);			
			newsharedval.directSetValue(swap.getNewVal(state, pid));	
			newsharedval.clearBuffer(pid);
			
			succ.add(new Action(state, newstate, swap, pid, "" + pid + 
					"." + swap.getLabel() + " " + swap.toString() + " //G"));	
			
			
		} 
		catch (NullPointerException e)
		{
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(sharedStr));
		}				
		return succ;
	}	
	
	protected List<Action> applyAtomicDec(SBState state, AtomicDec dec, int pid) {		
		List<Action> succ = new LinkedList<Action>();		
		int sharedStr = dec.getShared(state, pid);
		int local = dec.getRes();		

		try
		{
			EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
			SharedResVal newsharedval = newstate.shared.get(sharedStr);		
			ArithValue val = newsharedval.getGlobalVal();

			newstate.advancePC(pid);
			newstate.setLocal(pid, local, val);			
			newsharedval.directSetValue(val.minus(DetArithValue.getInstance(1)));	
			newsharedval.clearBuffer(pid);
			
			succ.add(new Action(state, newstate, dec, pid, "" + pid + 
					"." + dec.getLabel() + " " + dec.toString() + " //G"));	
			
			
		} 
		catch (NullPointerException e)
		{
			throw new  BadMemoryException(SymbolTable.getGlobalVariableName(sharedStr));
		}				
		return succ;
	}	
		
	protected List<Action> applyBarrier(SBState s, Statement statement, int pid) {
		EmpEagerAbsState origState = (EmpEagerAbsState) s;
		List<Action> succ = new LinkedList<Action>();		
		
		EmpEagerAbsState newState = (EmpEagerAbsState) factory.newState(origState);
		if(!statement.isLast())			
			newState.advancePC(pid);
		
		for (int res : origState.shared.keySet())
		{
			SharedResVal val = (SharedResVal) newState.shared.get(res);
			val.clearBuffer(pid);			
		}
		
		succ.add(new Action(origState, newState, statement, pid, "" + pid + 
			"." + statement.getLabel() + " " + statement.toString()));
		return succ;
	}

	protected List<Action> applyEndAtomic(SBState state, EndAtomic statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		EmpEagerAbsState newstate = (EmpEagerAbsState) factory.newState(state);
		
		for (int res : newstate.shared.keySet())
		{
			SharedResVal sharedval = newstate.shared.get(res);
			if (!sharedval.isEmpty(pid))
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
		EmpEagerAbsState state = (EmpEagerAbsState) s;
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
					return (sharedval.isEmpty(pid));
				}
			case SWAP:
				{
					Swap swap = (Swap) statement;
					sharedStr = swap.getShared(state, pid);
					SharedResVal sharedval;
					sharedval = state.shared.get(sharedStr);
					return (sharedval.isEmpty(pid));
				}
			case ATOMICDEC:
				{
					AtomicDec dec = (AtomicDec) statement;
					sharedStr = dec.getShared(state, pid);
					SharedResVal sharedval;
					sharedval = state.shared.get(sharedStr);
					return (sharedval.isEmpty(pid));
				}					
			case BARRIER:
				{
					for (Entry<Integer, SharedResVal> entry : state.shared.entrySet())
					{
						SharedResVal sharedval = entry.getValue();
						if (!sharedval.isEmpty(pid))
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
