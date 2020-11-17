#ifndef TOPOLOGY_ZOO_H
#define TOPOLOGY_ZOO_H

#include "ColoredGraph.h"
#include "WriteLatex.h"
#include <dirent.h>


// Each city is represented by its label
struct UniqueCityID {
	double x;
	double y;
	string label;
	UniqueCityID(double ax, double ay, string al) : x(ax), y(ay), label(al) {}
	friend bool operator== (const UniqueCityID& a, const UniqueCityID& b) {
		return (a.x == b.x && a.y == b.y && a.label == b.label);
	}
	friend bool operator< (const UniqueCityID& a, const UniqueCityID& b) {
		return a.label < b.label;
		/*if (a.x == b.x) 
			if (a.y == b.y)
				return a.label < b.label;
			else
				return a.y < b.y;
		else 
			return a.x < b.x;
			*/
	}
};

class TopologyZoo {
	int m_NumColors;
	EdgeColoredGraph m_Graph;
	GraphAttributes m_ga;
	node m_src;
	node m_dst;
	std::map<UniqueCityID, node> m_cityToNode;
	vector<string> m_colorName;

public:
	// Chain graph with n links and k nodes per link
	TopologyZoo() : m_NumColors(0) {
		DIR *dir = opendir("topology-zoo");
		Graph& G = m_Graph.graph();
		if (!dir)
		 cout << "Error opening directory: topology-zoo" << endl;
		struct dirent *ent;
		int numFiles = 10000;
		while ((ent = readdir (dir)) != NULL) {
			string fname(ent->d_name);
			fname = "topology-zoo/" + fname;
			if (fname.find("gml") != string::npos) {
				readGML(fname);
				if (--numFiles <= 0)
					break;
			}
		}
		closedir(dir);
		//m_Graph.printColorInfo(m_colorName);
		m_Graph.numColors(m_NumColors);
		//makeBidirectional();
		//std::ofstream os("neeraj.dot");
		//GraphIO::writeDOT(G, os);
		//cout << "Nodes:" << G.numberOfNodes() << " edges: " << G.numberOfEdges() << endl;
		EdgeArray<ColorSet>& C = m_Graph.colors();
		int totalOcc = 0;
		for (edge e : G.edges)
			totalOcc += C[e].size();
		cout << "Average: " << totalOcc * 1.0 / G.numberOfEdges() << " colors" << endl;
		cout << "Diameter: " << Utils::findFarthestNodePairs(m_Graph.graph(), m_src, m_dst) << endl;
		m_Graph.src() = m_src;
		m_Graph.dst() = m_dst;
		cout << "Connected? : " << isConnected(G) << endl;
		cout << "is parallel free? : " << isParallelFree(G) << endl; 
		cout << "is loop free? : " << isLoopFree(G) << endl; 
		//exit(0);
	}

	node src() { return m_src; }
	node dst() { return m_dst; }
	GraphAttributes& attributes() { return m_ga; }
	int numColors() { return m_NumColors; }

	EdgeColoredGraph& coloredGraph() { return m_Graph; }
	Graph& graph() { return m_Graph.graph(); }

	void readGML(string fname);
	void makeBidirectional() {
		Graph& G = m_Graph.graph();
		EdgeArray<ColorSet>& C = m_Graph.colors();
		vector<edge> allEdges;
		for (edge e : G.edges) 
			allEdges.push_back(e);
		for (edge e : allEdges) {
			node u = e->source();
			node v = e->target();
			edge ep = G.newEdge(v, u);
			C[ep] = C[e];
		}
	}
};

void TopologyZoo::readGML(string fname) {
	// Each network is a unique color.
	//fname = "topology-zoo/Uninett2010.gml";
	Graph& G = m_Graph.graph();
	EdgeArray<ColorSet>& C = m_Graph.colors();
	Graph g;
	GraphAttributes ga(g, GraphAttributes::nodeLabel | GraphAttributes::nodeGraphics);
	cout << "Reading file : " << fname << endl;
	std::ifstream is(fname);
	bool status = GraphIO::readGML(ga, g, is);
	makeParallelFree(g);
	cout << "Graph g: " << g.numberOfNodes() << " nodes and " << g.numberOfEdges() << " edges " << endl;
	NodeArray<node> gToG(g);

	for (node v : g.nodes) {
		UniqueCityID c(ga.x(v), ga.y(v), ga.label(v));
		// no latitude or longitude provided, cant reuse this vertex.
		if (c.x == 0 && c.y == 0)
			c.label = fname + std::to_string(v->index());
		
		// look for cities we have already mapped..
		if (m_cityToNode.find(c) == m_cityToNode.end())
			m_cityToNode[c] = G.newNode();
		else 
			cout << c.label << " already exists in G. Reuse for connections!" << endl;

		gToG[v] = m_cityToNode[c];
	}
	bool subdivide = false;
	for (edge e : g.edges) {
		//cout << "Creating edges for " << e << " : ";
		node u = gToG[e->source()];
		node v = gToG[e->target()];
		if (u == v) {
			cout << "Loop Edge?!" << endl;
			continue;
		}
		edge ep = G.searchEdge(u, v);
		if (ep) {
			//cout << "Parallel edge!" << endl;
			if (subdivide) {
				node ve = G.newNode();
				edge e1 = G.newEdge(u, ve);
				edge e2 = G.newEdge(ve, v);
				C[e1].insert(m_NumColors);
				C[e2].insert(m_NumColors);
			}
			else 
				C[ep].insert(m_NumColors);
		}
		else {
			//cout << "Yes!" << endl;
			ep = G.newEdge(u, v);
			C[ep].insert(m_NumColors);
		}
	}
	m_colorName.push_back(fname);
	m_NumColors++;
	cout << "G has nodes: " << G.numberOfNodes() << " edges: " << G.numberOfEdges() << endl;
}
#endif
