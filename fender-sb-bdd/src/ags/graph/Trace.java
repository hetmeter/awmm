package ags.graph;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import fsb.explore.Transition;
import fsb.explore.Vertex;

/**
 * 
 * @author yahave
 * 
 * @TODO: EY - clean all this forward business. We never really use it, and it
 *        complicates the code and makes it fragile.
 */
public class Trace {
	private List<Transition> steps;

	private boolean forward;
	
	public static int debugCount = 0;

	public Trace() {
		steps = new ArrayList<Transition>();
	}

	public Trace(Trace t) {
		steps = new ArrayList<Transition>(t.steps);
		forward = t.forward;
	}

	public Trace(Transition step, boolean fwd) {
		this();
		this.forward = fwd;
		addStep(step);
	}

	public void addStep(Transition step) {
		steps.add(step);
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		Trace other = (Trace) obj;
		if (forward != other.forward)
			return false;
		if (steps == null) {
			if (other.steps != null)
				return false;
		} else if (!steps.equals(other.steps))
			return false;
		return true;
	}

	/**
	 * If it is a forward trace, return the first state in steps. If it is a
	 * backward trace, return the last state of the last transition.
	 * 
	 * @return
	 */
	public Vertex firstState() {
		if (steps.isEmpty()) {
			return null;
		}
		return firstStep().getSrc();
	}

	private Transition firstStep() {
		if (steps.isEmpty()) {
			return null;
		}
		if (forward) {
			return steps.get(0);
		} else {
			int size = steps.size();
			return steps.get(size - 1);
		}
	}

	/**
	 * returns a set of transitions { ti | pid(ti) != pid(ti+1) }
	 * 
	 * @return
	 */
	public Set<Transition> getFollowedByCtxSwith() {
		Set<Transition> result = new HashSet<Transition>();
		for (int i = steps.size() - 1; i > 0; i--) {
			Transition curr = steps.get(i);
			Transition next = steps.get(i - 1);
			if (curr.getPid() != next.getPid()) {
				result.add(curr);
			}
		}
		return result;
	}

	/**
	 * Returns the next transition in this trace that executes with the same
	 * pid, or null if no such transition exists
	 * 
	 * currently assumes trace goes backwards
	 * 
	 * @param t
	 * @return
	 */
	public Transition getFollows(Transition t) {
		int index = getIndex(t) - 1;
		int pid = t.getPid();
		for (int i = index; i >= 0; i--) {
			Transition curr = steps.get(i);
			if (curr.getPid() == pid) {
				return curr;
			}
		}
		return null;
	}

	/**
	 * Returns the index of the given transition.
	 * 
	 * currently just iterate (inefficient), later if we care introduce a
	 * hashmap.
	 * 
	 * @param t
	 * @return
	 */
	private int getIndex(Transition t) {
		for (int i = 0; i < steps.size(); i++) {
			Transition curr = steps.get(i);
			if (curr.equals(t)) {
				return i;
			}
		}
		return -1;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (forward ? 1231 : 1237);
		result = prime * result + ((steps == null) ? 0 : steps.hashCode());
		return result;
	}

	public int length() {
		return steps.size();
	}

	public String toString() {
		StringBuffer sb = new StringBuffer();
		if (forward) {
			for (int i = 0; i < steps.size(); i++) {
				sb.append(steps.get(i).getPid());
				sb.append(":");
				sb.append(steps.get(i).getAction());
				sb.append(" -> ");
			}
		} else {
			for (int i = steps.size() - 1; i >= 0; i--) {
				sb.append(steps.get(i).getPid());
				sb.append(":");
				sb.append(steps.get(i).getAction());
				sb.append(" -> ");
			}
		}

		return sb.toString();
	}
	
	/**
	 * does the trace contain the given transition?
	 * 
	 * @param x
	 * @return
	 */
	public boolean contains(Transition x) {
		return steps.contains(x);
	}

	public Collection<Transition> getSteps() {
		return steps;
	}
}
