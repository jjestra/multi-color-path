#ifndef RANDOM_GRAPHS_H
#define RANDOM_GRAPHS_H

#include "ColoredGraph.h"
#include "Utils.h"
#include "WriteLatex.h"
#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/simple_graph_alg.h>
#include "UnitDiskGraph.h"

extern EdgeColoredGraph::DistributionType gDistType;
extern int gLimitPerEdge;

class RandomGraph {
protected:
	int m_numColors;
	EdgeColoredGraph m_Graph;
	GraphAttributes m_ga;
	// space between adjacent nodes in graph
	int m_spacing;

	RandomGraph(int numColors) : m_numColors(numColors),
							m_ga(m_Graph.graph(), GraphAttributes::nodeGraphics | GraphAttributes::nodeLabel), 
							m_spacing(15) { }
public:
	GraphAttributes& attributes() { return m_ga; }
	EdgeColoredGraph& coloredGraph() { return m_Graph; }
	Graph& graph() { return m_Graph.graph(); }
	virtual string getName() = 0;
	void drawLatex(WriteToLatex& wl, std::string style = "") {
		Utils::alignGraphToGrid(m_ga, m_spacing, m_Graph.src(), m_Graph.dst());
		std::stringstream ss;
		Graph& G = m_Graph.graph();
		ss << getName() << " : G=(" << G.numberOfNodes() << ", " << G.numberOfEdges() << " ): Colors: " << m_numColors; 
		wl.drawGraph(m_ga);
	}
};

struct CallBack {
	int numNodes;
	CallBack(int n) : numNodes(n) {}
	double prob(node u, node v) {
		return (0.5 * log(numNodes)) /numNodes;
	}
};

class RandomColoredGraph : public RandomGraph {
public: 
	RandomColoredGraph(int numNodes, int numColors) : RandomGraph(numColors) {
		Graph& G = m_Graph.graph();
		for (int i = 0; i < numNodes; i++)
			G.newNode();
		CallBack cb(numNodes);
		std::function<double(node, node)> f = std::bind(&CallBack::prob, &cb, std::placeholders::_1, std::placeholders::_2);
		randomEdgesGraph(G, f);
		//randomDigraph(G, numNodes, prob);
		cout << "|G| = " << G.numberOfEdges() << endl;
		//makeSimple(G);
		//randomSimpleGraph(G, numNodes, numEdges);
		//completeGraph(G, numNodes);
		assert(G.numberOfNodes() == numNodes);
		int numEdges = G.numberOfEdges();
		//m_Graph.setRandomColors(numColors);
		m_Graph.assignColorsOnEdges(numColors, EdgeColoredGraph::cYuanEtAl, gLimitPerEdge);
		
		
		// Plant a path of color c
		/*
		m_Graph.assignColorsOnEdges(numColors-1, EdgeColoredGraph::cYuanEtAl, gLimitPerEdge);
		int pathLength = 10;
		int c = numColors-1;
		vector<node> pathNodes;
		for (node v : G.nodes) {
			pathNodes.push_back(v);
			if (--pathLength <= 0)
				break;
		}
		EdgeArray<ColorSet>& C = m_Graph.colors();
		for (int i = 1; i < pathNodes.size(); i++) {
			edge e = G.newEdge(pathNodes[i], pathNodes[i-1]);
			C[e].insert(c);
		}
		edge se = G.newEdge(m_Graph.src(), pathNodes.front());
		edge te = G.newEdge(m_Graph.dst(), pathNodes.back());
		C[se].insert(c);
		C[te].insert(c);
		m_Graph.numColors(numColors);
		*/
	}
	virtual string getName() { return "RandomColoredGraph"; }
};

// Unit Disk Graphs have large diameters
class RandomColoredUnitDiskGraph : public RandomGraph {
public: 
	RandomColoredUnitDiskGraph(int numNodes, int numColors, WriteToLatex* wl) : RandomGraph(numColors) {
		Graph& G = m_Graph.graph();
		UnitDiskGraph udg(G, numNodes, wl);
		m_Graph.assignColorsOnEdges(numColors, gDistType, gLimitPerEdge);
	}
	virtual string getName() { return "RandomColoredGraphUDG"; }
};

class RandomColoredUnionGraph : public RandomGraph {
public: 
	RandomColoredUnionGraph(int numNodes, int numColors, int numDiamonds = 20) : RandomGraph(numColors) {
		// Each diamond is a small graph with numNodes/numDiamonds nodes, and randomly assigned colors.
		// the returned graph G is a concatenation of all those diamonds.
		Graph diamond;
		int numNodesPerDiamond = numNodes/numDiamonds;
		double prob = (10*log(numNodes)) /numNodes;
		randomDigraph(diamond, numNodesPerDiamond, prob);
		//int numEdgesPerDiamond = numNodesPerDiamond * 4;
		//int numEdgesPerDiamond = numNodesPerDiamond * numNodesPerDiamond * 0.2;
		//randomSimpleConnectedGraph(diamond, numNodesPerDiamond, numEdgesPerDiamond);
		node sd, td;
		int dist = Utils::findFarthestNodePairs(diamond, sd, td);

		Graph& G = m_Graph.graph();
		// Create graph concatenating diamonds
		node src = G.newNode();
		node dst = G.newNode();

		node connector = src;
		for (int i = 0; i < numDiamonds; i++) {
			node tmpConn = connector;
			NodeArray<node> nmap(diamond);
			for (node v : diamond.nodes) {
				node w = G.newNode();
				nmap[v] = w;
				if (v == sd)
					G.newEdge(tmpConn, w);
				if (v == td)
					connector = w;
			}
			for (edge e : diamond.edges)
				G.newEdge(nmap[e->source()], nmap[e->target()]);
		}
		G.newEdge(connector, dst);
		m_Graph.assignColorsOnEdges(numColors, gDistType, gLimitPerEdge);
	}
	virtual string getName() { return "RandomColoredGraphUnion"; }
};

class ColoredRoadNetwork : public RandomGraph {
public: 
	ColoredRoadNetwork(int numColors) : RandomGraph(numColors) {
		Graph& G = m_Graph.graph();
		std::map<int, node> idToNode;
		std::ifstream	is("road-networks/roadNet-CA.txt");
		string line;
		while (std::getline(is, line)) {
			int idx = 0;
			while (line[idx] == ' ') idx++;
			if (line[idx] == '#')
				continue; // a comment
			std::stringstream ss(line);
			int srcId, dstId;
			ss >> srcId >> dstId;
			if (idToNode.find(srcId) == idToNode.end())
				idToNode[srcId] = G.newNode(); 
			if (idToNode.find(dstId) == idToNode.end())
				idToNode[dstId] = G.newNode();
			G.newEdge(idToNode[srcId], idToNode[dstId]);
		}
		makeParallelFreeUndirected(G);
		cout << " G has : " << G.numberOfNodes() << " nodes and " << G.numberOfEdges() << " edges." << endl;
		cout << "Constructed road-newtork graph! Now assigning colors." << endl;
		m_Graph.assignColorsOnEdges(numColors, gDistType, gLimitPerEdge);
		cout << "..All done!." << endl;
	}

	virtual string getName() { return "CA-RoadNetwork"; }
};

#endif
