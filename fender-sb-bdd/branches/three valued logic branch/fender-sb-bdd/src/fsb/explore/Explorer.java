package fsb.explore;

import java.util.Deque;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import ags.constraints.Formula;

import fsb.semantics.Semantics;
import fsb.utils.Debug;
import fsb.utils.Options;

public class Explorer
{
	private String filename = "ts";
	private State initialState;
	private Semantics sem;

	public void SetInitial(State init) {
		// init.setScheduler(true);
		initialState = init;
	}

	public void setFileName(String fn) {
		filename = fn;
	}

	public Explorer(Semantics sem) {
		this.sem = sem;
	}

	/**
	 * This build the transition system, and then hands it off
	 * to find the constraint formula, which is returned.
	 * <IBM!> 
	 */
	public void explore() {
		Runtime.getRuntime().gc();
		
		System.out.println("\n--- STARTING EXPLORATION --- ");
		long startTime = System.currentTimeMillis();
		
		//Lots of initialization...
		TransitionSystem ts = new TransitionSystem();
		Map<State, State> space = new HashMap<State,State>();
		Stack<State> stack = new Stack<State>();		
		
		System.out.println("Pushing initial state:" + initialState);		
		Vertex initialV = new Vertex(initialState, 0);			
		stack.push(initialState);
		space.put(initialState, initialState);
						
		initialV.setFormula(Vertex.initStateFormula);
		ts.setInitial(initialV);
		ts.addState(initialV);		
		
		enumerateStateSpaceViaDFS(ts, space, stack, initialV);
		
		if(!Options.ONLY_MODEL_CHECKER){//TODO: for now the following does nothing
			ts.analyzeFixedPoint(Formula.trueValue());
		}
		long memUsage = (Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory());
		space = null;		
		long endTime = System.currentTimeMillis();

		//Write out the TS.
		//Note that this might take a whole lot of time for large TS...		
		if (Options.WRITE_TS)
			ts.write(filename);
		
		System.out.println("---FINISHED---");
		System.out.println("States: " + ts.graph.vertexSet().size() + ", Edges : " + ts.graph.edgeSet().size());
		System.out.println("Error states: " + ts.getErrorStates().size());
		System.out.println("Running Time: " + (endTime - startTime) + " ms");
		System.out.println("Memory usage: " + (int)(memUsage/(1024.0*1024)) + " MB");
		
		System.gc();
	}

	private void enumerateStateSpaceViaDFS(TransitionSystem ts,
			Map<State, State> space, Stack<State> stack, Vertex initialV) {
		int cnt = 0;
		//int allowedWeight = 100;
		int allowedWeight = 0;
		
		boolean foundError = false;
		
		//Enumerate the entire state-space, by DFS.
		while (!stack.isEmpty()) {
			cnt++;
			if (cnt % 1000 == 0) {
				System.out.println("Depth: " + stack.size() + " Space: "
						+ space.size() + " Edges: " + ts.graph.edgeSet().size());
			}			
			
			State current = stack.pop();
			Vertex currentV = ts.getVertexForState(current);
						
			
			/*
			int ssize = stack.size();			
			if (ssize > 200000)
			{				
				System.out.println("DEBUG: stack size too large, bailing. Are you sure this program terminates?");
				break;
			}*/
			
			
			/*
			//TODO: Remove
			if(space.size() > 1000)
			{
				System.out.println("Debug, stopping at 300K states");
				break;
			}
			*/
			
			State found = space.get(current);
			
			if (found == null) {
				System.out.println("Error, state = " + current
						+ " is not in space");
			}
			
			if (Options.PRINT_FIRST_ERROR_TRACE && !foundError && current.isErr())
			{
				foundError = true;
				ts.printTrace(initialV, currentV);
				ts.printPathEquasion(initialV, currentV);
			}

			// do not continue exploring from error states
			if (Options.AVOID_EXPLORING_FROM_ERR && current.isErr()) {
				//System.out.println("Error!");
				//TODO: We want to keep exploring, so this should be continue. break is for debug.
				continue;
				//break;
			}
			
			List<Action> successors;
			//Find which statements are enabled.
			try
			{
				successors = sem.getSuccessors(current);
			}
			catch(BadMemoryException e)
			{
				//There is a statement from this state that leads to an uninitialized memory access. 
				//Mark this state as bad,
				current.setBad();
				continue;
			}
			
			for (Action t : successors) {
				if (Debug.DEBUG_LEVEL > 1) {
					System.out.println();
					System.out.println("Process " + t.getPid() + " Applying " + t);
					System.out.println("BEFORE: " + current);
					System.out.println();
				}
								
				// apply action
				State next = t.getTarget();
			
				//TODO: The correct place to do this would be, of course, before the application. 
				//Need to change the interface to allow this.
				int newWeight = currentV.getWeight() + ts.computeWeight(current, next, t);
				//Primitive context-bounding.
				if ((allowedWeight > 0) && (newWeight > allowedWeight))
					continue;
				
				Vertex nextV;
				//If not in the state-space, add it.
				found = space.get(next);				
				if (found == null) {
					space.put(next, next);
					nextV = new Vertex(next, newWeight);
					ts.addState(nextV);
					stack.push(next);
				}
				else
				{
					//First of all, it's possible that we need to perform a join here.
					//Note that the join must NOT change the hashcode!
					boolean changed = found.join(next);
					//TODO: Need to check it's not already on the stack?
					//if ((changed) && (!stack.contains(found))) <-- Poor performance, obviously.
					if (changed)
						stack.push(found);
					//The vertex's weight possibly changed and needs to be reduced.
					nextV = ts.getVertexForState(found);
					nextV.reduceWeight(newWeight);					
				}
				
				//In any case, the transition is new.
				//Do not add self-loops!
				if (!currentV.equals(nextV))
					ts.addTransition(currentV, nextV, t);
				
				if (Debug.DEBUG_LEVEL > 1) {
					System.out.println(current.toString() + " -> " + next.toString()
							+ " [label=\"" + t + "\"]");
				}
			}
		}
	}	
}
