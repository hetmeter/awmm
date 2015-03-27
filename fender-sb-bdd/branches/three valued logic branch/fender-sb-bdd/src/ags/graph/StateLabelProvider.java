package ags.graph;

import org.jgrapht.ext.VertexNameProvider;
import fsb.explore.Vertex;

public class StateLabelProvider implements VertexNameProvider<Vertex> {

	//private Map<State, String> stmtLabels;

	public StateLabelProvider() {
		//this.stmtLabels = stmtLab;
	}

	@Override
	public String getVertexName(Vertex s) {
		//return stmtLabels.get(s);
		return s.toString();
	}

}
