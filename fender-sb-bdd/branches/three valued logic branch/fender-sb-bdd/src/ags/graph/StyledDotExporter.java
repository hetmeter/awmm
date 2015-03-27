package ags.graph;

import java.io.PrintWriter;
import java.io.Writer;

import org.jgrapht.DirectedGraph;
import org.jgrapht.Graph;
import org.jgrapht.ext.EdgeNameProvider;
import org.jgrapht.ext.IntegerNameProvider;
import org.jgrapht.ext.VertexNameProvider;

import fsb.explore.Transition;
import fsb.explore.Vertex;

public class StyledDotExporter<V, E> {

	// ~ Instance fields
	// --------------------------------------------------------

	private VertexNameProvider<V> vertexIDProvider;
	private VertexNameProvider<V> vertexLabelProvider;
	private EdgeNameProvider<E> edgeLabelProvider;
	private Decorator<V> vertexDecorator;
	private EdgeDecorator edgeDecorator;

	// ~ Constructors
	// -----------------------------------------------------------

	/**
	 * Constructs a new DOTExporter object with an integer name provider for the
	 * vertex IDs and null providers for the vertex and edge labels.
	 */
	public StyledDotExporter() {
		this(new IntegerNameProvider<V>(), null, null, null,null);
	}

	/**
	 * Constructs a new DOTExporter object with the given ID and label
	 * providers.
	 * 
	 * @param vertexIDProvider
	 *            for generating vertex IDs. Must not be null.
	 * @param vertexLabelProvider
	 *            for generating vertex labels. If null, vertex labels will not
	 *            be written to the file.
	 * @param edgeLabelProvider
	 *            for generating edge labels. If null, edge labels will not be
	 *            written to the file.
	 */
	public StyledDotExporter(VertexNameProvider<V> vertexIDProvider,
			VertexNameProvider<V> vertexLabelProvider,
			EdgeNameProvider<E> edgeLabelProvider,
			Decorator<V> vertexDecorator,
			EdgeDecorator edgeDecorator) {
		this.vertexIDProvider = vertexIDProvider;
		this.vertexLabelProvider = vertexLabelProvider;
		this.edgeLabelProvider = edgeLabelProvider;
		this.vertexDecorator = vertexDecorator;
		this.edgeDecorator = edgeDecorator;
	}

	// ~ Methods
	// ----------------------------------------------------------------

	/**
	 * Exports a graph into a plain text file in DOT format.
	 * 
	 * @param writer
	 *            the writer to which the graph to be exported
	 * @param g
	 *            the graph to be exported
	 */
	@SuppressWarnings("unchecked")
	public void export(Writer writer, Graph<V, E> g) {
		PrintWriter out = new PrintWriter(writer);
		String indent = "  ";
		String connector;

		if (g instanceof DirectedGraph) {
			out.println("digraph G {");
			connector = " -> ";
		} else {
			out.println("graph G {");
			connector = " -- ";
		}

		for (V v : g.vertexSet()) {
			out.print(indent + vertexIDProvider.getVertexName(v));
			
			if (vertexLabelProvider != null) {
				out
						.print(" [label = \""
								+ vertexLabelProvider.getVertexName(v) + "\"");
				if (vertexDecorator != null) {
					out.print(" " + vertexDecorator.getDecoration(v));
				}
				out.print("]");
			}

			out.println(";");
		}

		for (E e : g.edgeSet()) {
			String source = vertexIDProvider.getVertexName(g.getEdgeSource(e));
			String target = vertexIDProvider.getVertexName(g.getEdgeTarget(e));

			out.print(indent + source + connector + target);

			if (edgeLabelProvider != null) {
				//out.print(" [label = \"" + edgeLabelProvider.getEdgeName(e) + "\"");
				out.print(" [");
				if (edgeDecorator != null) {
					out.print(" " + edgeDecorator.getDecoration((Graph<Vertex,Transition>)g,(Transition)e));
				}
				out.print("]");
			}

			out.println(";");
		}

		out.println("}");

		out.flush();
	}
}
