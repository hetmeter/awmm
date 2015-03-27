package fsb.explore;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Stack;

import ags.constraints.Formula;
import fsb.ast.AssertComplexBool;
import fsb.ast.ComplexBool;
import fsb.explore.statespace.HashMapStateSpace;
import fsb.explore.statespace.StateQueue;
import fsb.explore.statespace.StateSpaceInterface;
import fsb.explore.statespace.StateStack;
import fsb.explore.statespace.StatesToExploreInterface;
import fsb.explore.statespace.TrieMapStateSpace;
import fsb.semantics.Semantics;
import fsb.semantics.sc.SCState;
import fsb.semantics.sc.SCStateWithStatistics;
import fsb.utils.Debug;
import fsb.utils.Options;
import gnu.trove.map.hash.TIntObjectHashMap;

public class Explorer {

	
	private String m_filename = "ts";
	private State m_initialState;
	private Semantics m_sem;
	private long m_max_mem_usage = -1;

	/**
	 * used when {@linkplain Options#DELETE_STATES_IN_ATOMICS} is true
	 */
	private TIntObjectHashMap<StateSpaceInterface> m_statesInAtomics = null;
	
	public void SetInitial(State init) {
		// init.setScheduler(true);
		m_initialState = init;
	}

	public void setFileName(String fn) {
		m_filename = fn;
	}

	public Explorer(Semantics sem) {
		
		this.m_sem = sem;
		this.m_statesInAtomics = new TIntObjectHashMap<StateSpaceInterface>(State.numProcs);
		for(int i=0;i<State.numProcs;++i){
			this.m_statesInAtomics.put(i, getNewStateSpaceStruct());
		}
	}

	/**
	 * This build the transition system, and then hands it off to find the
	 * constraint formula, which is returned. <IBM!>
	 */
	public void explore() {
		Runtime.getRuntime().gc();

		System.out.println("\n--- STARTING EXPLORATION --- ");
		long startTime = System.currentTimeMillis();

		// Lots of initialization...
		TransitionSystem ts = new TransitionSystem();
		StateSpaceInterface space = getNewStateSpaceStruct();
		
		StatesToExploreInterface stack = getStatesToExploreStorageStruct();

		System.out.println("Pushing initial state:" + m_initialState);
		Vertex initialV = new Vertex(m_initialState, 0);
		stack.push(m_initialState);
		
		space.putInSubSpaceAndRemoveSubsumed(m_initialState);

		initialV.setFormula(Vertex.initStateFormula);
		ts.setInitial(initialV);
		ts.addState(initialV);

		enumerateStateSpaceViaDFS(ts, space, stack, initialV);

		if (!Options.ONLY_MODEL_CHECKER) {// TODO: for now the following does
											// nothing
			ts.analyzeFixedPoint(Formula.trueValue());
		}
		if(Options.INVOKE_GC_BEFORE_END_STATISTICS){
			System.gc();
		}
		long memUsage = (Runtime.getRuntime().totalMemory() - Runtime
				.getRuntime().freeMemory());
		
		long endTime = System.currentTimeMillis();

		debugPrintOutAtTheEndOfExecution(ts, endTime, space);
		System.out.println("---FINISHED---");
		System.out.println("States: " + space.size()
				+ ", Edges : " + ts.graph.edgeSet().size());
		System.out.println("Error states: " + ts.getErrorStates().size());
		System.out.println("Running Time: " + (endTime - startTime) + " ms");
		System.out.println("Memory usage: "
				+ (int) (memUsage / (1024.0 * 1024)) + " MB");

		if(-1 != m_max_mem_usage){
			if(m_max_mem_usage < memUsage){
				m_max_mem_usage = memUsage;
			}
			System.out.println("Max mem usage: " 
					+ (int) (m_max_mem_usage / (1024.0 * 1024)) + " MB");
		}

		space = null;
		System.gc();
	}

	/**
	 * @return
	 */
	private StatesToExploreInterface getStatesToExploreStorageStruct() {
		if(Options.EXPLORATION_METHOD_DFS){
			return new StateStack(State.numProcs);
		}else{
			return new StateQueue(State.numProcs);
		}
	}

	private StateSpaceInterface getNewStateSpaceStruct() {
		switch(Options.USE_FOR_STATE_SPACE){
			case TRIE: {
				return new TrieMapStateSpace();
			}
		
			case HASHMAP: {
				return new HashMapStateSpace(); 
			}
		
		//	case BDD:
			
			}
		return null;
	}

	private class Pids{
		/* (non-Javadoc)
		 * @see java.lang.Object#hashCode()
		 */
		public Pids(int numProcesses){
			this.pid = new int[numProcesses];
		}
		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + getOuterType().hashCode();
			result = prime * result + Arrays.hashCode(pid);
			return result;
		}

		/* (non-Javadoc)
		 * @see java.lang.Object#equals(java.lang.Object)
		 */
		@Override
		public boolean equals(Object obj) {
			if (this == obj) {
				return true;
			}
			if (obj == null) {
				return false;
			}
			if (!(obj instanceof Pids)) {
				return false;
			}
			Pids other = (Pids) obj;
			if (!getOuterType().equals(other.getOuterType())) {
				return false;
			}
			if (!Arrays.equals(pid, other.pid)) {
				return false;
			}
			return true;
		}

		public int pid[] = null;

		private Explorer getOuterType() {
			return Explorer.this;
		}
		
	}
	/**
	 * @param ts
	 * @param endTime
	 */
	private void debugPrintOutAtTheEndOfExecution(TransitionSystem ts,
			long endTime, StateSpaceInterface space) {
		
//		int[] count = new int[State.numProcs];
//		int counter = 0;
//		THashSet<Pids> pids = new THashSet<Pids>();
//		for(int i=0;i<State.numProcs;i++){
//			count[i]=0;
//		}
//		for(int s_i : space.keys()){
//			THashMap<State> subSpace = space.get(s_i);
//			if(null == subSpace){
//				continue;
//			}
//			for(State s: subSpace.keySet()){
//				if(s.inAtomicSection() != -1){
//					count[s.inAtomicSection()]++;
//				}
//				Pids pid = new Pids(State.numProcs);
//				for(int i=0; i< State.numProcs; ++i){
//					pid.pid[i] = s.getPC(i);
//				}
//				pids.add(pid);
//				counter++;
//			}
////		}
//		for(int i=0;i<State.numProcs;i++){
//			System.out.println("Process "+ i + " has " + count[i] + " states in atomics");
//		}
//		System.out.println("num distinct labels: " + pids.size());
//		System.out.println("counter " + counter);
		
		// Write out the TS.
		// Note that this might take a whole lot of time for large TS...
		if (Options.WRITE_TS){
			System.out.println("writing the transition system to =" + m_filename+ Long.toString(endTime)+"_ts.dt");
			ts.write(m_filename+Long.toString(endTime)+"_ts");
		}
		if (Options.PRINT_USED_CUBES) {
				System.out.println("num Assert and assume cubes is: "
						+ AssertComplexBool.m_allCubesGlobal.size() + " num used: "
						+ AssertComplexBool.m_usefullCubesGlobal.size());
				System.out.println("num ifgoto and choose cubes is: "
						+ ComplexBool.m_allCubesGlobal.size() + " num used: "
						+ ComplexBool.m_usefullCubesGlobal.size());

				System.out
						.println("The usefull Assert and assume boolean cubes are: "
								+ AssertComplexBool.m_usefullCubesGlobal.toString());
				System.out
						.println("The usefull boolean cubes in ifgoto and choose are: "
								+ ComplexBool.m_usefullCubesGlobal.toString());
				AssertComplexBool.m_allCubesGlobal
						.removeAll(AssertComplexBool.m_usefullCubesGlobal);
				ComplexBool.m_allCubesGlobal.removeAll(ComplexBool.m_usefullCubesGlobal);
				System.out
						.println("num NONusefull Assert and assume cubes is: "
								+ AssertComplexBool.m_allCubesGlobal.size());
				System.out
						.println("num NONusefull ifgoto and choose cubes is: "
								+ ComplexBool.m_allCubesGlobal.size());
				Semantics.printTrueFalseList();
				if(Options.PRINT_USED_CUBES_PER_STATEMENT){
					System.out.println(State.program);
				}
			
		}
		if(Debug.DEBUG_LEVEL>0 && fsb.utils.Options.PRINT_FOCUS_STATISTICS){

			SCStateWithStatistics.printFocusStatistics();
		}
	}

	private void enumerateStateSpaceViaDFS(TransitionSystem ts,
			StateSpaceInterface space, StatesToExploreInterface stack, Vertex initialV) {
		int cnt = 0;
		// int allowedWeight = 100;
		int allowedWeight = 0;

		boolean foundError = false;

		// Enumerate the entire state-space, by DFS.
		while (!stack.isEmpty()) {
			cnt++;
			if (cnt % 1000 == 0) {
				long memUsage = -1;
				if(Options.PRINT_INTERMEDIATE_MAX_MEM){
					memUsage = (Runtime.getRuntime().totalMemory() - Runtime
							.getRuntime().freeMemory());
					if(m_max_mem_usage < memUsage){
						m_max_mem_usage = memUsage;
					}
				}
				System.out
						.println("Depth: " + stack.size() + " Space: "
								+ space.size() + " Edges: "
								+ ts.graph.edgeSet().size()
								+ ((-1 == memUsage)?"" : (" Memory " + (int) (memUsage / (1024.0 * 1024)) + " MB")));
				
			}

			State current = stack.pop();
			
			Vertex currentV = null;
			if(Options.USE_TS){
				currentV = ts.getVertexForState(current);
				if(currentV == null){
					throw new RuntimeException("transition systme doesn't have a vertex for the element " + current);
				}
			}

			/*
			 * int ssize = stack.size(); if (ssize > 200000) {
			 * System.out.println(
			 * "DEBUG: stack size too large, bailing. Are you sure this program terminates?"
			 * ); break; }
			 */

			/*
			 * //TODO: Remove if(space.size() > 1000) {
			 * System.out.println("Debug, stopping at 300K states"); break; }
			 */

			State found = null;
			Set<State> foundSet = null;

			if(!Options.DELETE_STATES_IN_ATOMICS || current.inAtomicSection() == -1){
				found = space.myFindStateExactOrSubsume(current);
			}else{
				found = m_statesInAtomics.get(current.inAtomicSection()).myFindStateExact(current);
			}
			if (found == null) {
				System.out.println("Error, state = " + current
						+ "in atomic" + current.inAtomicSection() + " is not in space");
			}
			if (Options.PRINT_FIRST_ERROR_TRACE && !foundError
					&& current.isErr()) {
				foundError = true;
				ts.printTrace(initialV, currentV);
				// YURI: maybe later add here print path equation
				// ts.printPathEquasion(initialV, currentV);
			}

			// do not continue exploring from error states
			if (Options.AVOID_EXPLORING_FROM_ERR && current.isErr()) {
				// System.out.println("Error!");
				// TODO: We want to keep exploring, so this should be continue.
				// break is for debug.
				
				// a hack - if we don't have a transition system,
				//but still want to that we had an error
				if(!Options.USE_TS){
					Vertex tempV = new Vertex(current, 0);
					ts.addState(tempV);
					System.out.println("\n" + current);
				}
				if (fsb.utils.Options.HALT_AT_FIRST_ERROR_TRACE)
					break;
				else
					continue;
			}

			List<Action> successors;
			// Find which statements are enabled.
			try {
				successors = m_sem.getSuccessors(current);
			} catch (BadMemoryException e) {
				// There is a statement from this state that leads to an
				// uninitialized memory access.
				// Mark this state as bad,
				current.setBad();
				continue;
			}

			for (Action t : successors) {
				if (Debug.DEBUG_LEVEL > 3) {
					System.out.println();
					System.out.println("Process " + t.getPid() + " Applying "
							+ t);
					System.out.println("BEFORE: " + current);
					System.out.println();
				}

				// apply action
				State next = t.getTarget();
				
				int newWeight = -1;
				if(Options.USE_TS){

					// TODO: The correct place to do this would be, of course,
					// 	before the application.
					// Need to change the interface to allow this.
					newWeight = currentV.getWeight()
							+ ts.computeWeight(current, next, t);
					// Primitive context-bounding.
					if ((allowedWeight > 0) && (newWeight > allowedWeight))
						continue;
				}
				Vertex nextV = null;
				// If not in the state-space, add it.
				
				if(!Options.DELETE_STATES_IN_ATOMICS || next.inAtomicSection() == -1){
					foundSet = space.myFindStateThatSubsume(next);
				}else{
					foundSet = m_statesInAtomics.get(next.inAtomicSection()).myFindStateThatSubsume(next);
				}
//				if(next.isErr()){
//					int i=0;
//					i++;
//				}
				Set<State> removedStates = new HashSet<State>();
				if(foundSet == null || foundSet.isEmpty()) {
					//if Deleting atomic intermidiate states the register for deletion will store the
					//state
					if(!Options.DELETE_STATES_IN_ATOMICS || next.inAtomicSection() == -1){
						
						removedStates.addAll(space.putInSubSpaceAndRemoveSubsumed(next));
					}
					if(Options.USE_TS){
						nextV = new Vertex(next, newWeight);
						ts.addState(nextV);
						ts.getVertexForState(next);
					}
					stack.push(next);
					removedStates.addAll(registerForDeletionIfInAtomicsAndRemoveSubsumed(next));
				} else {
					// First of all, it's possible that we need to perform a
					// join here.
					// Note that the join must NOT change the hashcode!
					State f_chngd = SCState.topOfSet(foundSet);
					boolean changed = f_chngd.join(next);
					// TODO: Need to check it's not already on the stack?
					// if ((changed) && (!stack.contains(found))) <-- Poor
					// performance, obviously.
					if (changed || foundSet.size() > 1){
						if(!Options.DELETE_STATES_IN_ATOMICS || f_chngd.inAtomicSection() == -1){
							removedStates.addAll(replace_joined_AddAndRemoveSubsumed(space, found, f_chngd));
						}else{
							removedStates.addAll(replace_joined_AddAndRemoveSubsumed(m_statesInAtomics.get(f_chngd.inAtomicSection()),found,f_chngd));
						}
						
						removedStates.addAll(registerForDeletionIfInAtomicsAndRemoveSubsumed(f_chngd));
						if(foundSet.size() > 1){
							throw new RuntimeException("for now found set should be a singleton");
						}
						
						stack.push(f_chngd);
						
						if(Options.USE_TS){
							nextV = new Vertex(f_chngd, newWeight);;
							ts.addState(nextV);
							//ts.replacellVerices(removedStates, nextV);
							//ts.getVertexForState(f_chngd);
						}

						//TODO: not deleting state in atomic from the transition system
					}else{
						if(Options.USE_TS){
							nextV = ts.getVertexForState(f_chngd);
						}
					}
					// The vertex's weight possibly changed and needs to be
					// reduced.
					if(Options.USE_TS){
						nextV.reduceWeight(newWeight);
					}
				}//if(foundSet == null || foundSet.isEmpty()) ..else 
				
				
				if(Options.USE_TS){
					//idealy I would of wanted to remove the removed states from the state graph
					//such as atomics and subsumed
					//this is not the point to do this probably
					// and should take into account that the stack doesn't have the subsumed states 
					// replaced by the new inserted state for efficiency
					
					//ts.removeAllStates(removedStates);
				}
				if(Options.USE_TS){
				// In any case, the transition is new.
				// Do not add self-loops!
					if (!currentV.equals(nextV))
						ts.addTransition(currentV, nextV, t);
				}
				debugPrintoutsAfterAddedState(current, t, next);
			}//end of for (Action t : successors)
			
			deleteInAtomicsAfterFinishingWithCurrent(space, current,stack);
		}//end of while (!stack.isEmpty()
	}

	
	/**
	 * @param space
	 * @param found
	 * @param f_chngd
	 */
	private Set<State> replace_joined_AddAndRemoveSubsumed(StateSpaceInterface space, State found, State f_chngd) {

//		Set<State> ret = space.removeFromSubSpaceAllSubsumedBy(f_chngd);
//		
//		ret.addAll(space.putInSubSpaceAndRemoveSubsumed(f_chngd));
//		
//		return ret;
		return space.putInSubSpaceAndRemoveSubsumed(f_chngd);
	}



	
	
	/**
	 * @param space
	 * @param current
	 */
	private void deleteInAtomicsAfterFinishingWithCurrent(
			StateSpaceInterface space, State current, StatesToExploreInterface stack) {
		
		if(current.inAtomicSection != -1 && Options.DELETE_STATES_IN_ATOMICS){
			
			int pid_thats_inAtomic = current.inAtomicSection;
			
			if(stack.numStateOnStackForProcess(pid_thats_inAtomic) == 0){
					m_statesInAtomics.get(pid_thats_inAtomic).clear();
				//m_statesInAtomics.put(pid_thats_inAtomic, getNewStateSpaceStruct());
			}
		}
		
	}

	/**
	 * @param next
	 */
	private Set<State> registerForDeletionIfInAtomicsAndRemoveSubsumed(State next) {
		Set<State> ret = new HashSet<State>();
		if(next.inAtomicSection != -1 && Options.DELETE_STATES_IN_ATOMICS){
			int pid_thats_inAtomic = next.inAtomicSection;
			ret = m_statesInAtomics.get(pid_thats_inAtomic).putInSubSpaceAndRemoveSubsumed(next);
		}
		return ret;
	}

	/**
	 * @param current
	 * @param t
	 * @param next
	 */
	private void debugPrintoutsAfterAddedState(State current, Action t,
			State next) {
		if (Debug.DEBUG_LEVEL > 0 && Options.PRINT_STATE_AT_PROCESS != -1) {
			int nextRealPC = next.getPC(Options.PRINT_STATE_AT_PROCESS);
			int currRealPC = current.getPC(Options.PRINT_STATE_AT_PROCESS);
			int nextStatePC = State.program.getListing(Options.PRINT_STATE_AT_PROCESS).
					get(nextRealPC).getLabel();
			int currStatePC = State.program.getListing(Options.PRINT_STATE_AT_PROCESS).
					get(currRealPC).getLabel();
			if(nextStatePC != currStatePC){
				
				if( nextStatePC== Options.PRINT_STATE_AT_LABEL){
					System.out.println(next.toString());
				}
			}
			
		}
		if (Debug.DEBUG_LEVEL > 3) {
			System.out.println(current.toString() + " -> "
					+ next.toString() + " [label=\"" + t + "\"]");
		}
	}
}
