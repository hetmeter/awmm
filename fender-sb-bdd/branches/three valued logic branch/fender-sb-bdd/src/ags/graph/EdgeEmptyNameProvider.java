package ags.graph;

import org.jgrapht.ext.EdgeNameProvider;

 
public class EdgeEmptyNameProvider<T> implements EdgeNameProvider<T> {

	@Override
	public String getEdgeName(T e) {
		return "";
	}

}
