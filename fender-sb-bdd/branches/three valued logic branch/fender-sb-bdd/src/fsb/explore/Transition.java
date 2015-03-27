package fsb.explore;

import org.jgrapht.graph.DefaultWeightedEdge;


/**
 * A transition in the transition system, from a source state to a destination
 * state, labeled by the statement executed.
 * 
 * @author yahave
 * 
 */
public class Transition extends DefaultWeightedEdge implements Comparable<Transition>{
	private Vertex src;

	private Vertex dest;

	private Action act;
	
	/**
	 * generated
	 */
	private static final long serialVersionUID = 8000494412561944469L;

	protected Transition(Vertex src, Vertex dest, Action act) {
		this.act = act;
		this.src = src;
		this.dest = dest;
	}

	@Override
	public boolean equals(Object obj) {
		//TODO: Does runNumber count?
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		final Transition other = (Transition) obj;
		if (dest == null) {
			if (other.dest != null)
				return false;
		} else if (!dest.equals(other.dest))
			return false;
		if (src == null) {
			if (other.src != null)
				return false;
		} else if (!src.equals(other.src))
			return false;
		if (act == null) {
			if (other.act != null)
				return false;
		} else if (!act.equals(other.act))
			return false;
		return true;
	}

	public Vertex getDest() {
		return dest;
	}

	public Vertex getSrc() {
		return src;
	}

	public Action getAction() {
		return act;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((dest == null) ? 0 : dest.hashCode());
		result = prime * result + ((src == null) ? 0 : src.hashCode());
		result = prime * result
				+ ((act == null) ? 0 : act.hashCode());
		return result;
	}

	public String toString() {
		return src + " -> " + dest + "[" + act + "] ";
	}

	public boolean isSelfLoop() {
		return src.equals(dest);
	}

	public int getPid() {
		return act.getPid();
	}

	public int compareTo(Transition o) {		
		return act.compareTo(o.act);
	}
}
