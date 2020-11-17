#ifndef K_CHAIN_GRAPH_H
#define K_CHAIN_GRAPH_H

#include "ColoredGraph.h"
#include "WriteLatex.h"

/////////////////////////////////////////////////////
//       /-- 1 --- 2 --- 3 ---\
// src__/   K2,2     k2,2      \__ dst
//      \                      /
//       \-- 1 --- 2 --- 3 ---/
/////////////////////////////////////////////////////

extern EdgeColoredGraph::DistributionType gDistType;
extern int gLimitPerEdge;

class kChainGraph {
	// Nodes are arranged in k * n\k grid
	// Number of colors is also k
	int m_NumColors;
	std::vector<std::vector<node>> m_Grid;
	EdgeColoredGraph m_Graph;
	GraphAttributes m_ga;
	// space between adjacent nodes in graph
	int m_spacing;
	// Begining and end of k chain
	node m_src;
	node m_dst;

public:
	// Chain graph with n links and k nodes per link
	kChainGraph(int k, int n, int nc = 0) : m_NumColors(nc), m_Grid(k), 
					m_ga(m_Graph.graph(), GraphAttributes::nodeLabel | GraphAttributes::edgeLabel | GraphAttributes::nodeGraphics),
					m_spacing(15)
	{
		assert(k > 0 && n > 0);
		if (m_NumColors == 0)
			m_NumColors = k;
		m_ga.directed() = true;
		Graph& G = m_Graph.graph();
		m_src = G.newNode();
		for (int i = 0; i < k; i++)
			for (int j = 0; j < n; j++)
				m_Grid[i].push_back(G.newNode());
		m_dst = G.newNode();
		addColoredEdges();
		setAttributes();
		m_Graph.numColors(m_NumColors);
		//shuffleAdjEntries();
	}

	node src() { return m_src; }
	node dst() { return m_dst; }

	EdgeColoredGraph& coloredGraph() { return m_Graph; }
	Graph& graph() { return m_Graph.graph(); }
	void setAttributes();
	void shuffleAdjEntries();
	GraphAttributes& attributes() { return m_ga; }
	// basically two consecutive columns are fully-connected
	// by k*k edges each k edge from one vertex has unique color.
	void addColoredEdges();
	
	// TODO: Save this dataset into some graph format that can be read.
	// All drawings will be in Latex..
	void write(std::string f, GraphIO::AttrWriterFunc wf) {
		m_Graph.write(f, wf);
	}

	void drawLatex(WriteToLatex& wl, std::string style = "") {
		Color::Name cList[5] = {Color::Name::Red, Color::Name::Green, Color::Name::Blue, Color::Name::Orange, Color::Name::Gray };
		int k = m_Grid.size();
		int n = m_Grid[0].size();
		std::stringstream ss;
		ss << "kChainGraph: " << k << "x" << n << ": Colors: " << k; 
		wl.initPage(ss.str());
		Graph& G = m_Graph.graph();
		for (node v : G.nodes)
			wl.drawNode(m_ga, v);
		EdgeArray<ColorSet>& C = m_Graph.colors();
		for (edge e : G.edges) {
			//wl.drawEdge(m_ga, e, "gray", 1);
			Color c(Color::Name::Black);
			if (C[e].size() > 0) {
				int idx = (*C[e].begin()) % 5;
				c = cList[idx];
			}
			wl.drawColoredEdge(m_ga, e, c, 1, style);
		}
	}
};

void kChainGraph::shuffleAdjEntries() {
	Graph& G = m_Graph.graph();
	for (node v : G.nodes) {
		List<adjEntry> adjList;
		v->allAdjEntries(adjList);
		adjList.permute();
		G.sort(v, adjList);
	}
}

void kChainGraph::addColoredEdges() {
	Graph& G = m_Graph.graph();
	int k = m_Grid.size();
	int n = m_Grid[0].size();
	// Connect src to column 0, dst to col n, with uncolored edges
	for (int i = 0; i < k; i++) {
		G.newEdge(m_src, m_Grid[i][0]);
		G.newEdge(m_Grid[i][n-1], m_dst);
	}

	// Each vertex has exactly k neighbors, we connect them 
	// using unique colors but in random order.

	srand (time(NULL));
	vector<int> nbrIndices(k);
	for (int i = 0; i < k; i++)
		nbrIndices[i] = i;

	EdgeArray<ColorSet>& C = m_Graph.colors();
	for (int j = 0; j < n-1; j++)
		for (int i = 0; i < k; i++) {
			node u = m_Grid[i][j];
			//std::random_shuffle(nbrIndices.begin(), nbrIndices.end());
			int colorIdx = 0;
			// Create complete bipartite graph between two neighboring columns
			for (int idx : nbrIndices) {
				node v = m_Grid[idx][j+1]; // next column vertex
				edge e = G.newEdge(u, v);
				//C[e].insert(colorIdx++);
			}
		}
	
	//m_Graph.setRandomColors(m_NumColors);
	m_Graph.assignColorsOnEdges(m_NumColors, gDistType, gLimitPerEdge);
}		

void kChainGraph::setAttributes() {
	int k = m_Grid.size();
	int n = m_Grid[0].size();

	m_ga.x(m_src) = 0;
	m_ga.x(m_dst) = (n+1) * m_spacing;
	m_ga.y(m_src) = m_ga.y(m_dst) = (k/2 + 1) * m_spacing;
	m_ga.label(m_src) = std::to_string(m_src->index());
	m_ga.label(m_dst) = std::to_string(m_dst->index());
	m_ga.width(m_src) =  m_ga.width(m_dst) = 1;
	m_ga.height(m_src) =  m_ga.height(m_dst) = 1;

	for (int i = 0; i < m_Grid.size(); i++) 
		for (int j = 0; j < m_Grid[i].size(); j++) {
			node v = m_Grid[i][j];
			m_ga.x(v) = m_spacing * (j+1);
			m_ga.y(v) = m_spacing * (i+1);
			m_ga.width(v) = 0.8;
			m_ga.height(v) = 0.4;
			m_ga.label(v) =  std::to_string(v->index());
		}
	
	EdgeArray<ColorSet>& C = m_Graph.colors();
	for (edge e : m_Graph.graph().edges)
		m_ga.label(e) = Utils::colorSetToString(C[e]);
}

#endif
