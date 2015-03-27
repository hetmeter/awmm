package ags.graph;

import fsb.explore.State;
import fsb.explore.Vertex;

public class StateDecorator implements Decorator<Vertex> {
	private static final String EMPTY_DECORATION = "";
	private State initial;

	public StateDecorator(Vertex initial) {
		this.initial = initial.getState();
	}

	public String getDecoration(Vertex v) {
		String result = EMPTY_DECORATION;
		State s = v.getState();
		if (initial != null && initial.equals(s)) {
			result+= "color= \"blue\" ";
		}
		else if (s.isErr()) {
			result+= "color= \"red\" ";
		}	
		else if (s.isFinal()) {
			result+= "color= \"violet\" ";
		}
		return result;
	}
}
