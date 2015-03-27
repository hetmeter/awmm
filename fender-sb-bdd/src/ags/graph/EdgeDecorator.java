package ags.graph;

import java.util.Set;

import org.jgrapht.Graph;

import fsb.explore.State;
import fsb.explore.Action;
import fsb.explore.Transition;
import fsb.explore.Vertex;

public class EdgeDecorator {

	Set<Transition> cutEdges;
	Transition stepEdge;

	public EdgeDecorator(Set<Transition> cutEdges, Transition stepEdge) {
		this.cutEdges = cutEdges;
		this.stepEdge = stepEdge;
	}

	public String getDecoration(Graph<Vertex, Transition> g, Transition e) {
		State src = g.getEdgeSource(e).getState();
		State dest = g.getEdgeTarget(e).getState();
		StringBuffer decoration = new StringBuffer();
		Action act = e.getAction();

		if (!src.isErr() && dest.isErr()) {
			decoration.append(" color = \"red\"  ");
		} 
			
		if (act != null) {
			decoration.append("label = \"" + act.toString() + "\"");
		}

		if (!cutEdges.isEmpty())
			if (cutEdges.contains(e)
					&& ((stepEdge == null || !stepEdge.equals(e)))) {
				decoration.append(" style = \"dotted\"  ");
			} else if (stepEdge.equals(e)) {
				decoration.append(" style = \"dashed\"  ");
			}

		return decoration.toString();
	}
}
