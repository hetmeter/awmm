package fsb.semantics;

import java.util.LinkedList;
import java.util.List;

import fsb.ast.Allocate;
import fsb.ast.AritExpr;
import fsb.ast.Assign;
import fsb.ast.Assume;
import fsb.ast.AtomicDec;
import fsb.ast.BeginAtomic;
import fsb.ast.Branch;
import fsb.ast.CAS;
import fsb.ast.ChooseAssign;
import fsb.ast.ChooseStore;
import fsb.ast.Comment;
import fsb.ast.ConstExpr;
import fsb.ast.EndAtomic;
import fsb.ast.Load;
import fsb.ast.NondetArit;
import fsb.ast.NondetBool;
import fsb.ast.Nop;
import fsb.ast.Statement;
import fsb.ast.Store;
import fsb.ast.Swap;
import fsb.ast.tvl.BoolValue;
import fsb.ast.tvl.DeterArithValue;
import fsb.ast.tvl.DeterBoolValue;
import fsb.ast.tvl.NonDeterBoolValue;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.State;
import fsb.explore.StateFactory;

public abstract class Semantics {
	public final List<Action> getSuccessors(State s)
	{
		SBState absState = (SBState) s;		
		
		List<Action> successors = new LinkedList<Action>();
		
		//If the state failed an assume() directive, forget it.
		if (s.assumeFailed())
			return successors;
		
		//We are still in the init stage
		if (!absState.isInitFinished())
		{
			successors.addAll(getSucc(absState, 0));
			successors.addAll(getFlushes(absState, 0));
		}
		
		//Is any process in an atomic section?
		else if (s.inAtomicSection() == -1)
		{
			//No. Find, for each processor, whether there is a local action enabled.
			//If there is one, just apply it. (Partial Order Reduction for the poor)
			for (int pid = 0; pid < s.getProcs(); pid++)
			{
				Statement st = absState.program.getListing(pid).get(absState.getPC(pid));
				if (st.isLocal())
				{
					return apply(absState, st, pid);
				}
			}

			//Ok, so all threads have non-local actions enabled.
			//Collect them all.
			for (int pid = 0; pid < s.getProcs(); pid++)
			{
				successors.addAll(getSucc(absState, pid));
				successors.addAll(getFlushes(absState, pid));
			}
		}
		else
		{
			//Yes, only this process may move.
			successors.addAll(getSucc(absState, s.inAtomicSection()));
			//No need to flush - these flushes can not become visible until after
			//the end of the atomic section anyway.
			//successors.addAll(getFlushes(absState, s.inAtomicSection()));
		}
				
		return successors;		
	}
	
	//Just a wrapper - we go from (state, pid) to the list of successor states.
	//The real work is in apply()
	protected final List<Action> getSucc(SBState s, int pid)
	{
		Statement st = s.program.getListing(pid).get(s.getPC(pid));
		if (isEnabled(s, pid, st))
			return apply(s, st, pid);
		else
			return new LinkedList<Action>();
	}
		
	//Delegate actually applying the statement to the specific semantics.
	protected final List<Action> apply(SBState s, Statement statement, int pid) {
		List <Action> ret = null;
		switch (statement.getType())
		{
			case ASSIGN:{
				ret = applyAssign(s, (Assign)statement, pid);
				break;
			}
			case ALLOCATE:{
				ret = applyAllocate(s, (Allocate)statement, pid);
				break;
			}
			case LOAD:{
				ret = applyLoad(s, (Load)statement, pid);
				break;
			}
			case STORE:{
				if ((pid != 0) && (s.inAtomicSection() == -1))
					ret = applyStore(s, (Store)statement, pid);
				else
					//TODO: What if there is already stuff in the buffer?
					//This is probably a bad idea, but it's an optimization...
					ret = applyDirectStore(s, (Store)statement, pid);
				break;
			}
			case BRANCH:{
				ret = applyBranch(s, (Branch)statement, pid);
				break;
				}
			case BARRIER:{
				//Fences have no effect inside in an atomic section.
				if (s.inAtomicSection() == -1)
					ret = applyBarrier(s, statement, pid);
				else
					ret = applyNop(s, statement, pid);
				break;
			}
			case END:{
				//May not reach end of execution while in atomic section.
				//That would be a bug in the input program!
				if (s.inAtomicSection() == -1)
					ret = applyBarrier(s, statement, pid);
				else
					throw new RuntimeException("Reached end of program for process " + pid + " while in atomic section!");
				break;
			}
			case CAS:{
				ret = applyCAS(s, (CAS)statement, pid);
				break;
			}
			case SWAP:{
				ret = applySwap(s, (Swap)statement, pid);
				break;
			}
			case ATOMICDEC:{
				ret =applyAtomicDec(s, (AtomicDec)statement, pid);
				break;
			
			}
			case NOP:{
				ret = applyNop(s, (Nop)statement, pid);
				break;
			}
			case BEGINATOMIC:{
				ret = applyBeginAtomic(s, (BeginAtomic)statement, pid);
				break;
			}
			case ENDATOMIC:{
				ret = applyEndAtomic(s, (EndAtomic)statement, pid);
				break;
			}
			case ASSUME:{  
				ret = applyAssume(s, (Assume)statement, pid);
				break;
			}
			case CHOOSE_ASSIGN:{
				ret = applyChooseAssign(s, (ChooseAssign)statement, pid);
				break;
			}
			case CHOOSE_STORE:{
				ret = applyChooseStore(s, (ChooseStore)statement, pid);
				break;
			}
			case C_COMMENT:{
				return ret = applyComment(s, (Comment)statement, pid);
			}
			default:
				throw new RuntimeException("Unsupported statement type!");
		}
		
		// YURI: the following is mainly for SC
		// maybe others will find use of it though I doubt it
		for(Action a : ret){
			a.getTarget().registerAction(a);
		}
		return ret;
	}
	


	protected abstract List<Action> applyAssign(SBState s, Assign statement, int pid);
	protected abstract List<Action> applyAllocate(SBState s, Allocate statement, int pid);
	protected abstract List<Action> applyLoad(SBState s, Load statement, int pid);
	protected abstract List<Action> applyStore(SBState s, Store statement, int pid);
	protected abstract List<Action> applyCAS(SBState s, CAS statement, int pid);
	protected abstract List<Action> applySwap(SBState s, Swap statement, int pid);
	protected abstract List<Action> applyAtomicDec(SBState s, AtomicDec statement, int pid);
	protected abstract List<Action> applyBarrier(SBState s, Statement statement, int pid);
	protected abstract List<Action> applyDirectStore(SBState s, Store statement, int pid);

	protected List<Action> applyChooseAssign(SBState state, ChooseAssign choose, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		
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
		
		Assign s = new Assign(src, choose.dst);
		
		return applyAssign(state, s, pid);
	}
	
	protected List<Action> applyChooseStore(SBState state, ChooseStore choose,
			int pid) {
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
		

		Store store = new Store(src, choose.dst);
		return applyStore(state, store, pid);
	}
	
	protected List<Action> applyBranch(SBState state, Branch branch, int pid) {
		List<Action> succ = new LinkedList<Action>();			

		//The only effect of a BRANCH is to change the PC according to whether
		//cond is true or false.
		//If cond is non-deterministic, create both successors.
		
		BoolValue branch_cond = branch.cond.evaluate(state, pid);
		if (branch_cond instanceof NonDeterBoolValue)
		{
			SBState falsestate = (SBState) getStateFactory().newState(state);
			SBState truestate = (SBState) getStateFactory().newState(state);
			
			falsestate.advancePC(pid);
			truestate.setPC(pid, branch.toLabel);
			
			succ.add(new Action(state, falsestate, branch, pid, "" + pid + 
					"." + branch.getLabel() + " " + branch.toString()));
			
			succ.add(new Action(state, truestate, branch, pid, "" + pid + 
					"." + branch.getLabel() + " " + branch.toString()));
		}
		else//branch_cond deterministic
		{		
			SBState newstate = (SBState) getStateFactory().newState(state);
			if (branch_cond.isTrue())
				newstate.setPC(pid, branch.toLabel);
			else//branch_cond is false
				newstate.advancePC(pid);
			
			succ.add(new Action(state, newstate, branch, pid, "" + pid + 
					"." + branch.getLabel() + " " + branch.toString()));
		}
		
		return succ;
	}
	
	protected List<Action> applyAssume(SBState state, Assume assume, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		SBState newstate = (SBState) getStateFactory().newState(state);
		
		BoolValue assume_cond = assume.cond.evaluate(state, pid);
		boolean assumePassed = assume_cond.isTrue();
		if (!assumePassed)
			newstate.setAssumeFailed();
		
		newstate.advancePC(pid);
		
		succ.add(new Action(state, newstate, assume, pid, "" + pid + 
				"." + assume.getLabel() + " " + assume.toString() + " Result: " + assumePassed));
		return succ;
	}
	
	protected final List<Action> applyNop(SBState state, Statement statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		SBState newstate = (SBState) getStateFactory().newState(state);
		
		//NOP is not a complete nop - advance the PC...
		newstate.advancePC(pid);
		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
				"." + statement.getLabel() + " " + statement.toString()));	
		
		return succ;		
	}

	protected final List<Action> applyComment(SBState state, Statement statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		SBState newstate = (SBState) getStateFactory().newState(state);
		
		//NOP is not a complete nop - advance the PC...
		newstate.advancePC(pid);
		succ.add(new Action(state, newstate, statement, pid, statement.toString()));	
		
		return succ;		
	}
	
	//Begin and End atomic are special - the only set a flag on the state, and 
	//are not affected by the specific semantics.
	protected List<Action> applyBeginAtomic(SBState state, BeginAtomic statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		SBState newstate = (SBState) getStateFactory().newState(state);
	
		newstate.setAtomic(pid);
		
		newstate.advancePC(pid);		
		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
				"." + statement.getLabel() + " " + statement.toString()));	
		
		return succ;		
	}
	
	protected abstract List<Action> applyEndAtomic(SBState state, EndAtomic statement, int pid);
	
	//getSucc() handles "normal" successors. These two functions handle the two other cases:
	//getFlushes() returns the successors that are the result of a flush.
	protected abstract List<Action> getFlushes(SBState s, int pid);
	
	//Checks whether a statement is enabled for a given pid. Mostly used for fence-like operations
	//(e.g. if the semantic rule is that a fence is enabled only if some buffer is empty)
	public abstract boolean isEnabled(SBState state, int pid, Statement statement);
	
	//Return the semantics-specific factory used to generate appropriate states.
	public abstract StateFactory getStateFactory();
}
