package fsb.semantics;

import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import fsb.ast.AritExpr;
import fsb.ast.AssertBoolExpr;
import fsb.ast.AssertComplexBool;
import fsb.ast.Assign;
import fsb.ast.Assume;
import fsb.ast.BeginAtomic;
import fsb.ast.BoolExpr;
import fsb.ast.Branch;
import fsb.ast.CAS;
import fsb.ast.ChooseAssign;
import fsb.ast.ChooseStore;
import fsb.ast.Comment;
import fsb.ast.ComplexBool;
import fsb.ast.ConstExpr;
import fsb.ast.EndAtomic;
import fsb.ast.Load;
import fsb.ast.NondetArit;
import fsb.ast.Nop;
import fsb.ast.Statement;
import fsb.ast.Store;
import fsb.ast.Swap;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.State;
import fsb.explore.StateFactory;
import fsb.tvl.BoolValue;
import fsb.utils.Debug;
import fsb.utils.Options;
import gnu.trove.map.hash.THashMap;

public abstract class Semantics {
	public static THashMap<BoolExpr, Set<BoolExpr>> cubesTrueTogetherMap =
			new THashMap<BoolExpr, Set<BoolExpr>>();
	public static THashMap<BoolExpr, Set<BoolExpr>> cubesFalseTogetherMap = 
			new THashMap<BoolExpr, Set<BoolExpr>>();
	
	public static THashMap<AssertBoolExpr, Set<AssertBoolExpr>> cubesAssertTrueTogetherMap =
			new THashMap<AssertBoolExpr, Set<AssertBoolExpr>>();
	public static THashMap<AssertBoolExpr, Set<AssertBoolExpr>> cubesAssertFalseTogetherMap = 
			new THashMap<AssertBoolExpr, Set<AssertBoolExpr>>();

	public static void addCubesToSet(Set<BoolExpr> cubes_to_add,
			Set<BoolExpr> all_cubes,
			THashMap<BoolExpr, Set<BoolExpr>> target
			){

		Set<BoolExpr> rest_cubes = new HashSet<BoolExpr>();
		rest_cubes.addAll(all_cubes);
		rest_cubes.remove(cubes_to_add);
		
		for(BoolExpr c : all_cubes){
			Set<BoolExpr> entry = target.get(c);
			if(null == entry){
				entry = new HashSet<BoolExpr>();
			}
			if(cubes_to_add.contains(c)){
				entry.addAll(cubes_to_add);
				entry.remove(rest_cubes);
			}else{
				entry.remove(cubes_to_add);
			}
			target.put(c, entry);
		}
	}
	
	public static void addAssertCubesToSet(Set<AssertBoolExpr> cubes_to_add,
			Set<AssertBoolExpr> all_cubes,
			THashMap<AssertBoolExpr, Set<AssertBoolExpr>> target
			){

		Set<AssertBoolExpr> rest_cubes = new HashSet<AssertBoolExpr>();
		rest_cubes.addAll(all_cubes);
		rest_cubes.removeAll(cubes_to_add);
		
		for(AssertBoolExpr c : all_cubes){
			Set<AssertBoolExpr> entry = target.get(c);
			if(null == entry){
				entry = new HashSet<AssertBoolExpr>();
			}
			if(cubes_to_add.contains(c)){
				entry.addAll(cubes_to_add);
				entry.remove(rest_cubes);
			}else{
				entry.remove(cubes_to_add);
			}
			target.put(c, entry);
		}
	}
	
	public static void printTrueFalseList(){
		System.out.println("printing the used cubes statistics per cube");
		for(BoolExpr b : cubesTrueTogetherMap.keySet()){
			System.out.println();
			System.out.println("each time " + b + " was true, so were " + cubesTrueTogetherMap.get(b));
		}
		for(AssertBoolExpr b : cubesAssertTrueTogetherMap.keySet()){
			System.out.println();
			System.out.println("each time " + b + " was true, so were " + cubesAssertTrueTogetherMap.get(b));
		}
		for(BoolExpr b : cubesFalseTogetherMap.keySet()){
			System.out.println();
			System.out.println("each time " + b + " was false, so were " + cubesFalseTogetherMap.get(b));
		}
		for(AssertBoolExpr b : cubesAssertFalseTogetherMap.keySet()){
			System.out.println();
			System.out.println("each time " + b + " was false, so were " + cubesAssertFalseTogetherMap.get(b));
		}
	}
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
				//return ret;
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
				//return ret;
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
	protected abstract List<Action> applyLoad(SBState s, Load statement, int pid);
	protected abstract List<Action> applyStore(SBState s, Store statement, int pid);
	protected abstract List<Action> applyCAS(SBState s, CAS statement, int pid);
	protected abstract List<Action> applySwap(SBState s, Swap statement, int pid);
	protected abstract List<Action> applyBarrier(SBState s, Statement statement, int pid);
	protected abstract List<Action> applyDirectStore(SBState s, Store statement, int pid);

	protected List<Action> applyChooseAssign(SBState state, ChooseAssign choose, int pid) {
		List<Action> succ = new LinkedList<Action>();			
//		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
//			ComplexBool.m_allCubesLocal.clear();
//			ComplexBool.m_usefullCubesLocal.clear();
//		}	
		AritExpr src =	chooseConditionEvaluation(state, choose.cond1,choose.cond2, pid);

		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
			choose.m_allCubes.addAll(choose.cond1.allCubes());
			choose.m_allCubes.addAll(choose.cond2.allCubes());
			if(choose.cond1.evaluate(state,pid).isTrue()){
				choose.m_usefullCubes.addAll(choose.cond1.allSatisfiedCubes(state,pid));
			}else{
				if(choose.cond1.evaluate(state,pid).isFalse()){
					choose.m_usefullCubes.addAll(choose.cond1.allNegationSatisfiedCubes(state,pid));
				}
				if(choose.cond2.evaluate(state,pid).isTrue()){
					choose.m_usefullCubes.addAll(choose.cond2.allSatisfiedCubes(state,pid));
				}
				if(choose.cond2.evaluate(state,pid).isFalse()){
					choose.m_usefullCubes.addAll(choose.cond2.allNegationSatisfiedCubes(state,pid));
				}
			}
		}
		
		Assign s = new Assign(src, choose.dst);
		return applyAssign(state, s, pid);
	}

	/**
	 * @param state
	 * @param choose
	 * @param pid
	 * @return
	 */
	protected AritExpr chooseConditionEvaluation(SBState state,
			BoolExpr cond1, BoolExpr cond2, int pid) {
		AritExpr src = null;
		BoolValue cond1Evaluation = cond1.evaluate(state, pid);
		if (cond1Evaluation.isTrue() )
		{
			if(Options.PRINT_USED_CUBES){
				addCubesToSet(cond1.allSatisfiedCubes(state, pid),
					cond1.allCubes(),cubesTrueTogetherMap);
				ComplexBool.m_usefullCubesGlobal.addAll(cond1.allSatisfiedCubes(state, pid));
				ComplexBool.m_allCubesGlobal.addAll(cond1.allCubes());
			}
			src = new ConstExpr(1);
		
		}else {
			if (Options.PRINT_USED_CUBES && cond1Evaluation.isFalse() ){
				//if undetermined condition not interesting
				addCubesToSet(cond1.allNegationSatisfiedCubes(state, pid),
						cond1.allCubes(),cubesFalseTogetherMap);
				ComplexBool.m_usefullCubesGlobal.addAll(cond1.allNegationSatisfiedCubes(state, pid));
				ComplexBool.m_allCubesGlobal.addAll(cond1.allCubes());
			}
			BoolValue con2Evaluation = cond2.evaluate(state, pid);
			if (con2Evaluation.isTrue() )
			{		
				if(Options.PRINT_USED_CUBES){
				addCubesToSet(cond2.allSatisfiedCubes(state, pid),
						cond2.allCubes(),cubesTrueTogetherMap);
				ComplexBool.m_usefullCubesGlobal.addAll(cond2.allSatisfiedCubes(state, pid));
				ComplexBool.m_allCubesGlobal.addAll(cond2.allCubes());
				}
				src = new ConstExpr(0);
		
			}else{
				if (Options.PRINT_USED_CUBES && con2Evaluation.isFalse() ){
					addCubesToSet(cond2.allNegationSatisfiedCubes(state, pid),
							cond2.allCubes(),cubesFalseTogetherMap);
					ComplexBool.m_usefullCubesGlobal.addAll(cond2.allNegationSatisfiedCubes(state, pid));
					ComplexBool.m_allCubesGlobal.addAll(cond2.allCubes());
				}
			src = new NondetArit();
		
			}
		}
		return src;
	}
	
	protected List<Action> applyChooseStore(SBState state, ChooseStore choose,
			int pid) {

		AritExpr src = 	chooseConditionEvaluation(state, choose.cond1,choose.cond2, pid);
		
		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
			choose.m_allCubes.addAll(choose.cond1.allCubes());
			choose.m_allCubes.addAll(choose.cond2.allCubes());
			BoolValue cond1Evaluation = choose.cond1.evaluate(state,pid);
			if(cond1Evaluation.isTrue()){
				choose.m_usefullCubes.addAll(choose.cond1.allSatisfiedCubes(state,pid));
			}else{
				if(cond1Evaluation.isFalse()){
					choose.m_usefullCubes.addAll(choose.cond1.allNegationSatisfiedCubes(state,pid));
				}
				BoolValue cond2Evaluation = choose.cond2.evaluate(state,pid);
				if(cond2Evaluation.isTrue()){
					choose.m_usefullCubes.addAll(choose.cond2.allSatisfiedCubes(state,pid));
				}
				if(cond2Evaluation.isFalse()){
					choose.m_usefullCubes.addAll(choose.cond2.allNegationSatisfiedCubes(state,pid));
				}
			}
		}
		
		Store store = new Store(src, choose.dst);
		return applyStore(state, store, pid);
	}
	
	protected List<Action> applyBranch(SBState state, Branch branch, int pid) {
		List<Action> succ = new LinkedList<Action>();			

		//The only effect of a BRANCH is to change the PC according to whether
		//cond is true or false.
		//If cond is non-deterministic, create both successors.
//		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
//			ComplexBool.m_allCubesLocal.clear();
//			ComplexBool.m_usefullCubesLocal.clear();
//		}
		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
			ComplexBool.m_allCubesGlobal.addAll(branch.cond.allCubes());
		}
		
		BoolValue condEvaluation = branch.cond.evaluate(state, pid);
		if (!condEvaluation.isDetermined())
		{
			
			SBState falsestate = (SBState) getStateFactory().newState(state);
			SBState truestate = (SBState) getStateFactory().newState(state);
			
			falsestate.advancePC(pid);
			truestate.setPC(pid, branch.toLabel);
			
			succ.add(new Action(state, falsestate, branch, pid));
//			succ.add(new Action(state, falsestate, branch, pid, "" + pid + 
//					"." + branch.getLabel() + " " + branch.toString()));
			
			succ.add(new Action(state, truestate, branch, pid));
//			succ.add(new Action(state, truestate, branch, pid, "" + pid + 
//					"." + branch.getLabel() + " " + branch.toString()));

		}
		else
		{		
			
			SBState newstate = (SBState) getStateFactory().newState(state);
					
			if (condEvaluation.isTrue() ){
				if(Options.PRINT_USED_CUBES){
					addCubesToSet(branch.cond.allSatisfiedCubes(state, pid),
							branch.cond.allCubes(),cubesTrueTogetherMap);
					ComplexBool.m_usefullCubesGlobal.addAll(branch.cond.allSatisfiedCubes(state, pid));
				}
				newstate.setPC(pid, branch.toLabel);
			}else{// it is false since if it was nondetermined it would of fallen on the first if
				if(Options.PRINT_USED_CUBES){
					addCubesToSet(branch.cond.allNegationSatisfiedCubes(state, pid),
						branch.cond.allCubes(),cubesFalseTogetherMap);
					ComplexBool.m_usefullCubesGlobal.addAll(branch.cond.allNegationSatisfiedCubes(state, pid));
					if(Options.PRINT_USED_CUBES_PER_STATEMENT){
						branch.m_usefullCubes.addAll(branch.cond.allNegationSatisfiedCubes(newstate, pid));
						branch.m_usefullCubes.addAll(branch.cond.allSatisfiedCubes(state, pid));
						branch.m_allCubes.addAll(branch.cond.allCubes());
					}
				}
				newstate.advancePC(pid);
			}
			succ.add(new Action(state, newstate, branch, pid));
//			succ.add(new Action(state, newstate, branch, pid, "" + pid + 
//					"." + branch.getLabel() + " " + branch.toString()));
		}
		
		return succ;
	}
	
	protected List<Action> applyAssume(SBState state, Assume assume, int pid) {
		List<Action> succ = new LinkedList<Action>();			
		List<SBState> newstates = new LinkedList<SBState>();
		
//		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
//			AssertComplexBool.m_allCubesLocal.clear();
//			AssertComplexBool.m_usefullCubesLocal.clear();
//		}
		boolean assumeDetermined = assume.cond.evaluate(state).isDetermined();
		
		if(Options.PRINT_USED_CUBES_PER_STATEMENT){
			AssertComplexBool.m_allCubesGlobal.addAll(assume.cond.allCubes());
		}
		if (!assumeDetermined){
			newstates = state.focus(assume,pid);
		}else{
			SBState newstate = (SBState) getStateFactory().newState(state);
			newstates.add(newstate);
		}
		
		for(SBState newstate : newstates){
			BoolValue assumeRes = assume.cond.evaluate(newstate);
			boolean assumePassed = assumeRes.isTrue();
			if (!assumePassed)
				newstate.setAssumeFailed();
		
			newstate.advancePC(pid);
			String action_name = "";
			if (Debug.DEBUG_LEVEL > 1){
				action_name =  "" + pid + 
						"." + assume.getLabel() + " " + assume.toString() + " Result: " + assumePassed;
			}
			
			succ.add(new Action(state, newstate, assume, pid));
			//succ.add(new Action(state, newstate, assume, pid,action_name));
			if(Options.PRINT_USED_CUBES){
				if(assumePassed){
					addAssertCubesToSet(assume.cond.allSatisfiedCubes(newstate),
							assume.cond.allCubes(),cubesAssertFalseTogetherMap);
					AssertComplexBool.m_usefullCubesGlobal.addAll(assume.cond.allSatisfiedCubes(newstate));
					AssertComplexBool.m_allCubesGlobal.addAll(assume.cond.allCubes());
					if(Options.PRINT_USED_CUBES_PER_STATEMENT){
						assume.m_usefullCubes.addAll(assume.cond.allSatisfiedCubes(newstate));
						assume.m_allCubes.addAll(assume.cond.allCubes());
					}
				}else{//for activation of focus unreachable.
					addAssertCubesToSet(assume.cond.allNegationSatisfiedCubes(newstate),
							assume.cond.allCubes(),cubesAssertTrueTogetherMap);
					AssertComplexBool.m_usefullCubesGlobal.addAll(assume.cond.allNegationSatisfiedCubes(newstate));
					AssertComplexBool.m_allCubesGlobal.addAll(assume.cond.allCubes());
					if(Options.PRINT_USED_CUBES_PER_STATEMENT){
						assume.m_usefullCubes.addAll(assume.cond.allNegationSatisfiedCubes(newstate));
						assume.m_allCubes.addAll(assume.cond.allCubes());
					}
				}
				
			}
		}
		
		return succ;
	}
	
	protected final List<Action> applyNop(SBState state, Statement statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		SBState newstate = (SBState) getStateFactory().newState(state);
		
		//NOP is not a complete nop - advance the PC...
		newstate.advancePC(pid);
		succ.add(new Action(state, newstate, statement, pid));
//		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
//				"." + statement.getLabel() + " " + statement.toString()));
		
		return succ;		
	}

	protected final List<Action> applyComment(SBState state, Statement statement, int pid)
	{
		List<Action> succ = new LinkedList<Action>();			
		SBState newstate = (SBState) getStateFactory().newState(state);
		
		//NOP is not a complete nop - advance the PC...
		newstate.advancePC(pid);
		succ.add(new Action(state, newstate, statement, pid));	
		//succ.add(new Action(state, newstate, statement, pid, statement.toString()));
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
		succ.add(new Action(state, newstate, statement, pid));
//		succ.add(new Action(state, newstate, statement, pid, "" + pid + 
		//"." + statement.getLabel() + " " + statement.toString()));
		
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
