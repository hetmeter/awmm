package fsb.semantics.pso;

import java.util.LinkedList;
import java.util.List;

import fsb.ast.Allocate;
import fsb.ast.Assign;
import fsb.ast.AtomicDec;
import fsb.ast.Branch;
import fsb.ast.CAS;
import fsb.ast.EndAtomic;
import fsb.ast.Load;
import fsb.ast.Statement;
import fsb.ast.Store;
import fsb.ast.Swap;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.Action;
import fsb.explore.BadMemoryException;
import fsb.explore.SBState;
import fsb.explore.StateFactory;
import fsb.semantics.Semantics;

public class PSOSemantics extends Semantics {
	//TODO: Implement LAZY_COPY

	private StateFactory factory;
	private int limit;

	public PSOSemantics(int limit)
	{
		this.factory = new PSOStateFactory();
		this.limit = limit;
	}
	
	protected List<Action> getFlushes(SBState s, int pid)
	{
		PSOState state = (PSOState) s;
		List<Action> succ = new LinkedList<Action>();
		for (String res : state.shared.keySet())
		{
			SharedResVal sharedval = state.shared.get(res);
			if (!sharedval.buffer[pid].isEmpty())
			{
				PSOState newstate = (PSOState) factory.newState(s);
				SharedResVal newsharedval = newstate.shared.get(res);
				succ.add(new Action(s, newstate, null, pid, "" + pid + " FLUSH " + res));
				newsharedval.value = newsharedval.buffer[pid].removeLast().contents;
			}
		}	
		return succ;
	}

	protected List<Action> applyAllocate(SBState state, Allocate allocate, int pid) {	
		List<Action> succ = new LinkedList<Action>();			
		PSOState newstate = (PSOState) factory.newState(state);
		
		int num = ((DeterArithValue)allocate.getNumVal(state, pid)).getValue();
		int mem = newstate.allocate(num);
		String dst = allocate.getDst();
		newstate.setLocal(pid, dst, DeterArithValue.getInstance(mem));
		
		newstate.advancePC(pid);		
		succ.add(new Action(state, newstate, allocate, pid, "" + pid + 
				"." + allocate.getLabel() + " " + allocate.toString()));
		return succ;
	}
	
	protected List<Action> applyAssign(SBState state, Assign assign, int pid) {	
		List<Action> succ = new LinkedList<Action>();			
		PSOState newstate = (PSOState) factory.newState(state);
		
		int val = ((DeterArithValue)assign.getSrcVal(state, pid)).getValue();
		String dst = assign.getDst();
		newstate.setLocal(pid, dst, DeterArithValue.getInstance(val));
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, assign, pid, "" + pid + 
				"." + assign.getLabel() + " " + assign.toString()));
		return succ;
	}
	
	protected List<Action> applyLoad(SBState s, Load load, int pid) {	
		PSOState state = (PSOState) s;
		List<Action> succ = new LinkedList<Action>();			
		PSOState newstate = (PSOState) factory.newState(state);
		
		String src = load.getSrc(state, pid);
		String dst = load.getDst();
		int val;
		try
		{
			val = state.shared.get(src).getVal(pid);
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(src);
		}		
		newstate.setLocal(pid, dst, DeterArithValue.getInstance(val));
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, load, pid, "" + pid + 
				"." + load.getLabel() + " " + load.toString()));
		return succ;
	}
	
	protected List<Action> applyStore(SBState s, Store store, int pid) {	
		PSOState state = (PSOState) s;
		List<Action> succ = new LinkedList<Action>();
		//List<Integer> vals = store.getSrcVal(state, pid);
		int val = ((DeterArithValue)store.getSrcVal(state, pid)).getValue();
		//for (Integer val : vals) {
			PSOState newstate = (PSOState) factory.newState(state);
			
			String dst = store.getDst(state, pid);
			
			SharedResVal shared = newstate.shared.get(dst);
			if (shared.buffer[pid].size() >= limit)
				return succ;
			
			shared.writeVal(pid, store.getLabel(), val);
			
			newstate.advancePC(pid);	
			succ.add(new Action(state, newstate, store, pid, "" + pid + 
					"." + store.getLabel() + " " + store.toString()));			
//		}

		return succ;
	}
	
	protected List<Action> applyDirectStore(SBState s, Store store, int pid) {
		PSOState state = (PSOState) s;
		List<Action> succ = new LinkedList<Action>();
	//	List<Integer> vals = store.getSrcVal(state, pid);
		int val = ((DeterArithValue)store.getSrcVal(state, pid)).getValue();
		//for (Integer val : vals) {			
			PSOState newstate = (PSOState) factory.newState(state);			
			
			String dst = store.getDst(state, pid);		
			SharedResVal sharedval = newstate.shared.get(dst);
			sharedval.value = val;
			sharedval.buffer[pid].clear();
			
			newstate.advancePC(pid);	
			succ.add(new Action(state, newstate, store, pid, "" + pid + 
					"." + store.getLabel() + " " + store.toString()));	
			
			succ.add(new Action(state, newstate, store, pid, "" + pid + 
					"." + store.getLabel() + " " + store.toString()));
	//	}
		
		return succ;
	}

	protected List<Action> applyCAS(SBState s, CAS cas, int pid) {	
		PSOState state = (PSOState) s;
		List<Action> succ = new LinkedList<Action>();			
		PSOState newstate = (PSOState) factory.newState(state);
		
		int currval;
		String sharedStr = cas.getShared(newstate, pid);
		SharedResVal sharedval;
		try
		{
			sharedval = newstate.shared.get(sharedStr);
			if (!sharedval.buffer[pid].isEmpty())
			{						
				sharedval.value = sharedval.buffer[pid].removeFirst().contents;
				sharedval.buffer[pid].clear();
			}			
			currval = sharedval.value;
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}
		
		if (currval == ((DeterArithValue)cas.getOldVal(state, pid)).getValue())
		{
			sharedval.value = ((DeterArithValue)cas.getNewVal(state, pid)).getValue();
			newstate.setLocal(pid, cas.getRes(), DeterArithValue.getInstance(1));
		}
		else
		{
			newstate.setLocal(pid, cas.getRes(), DeterArithValue.getInstance(0));
		}
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, cas, pid, "" + pid + 
				"." + cas.getLabel() + " " + cas.toString()));

		return succ;
	}
	
	protected List<Action> applySwap(SBState s, Swap swap, int pid) {	
		PSOState state = (PSOState) s;
		List<Action> succ = new LinkedList<Action>();			
		PSOState newstate = (PSOState) factory.newState(state);
		
		int currval;
		String sharedStr = swap.getShared(newstate, pid);
		SharedResVal sharedval;
		try
		{
			sharedval = newstate.shared.get(sharedStr);
			if (!sharedval.buffer[pid].isEmpty())
			{						
				sharedval.value = sharedval.buffer[pid].removeFirst().contents;
				sharedval.buffer[pid].clear();
			}			
			currval = sharedval.value;
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}
		
		sharedval.value = ((DeterArithValue)swap.getNewVal(state, pid)).getValue();
		newstate.setLocal(pid, swap.getRes(), DeterArithValue.getInstance(currval));
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, swap, pid, "" + pid + 
				"." + swap.getLabel() + " " + swap.toString()));

		return succ;
	}
	
	protected List<Action> applyAtomicDec(SBState s, AtomicDec dec, int pid) {	
		PSOState state = (PSOState) s;
		List<Action> succ = new LinkedList<Action>();			
		PSOState newstate = (PSOState) factory.newState(state);
		
		int currval;
		String sharedStr = dec.getShared(newstate, pid);
		SharedResVal sharedval;
		try
		{
			sharedval = newstate.shared.get(sharedStr);
			if (!sharedval.buffer[pid].isEmpty())
			{						
				sharedval.value = sharedval.buffer[pid].removeFirst().contents;
				sharedval.buffer[pid].clear();
			}			
			currval = sharedval.value;
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}
		
		sharedval.value = currval - 1;
		newstate.setLocal(pid, dec.getRes(), DeterArithValue.getInstance(currval));
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, dec, pid, "" + pid + 
				"." + dec.getLabel() + " " + dec.toString()));

		return succ;
	}
	
	protected List<Action> applyBarrier(SBState state, Statement statement, int pid) {
		List<Action> succ = new LinkedList<Action>();				
		PSOState newstate = (PSOState) factory.newState(state);		
		
		for (String res : newstate.shared.keySet())
		{
			SharedResVal sharedval = newstate.shared.get(res);
			if (!sharedval.buffer[pid].isEmpty())
			{						
				sharedval.value = sharedval.buffer[pid].removeFirst().contents;
				sharedval.buffer[pid].clear();
			}
		}
		
		if(!statement.isLast())			
			newstate.advancePC(pid);
		
		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
			"." + statement.getLabel() + " " + statement.toString()));
		return succ;
	}
	
	protected List<Action> applyEndAtomic(SBState state, EndAtomic statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		PSOState newstate = (PSOState) factory.newState(state);
		
		for (String res : newstate.shared.keySet())
		{
			SharedResVal sharedval = newstate.shared.get(res);
			if (!sharedval.buffer[pid].isEmpty())
			{						
				sharedval.value = sharedval.buffer[pid].removeFirst().contents;
				sharedval.buffer[pid].clear();
			}
		}		
		newstate.unsetAtomic();
		
		newstate.advancePC(pid);
		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
				"." + statement.getLabel() + " " + statement.toString()));
		return succ;
	}

	public boolean isEnabled(SBState s, int pid, Statement statement) 
	{
		return true;
	}
	
	@Override
	public StateFactory getStateFactory() {
		return factory;
	}
	
}
