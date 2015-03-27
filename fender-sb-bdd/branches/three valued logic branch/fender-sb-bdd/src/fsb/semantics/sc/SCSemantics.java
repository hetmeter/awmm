package fsb.semantics.sc;

import java.util.LinkedList;
import java.util.List;

import fsb.ast.Allocate;
import fsb.ast.AritExpr;
import fsb.ast.Assign;
import fsb.ast.AtomicDec;
import fsb.ast.Branch;
import fsb.ast.CAS;
import fsb.ast.ChooseAssign;
import fsb.ast.ChooseStore;
import fsb.ast.ConstExpr;
import fsb.ast.DerefSharedVal;
import fsb.ast.EndAtomic;
import fsb.ast.Load;
import fsb.ast.NondetArit;
import fsb.ast.Statement;
import fsb.ast.Store;
import fsb.ast.Swap;
import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.explore.Action;
import fsb.explore.BadMemoryException;
import fsb.explore.SBState;
import fsb.explore.StateFactory;
import fsb.semantics.Semantics;

public class SCSemantics extends Semantics {
	//TODO: Implement LAZY_COPY

	private StateFactory factory;

	public SCSemantics()
	{
		this.factory = new SCStateFactory();
	}	

	protected List<Action> applyAllocate(SBState state, Allocate allocate, int pid) {	
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
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
		SCState newstate = (SCState) factory.newState(state);
		
		ArithValue val = assign.getSrcVal(state, pid);
		String dst = assign.getDst();
		newstate.setLocal(pid, dst, val);
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, assign, pid, "" + pid + 
				"." + assign.getLabel() + " " + assign.toString()));
		return succ;
	}
	
	protected List<Action> applyChooseAssign(SBState state, ChooseAssign choose, int pid) {	
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
		AritExpr src = null;
		if (choose.cond1.evaluate(state, pid).isTrue() )
		{
			src = new ConstExpr(1);
		
		}else if (choose.cond2.evaluate(state, pid).isTrue() )
		{		
			src = new ConstExpr(0);
		
		}else{
		
			src = new NondetArit();
		}

		Assign assign = new Assign(src, choose.dst);
		ArithValue val = assign.getSrcVal(state, pid);
		String dst = assign.getDst();
		newstate.setLocal(pid, dst, val);
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, choose, pid, "" + pid + 
				"." + assign.getLabel() + " " + choose.toString()));
		return succ;
	}
	
	protected List<Action> applyLoad(SBState s, Load load, int pid) {	
		SCState state = (SCState) s;
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
		String src = load.getSrc(state, pid);
		String dst = load.getDst();
		ArithValue val;
		try
		{
			val = state.getShared(src);
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(src);
		}		
		newstate.setLocal(pid, dst, val);
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, load, pid, "" + pid + 
				"." + load.getLabel() + " " + load.toString()));
		return succ;
	}
	
	protected List<Action> applyStore(SBState s, Store store, int pid) {	
		SCState state = (SCState) s;
		List<Action> succ = new LinkedList<Action>();			
		ArithValue val = store.getSrcVal(state, pid);

		//for (Integer val : vals) {
			SCState newstate = (SCState) factory.newState(state);
			
			String dst = store.getDst(state, pid);
			newstate.setShared(dst, val);
			
			newstate.advancePC(pid);	
			succ.add(new Action(state, newstate, store, pid, "" + pid + 
					"." + store.getLabel() + " " + store.toString()));
//		}

		return succ;
	}
	
	protected List<Action> applyChooseStore(SBState s, ChooseStore choose, int pid) {	
		
		AritExpr src = null;
		if (choose.cond1.evaluate(s, pid).isTrue() )
		{
			src = new ConstExpr(1);
		
		}else if (choose.cond2.evaluate(s, pid).isTrue() )
		{		
			src = new ConstExpr(0);
		
		}else{
		
			src = new NondetArit();
		}

		Store store = new Store(src, choose.dst);
		SCState state = (SCState) s;
		List<Action> succ = new LinkedList<Action>();			
		ArithValue val = store.getSrcVal(state, pid);

		//for (Integer val : vals) {
			SCState newstate = (SCState) factory.newState(state);
			
			String dst = store.getDst(state, pid);
			newstate.setShared(dst, val);
			
			newstate.advancePC(pid);	
			succ.add(new Action(state, newstate, choose, pid, "" + pid + 
					"." + choose.getLabel() + " " + choose.toString()));
		//}
		return succ;
	}
	
	protected List<Action> applyDirectStore(SBState s, Store store, int pid) {
		return applyStore(s, store, pid);
	}
	
	protected List<Action> applyCAS(SBState s, CAS cas, int pid) {	
		SCState state = (SCState) s;
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
		ArithValue currval;
		String sharedStr = cas.getShared(state, pid);
		try
		{
			currval = newstate.getShared(sharedStr);
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}
		
		if (currval == cas.getOldVal(state, pid))
		{
			newstate.setShared(sharedStr, cas.getNewVal(state, pid));
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
		SCState state = (SCState) s;
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
		ArithValue currval;
		String sharedStr = swap.getShared(newstate, pid);
		try
		{
			currval = newstate.getShared(sharedStr);
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}
		newstate.setShared(sharedStr, swap.getNewVal(state, pid));
		newstate.setLocal(pid, swap.getRes(), currval);
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, swap, pid, "" + pid + 
				"." + swap.getLabel() + " " + swap.toString()));

		return succ;
	}
	
	protected List<Action> applyAtomicDec(SBState s, AtomicDec dec, int pid) {	
		SCState state = (SCState) s;
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
		ArithValue currval;
		String sharedStr = dec.getShared(state, pid);
		try
		{
			currval = newstate.getShared(sharedStr);
		} catch (NullPointerException e)
		{
			throw new BadMemoryException(sharedStr);
		}
		newstate.setShared(sharedStr, currval.sub(DeterArithValue.getInstance(1)));
		newstate.setLocal(pid, dec.getRes(), currval);
		
		newstate.advancePC(pid);	
		succ.add(new Action(state, newstate, dec, pid, "" + pid + 
				"." + dec.getLabel() + " " + dec.toString()));

		return succ;
	}

	protected List<Action> applyBarrier(SBState state, Statement statement, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
		if(!statement.isLast())			
			newstate.advancePC(pid);
		
		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
				"." + statement.getLabel() + " " + statement.toString()));	
		
		return succ;
	}

	protected List<Action> applyEndAtomic(SBState state, EndAtomic statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		SCState newstate = (SCState) factory.newState(state);
		
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
	
	protected List<Action> getFlushes(SBState s, int pid)
	{
		List<Action> succ = new LinkedList<Action>();
		return succ;
	}
	
	@Override
	public StateFactory getStateFactory() {
		return factory;
	}
}
