package fsb.explore;


import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.Stack;

import net.sf.javabdd.BDD;

//import org.jgrapht.IntegerNameProvider;
import org.jgrapht.graph.DirectedWeightedMultigraph;

import ags.constraints.BDDFormula;
import ags.constraints.Formula;
import ags.graph.EdgeDecorator;
import ags.graph.EdgeEmptyNameProvider;
import ags.graph.StateDecorator;
import ags.graph.StateLabelProvider;
import ags.graph.StyledDotExporter;
import fsb.ast.Statement.StatType;
import fsb.utils.Debug;
import fsb.utils.Options;
import gnu.trove.map.TMap;
import gnu.trove.map.hash.THashMap;
import gnu.trove.set.hash.THashSet;
public class TransitionSystem {
	private static final String DOT_EXTENSION = ".dt";
	static private FileWriter m_writeBuff = null;

	// Graph
	// TODO: Make private again
	protected DirectedWeightedMultigraph<Vertex, Transition> graph = new DirectedWeightedMultigraph<Vertex, Transition>(
			Transition.class);

	// private Map<State, String> stateLab = new HashMap<State, String>();

	private Vertex initial;

	private Set<Vertex> cachedErrorStates;

	private TMap<State, Vertex> stateToVertex = new THashMap<State, Vertex>();

	static FileWriter writeBuff = null;

	@SuppressWarnings("unchecked")
	protected void write(String fileName) {
		write(fileName, new THashSet<Vertex>(), Collections.EMPTY_SET, null);
	}

	private void write(String fileName, Set<Vertex> doomedStates,
			Set<Transition> cutEdges, Transition stepEdge) {
		FileWriter fw;
		fileName = fileName + DOT_EXTENSION;
		System.out.println("Writing: " + fileName);

		try {
			fw = new FileWriter(fileName);
			write(fw, doomedStates, cutEdges, stepEdge);
			fw.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	private void write(Writer w, Set<Vertex> doomedStates,
			Set<Transition> cutEdges, Transition stepEdge) {
//		StyledDotExporter<Vertex, Transition> de = new StyledDotExporter<Vertex, Transition>(
//				new IntegerNameProvider<Vertex>(), new StateLabelProvider(),
//				new EdgeEmptyNameProvider<Transition>(), new StateDecorator(
//						initial), new EdgeDecorator(cutEdges, stepEdge));
//		// TODO: FIX!
//		de.export(w, graph);
	}

	protected void addState(Vertex s) {
		if (Debug.DEBUG_LEVEL > 3) {
			System.out.println("TS Adding " + s);
		}
		graph.addVertex(s);
		stateToVertex.put(s.getState(), s);
		// stateLab.put(s, s.toString());
	}

	protected void removeAllVerices(Collection<? extends Vertex> s) {
		if (Debug.DEBUG_LEVEL > 3) {
			System.out.println("TS Removing " + s);
		}
		graph.removeAllVertices(s);
		for(Vertex v : s){
			stateToVertex.remove(v.getState());
		}
	}
	
	protected void replacellVerices(Collection<? extends State> stateRemovedSet , Vertex replaced) {
		if (Debug.DEBUG_LEVEL > 3) {
			System.out.println("TS Replacing " + stateRemovedSet + " with " + replaced);
		}
		
		if(graph.containsVertex(replaced)){
			throw new RuntimeException("replacing with a vertex already in graph");
		}
		
		graph.addVertex(replaced);
		stateToVertex.put(replaced.getState(), replaced);
		
		for(State state : stateRemovedSet){
			Vertex vertex = stateToVertex.get(state);
			for(Transition e : graph.incomingEdgesOf(vertex)){
				Transition se = new Transition(e.getSrc(), replaced, e.getAction());
				graph.addEdge(e.getSrc(), replaced, se);
			}
			for(Transition e : graph.outgoingEdgesOf(vertex)){
				Transition se = new Transition(replaced, e.getDest(), e.getAction());
				graph.addEdge(replaced, e.getDest(), se);
			}
			graph.removeVertex(vertex);
			stateToVertex.remove(vertex.getState());
		}
	}
	
	
	protected void removeAllStates(Collection<? extends State> stateSet) {
		if (Debug.DEBUG_LEVEL > 3) {
			System.out.println("TS Removing " + stateSet);
		}
		for(State s : stateSet){
			graph.removeVertex(stateToVertex.get(s));
			stateToVertex.remove(s);
		}
	}
	protected void addTransition(Vertex src, Vertex dest, Action act) {
		Transition se = new Transition(src, dest, act);
		if (graph.containsEdge(src, dest))
			return;

		// This is completely redundant, addEdge will throw an exception
		if (!graph.containsVertex(src)) {
			System.out.println("Does not contain src " + src);
		}
		if (!graph.containsVertex(dest)) {
			System.out.println("Does not contain dest " + dest);
		}
		graph.addEdge(src, dest, se);
		// graph.setEdgeWeight(se, computeWeight(src.getState(),
		// dest.getState(), inst));
	}
	
	

	public Set<Vertex> getErrorStates() {
		if (cachedErrorStates == null) {
			cachedErrorStates = new HashSet<Vertex>();
			for (Vertex s : graph.vertexSet()) {
				if (s.getState().isErr()) {
					cachedErrorStates.add(s);
				}
			}
		}
		return cachedErrorStates;
	}

	public void setInitial(Vertex initialState) {
		initial = initialState;
	}

	public Vertex getInitial() {
		return initial;
	}

	protected Vertex getVertexForState(State s) {
		if(null == stateToVertex.get(s)){
			int i=0;
			i++;
		}
		return stateToVertex.get(s);
	}

	public int computeWeight(State current, State next, Action t) {
		return 0;
	}

	/* IBM! */
	@SuppressWarnings("unchecked")
	Formula analyzeFixedPoint(Formula initialConstraint) {
		System.out.println("Total states : " + graph.vertexSet().size());
		System.out.println("Error states : " + getErrorStates().size());

		// Mark all unavoidable states with false formulas...
		/*
		if (!markUnavoidable())
			return initialConstraint;
		*/
		
		Deque<Vertex> deq = new LinkedList<Vertex>();
		// Purely an optimization... hopefully actually does some good.
		HashSet<Vertex> onDeq = new HashSet<Vertex>();
		HashSet<Vertex> onStack = new HashSet<Vertex>();
		Formula allErrPaths = initialConstraint;

		for (Vertex v : graph.vertexSet())
			v.setVisited(false);

		// Put the whole transition system on the dequeue in post-order DFS
		// order.
		// Nothing interesting here, except possible bugs.
		Stack<Vertex> stack = new Stack<Vertex>();
		stack.push(initial);
		onDeq.add(initial);
		onStack.add(initial);
		for (Transition t : graph.outgoingEdgesOf(initial)) {
			stack.push(t.getDest());
			onStack.add(t.getDest());
		}
		initial.setVisited(true);

		while (!stack.isEmpty()) {
			Vertex curr = stack.peek();
			State currState = curr.getState();
			if (currState.isErr()) {
				stack.pop();
				deq.push(curr);
				onDeq.add(curr);
				onStack.remove(curr);
				curr.setVisited(true);
			} else if (currState.isFinal()) {
				stack.pop();
				deq.push(curr);
				onDeq.add(curr);
				onStack.remove(curr);
				curr.setVisited(true);
				curr.setSafe(true);
			} else if (curr.isVisited()) {
				stack.pop();
				deq.push(curr);
				onDeq.add(curr);
				onStack.remove(curr);
			} else {
				for (Transition t : graph.outgoingEdgesOf(curr)) {
					Vertex dest = t.getDest();
					if (!onStack.contains(dest) && !dest.isVisited()) {
						stack.push(dest);
						onStack.add(dest);
					}
				}
				curr.setVisited(true);
			}
		}
		

		if (getErrorStates().isEmpty()) {
			System.out.println("No error states, nothing to infer!");
			System.out.println("Avoid formula: " + allErrPaths);
			return allErrPaths;
		}
		allErrPaths = synthesisOfCodeCorrectionSuggestion(deq, onDeq, allErrPaths);
		
		return allErrPaths;
	}

	private Formula synthesisOfCodeCorrectionSuggestion(Deque<Vertex> deq,
			HashSet<Vertex> onDeq, Formula allErrPaths) {
		System.out.println("Start generation");
		// And now, the actual formula generation
		int min = deq.size();
		int maxChanged = 0;
		while (!deq.isEmpty()) {
			Vertex v = deq.pop();
			onDeq.remove(v);
			// Avoiding a state is a conjunction of disjunctions:
			// For every parent you need to either the parent or the
			// parent->child egde, and you must avoid ALL (parent | edge) pairs.

			Formula conj = Formula.trueValue();

			// So, for each incoming edge, add a clause to the conjunction
			for (Transition t : graph.incomingEdgesOf(v)) {
				Formula disj = Formula.falseValue();
				// DisjunctionFormula disj = new DisjunctionFormula();
				Formula parentFormula = t.getSrc().getFormula();

				State srcState = t.getSrc().getState();
				if (srcState.disjPredicates()) {
					// Avoiding an edge is actually also a disjunction - since
					// an edge may create (and usually does create)
					// several violations.
					Set<Formula> badPredicates = srcState.getPredicates(t
							.getAction());
					// TODO: This is debug only, shouldn't happen with any sane
					// semantics.
					if (badPredicates == null)
						continue;

					for (Formula bad : badPredicates)
						disj = disj.or(bad);

					//System.out.println("Bad Predicates: " + disj);

					if (!badPredicates.isEmpty()) {
						// Formula parentPlusEdge = parentFormula.deepCopy();
						Formula parentPlusEdge = parentFormula.or(disj);
						conj = conj.and(parentPlusEdge);
					} else {
						conj = conj.and(parentFormula);
					}
				} else {
					Formula edge = srcState.getAvoidFormula(t.getAction());
//					System.out.println("Edge:" + edge);
//					System.out.println("Parent:" + parentFormula);
//					System.out.println(parentFormula + " OR " + edge);
					
					Formula parentPlusEdge = parentFormula.or(edge);
					
					//System.out.println("P+E = " + parentPlusEdge);
					
					//System.out.println("conj = " + conj);
					
					conj = conj.and(parentPlusEdge);
					
					//System.out.println("conj&PE = " + conj);
				}
			}

			// If the formula changed, this means we need to propagate the
			// change to the children,
			// so add them back to the stack (if they're not already there)
		
			conj = conj.and(v.getFormula());
			//System.out.println("conj & v-formula:" + conj);
			
			if (!conj.equals(v.getFormula())) {
				v.setFormula(conj);
				if (v.formulaChanged > maxChanged) {
					maxChanged = v.formulaChanged;
					System.out.println("Max formula changes per vertex: "
							+ maxChanged);
					//System.out.println(v.getFormula());
					if (!Formula.useBDD()) {
						BDD bdd = BDDFormula.getBDD(v.getFormula());
						System.out.println(bdd);
						System.out.println("from BDD: "
								+ BDDFormula.getFormula(bdd));
					}
				}

				for (Transition out : graph.outgoingEdgesOf(v)) {
					Vertex dest = out.getDest();
					if (!onDeq.contains(dest)) {
						if (!dest.isSafe()) {
							deq.push(dest);
							onDeq.add(dest);
						}
					}
				}
			}

			if (deq.size() < min) {
				min = deq.size();
				if (min % 1000 == 0) {
					System.out.println("Minimum size of dequeue: " + min);
				}
			}
		}

		for (Vertex v : getErrorStates())
			allErrPaths = allErrPaths.and(v.getFormula());

		System.out.println("Avoid formula: " + allErrPaths);
		List<Set> l = ((BDDFormula)allErrPaths).getSolutions(); 
		for(Set s : l)
		{
			System.out.println("Solution: " + s);
		}
		return allErrPaths;
	}

	private boolean markUnavoidable() {
		System.out.println("Marking unavoidable states");
		for (Vertex v : graph.vertexSet())
			v.setVisited(false);

		// Put the whole transition system on the dequeue in post-order DFS
		// order.
		Stack<Vertex> stack = new Stack<Vertex>();
		stack.push(initial);
		initial.setVisited(true);

		int count = 0;

		// DFS
		while (!stack.isEmpty()) {
			Vertex curr = stack.pop();
			for (Transition t : graph.outgoingEdgesOf(curr)) {
				if (!t.getSrc().getState().isAvoidable(t.getAction())
						&& !t.getDest().isVisited()) {
					count++;
					stack.push(t.getDest());
					t.getDest().setVisited(true);
					t.getDest().setFormula(Vertex.initStateFormula);
					if (t.getDest().getState().isErr()) {
						System.out.println("Found unavoidable error state!");
						return false;
					}
				}
			}
		}

		System.out.println("Marked " + count + " unavoidable states");
		return true;
	}
	

	/**
	 * Quick and dirty recursive version. Write an iterative one if this one causes problems.
	 * @param source
	 * @param target
	 */
	public void printTrace(Vertex source, Vertex target)
	{
		for (Vertex v : graph.vertexSet())
			v.setVisited(false);
		
		Deque<Transition> path = new LinkedList<Transition>();
		printTraceHelper2(source, target, path);
	}
	/**
	 * 
	 * searches the first trace from source to target in a DFS manner.
	 * @param source
	 * @param target
	 * @param path
	 * @return
	 */
	private boolean printTraceHelper(Vertex source, Vertex target, Deque<Transition> path)
	{
		String fileToWriteTraceTo = Options.fileNameForErrorTrace;
		FileWriter writeBuff = getWriteBuffOrNull(fileToWriteTraceTo);
		
		if (source.equals(target))
		{
			printFoundErrorTrace(path, fileToWriteTraceTo, writeBuff);
			System.out.println(target.getState());
			writeToFile(target.getState().toString(), fileToWriteTraceTo, writeBuff);
			
			return true;
		}
		else
		{
			source.setVisited(true);
			for (Transition t: graph.outgoingEdgesOf(source))
			{
				Vertex nextSource = t.getDest();
				if (!nextSource.isVisited())
				{
					path.addLast(t);
					boolean found = printTraceHelper(nextSource, target, path);
					if (found)
						return true;
					path.removeLast();
				}
			}
			return false;
		}
	}
	private boolean printTraceHelper2(Vertex source, Vertex target, Deque<Transition> path)
	{
		String fileToWriteTraceTo = Options.fileNameForErrorTrace;
		FileWriter writeBuff = getWriteBuffOrNull(fileToWriteTraceTo);
		Stack<Iterator<Transition>> stack = new Stack<Iterator<Transition>>();
		Iterator<Transition> it = graph.outgoingEdgesOf(source).iterator();
		stack.push(it);
		
	while(!stack.isEmpty())
	{
		if (source.equals(target))
		{
			printFoundErrorTrace(path, fileToWriteTraceTo, writeBuff);
		
			System.out.println(target.getState());
			writeToFile(target.getState().toString(), fileToWriteTraceTo, writeBuff);
			
			return true;
		}
		//if reached here source is not equal to target updating trace
		source.setVisited(true);
		
		Iterator<Transition> top = stack.peek();
		if(!top.hasNext()){
			path.removeLast();
			stack.pop();
			continue;
		}
		Transition current_transition = top.next();
					
		Vertex nextSource = current_transition.getDest();
				
		if (!nextSource.isVisited()){
			path.addLast(current_transition);
			source=nextSource;
			Iterator<Transition> it2 = graph.outgoingEdgesOf(source).iterator();
			stack.push(it2);
		}
			
	
	}
	//reached here the stack was empty all possible paths exhausted, error trace not found. 
	return false;
	}

	private void printFoundErrorTrace(Deque<Transition> path,
			String fileToWriteTraceTo, FileWriter writeBuff) {
		for (Transition t : path) {
			if(Options.PRINT_ONLY_COMMENTS_IN_ERROR_TRACE && ! (t.getAction().getStatement().getType() == StatType.C_COMMENT) ){
				if(!Options.SUCCINT_ERROR_TRACE){
					System.out.println("\n");
					System.out.println("");
					writeToFile("\n", fileToWriteTraceTo, writeBuff);
					writeToFile("", fileToWriteTraceTo, writeBuff);
				}
			}else{
				if(!Options.SUCCINT_ERROR_TRACE){
					System.out.println(t.getAction().getSource());
					writeToFile(t.getAction().getSource().toString(), fileToWriteTraceTo, writeBuff);
				}
				if(!Options.SUCCINT_ERROR_TRACE || t.getAction().toString().contains("Statement")){
					System.out.println(t.getAction());
				
					writeToFile(t.getAction().toString(), fileToWriteTraceTo, writeBuff);
				}
			
			}
		
			
		}
	}

	private void writeToFile(String s, String fileToWriteTraceTo,
			FileWriter writeBuff) {
		if(null != fileToWriteTraceTo){
			try {
				writeBuff.write(s);
				writeBuff.write("\n");
				writeBuff.flush();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
	}

	private FileWriter getWriteBuffOrNull(String fileToWriteTraceTo) {
		if(null == m_writeBuff){
			if(fileToWriteTraceTo != null){
				try {
					m_writeBuff = new FileWriter(fileToWriteTraceTo);
				} catch (IOException e) {
					m_writeBuff = null;
					e.printStackTrace();
				}
			}
		}
		return m_writeBuff;
	}

	/*
	 * first step towards symbolic execution I want to run the error trace in the concrete program,
	 * my approach is use the comments:
	 * 1. get the predicates from a file
	 * 2. collect all the assert statements
	 * 3. parse from the comment assignment and load statements.
	 */
	public void printPathEquasion(Vertex source, Vertex target) {
		for (Vertex v : graph.vertexSet())
			v.setVisited(false);
		
		Deque<Transition> path = new LinkedList<Transition>();
		printTraceHelper(source, target, path);
		
	}
}
