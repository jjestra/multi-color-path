#ifndef CUSTOM_COLORED_GRAPHS_H
#define CUSTOM_COLORED_GRAPHS_H

#include "ColoredGraph.h"
#include "Utils.h"
#include "WriteLatex.h"
#include <ogdf/basic/GraphAttributes.h>

class CustomColoredGraph : public EdgeColoredGraph {
	GraphAttributes m_ga;
	// mapping of user ids to nodes
	std::map<int, node> m_idToNodeMap;
	// space between adjacent nodes in graph drawing
	int m_spacing;

	node addNodeIfNeeded(int id) {
		auto iter = m_idToNodeMap.find(id);
		if (iter != m_idToNodeMap.end()) {
			return iter->second;
		}
		node v = m_Graph.newNode(id);
		m_idToNodeMap[id] = v;
		return v; 
	}

public:
	// Constructor: Read graph from file
	CustomColoredGraph(const std::string& filename, int srcId, int dstId) 
							:m_ga(m_Graph, GraphAttributes::nodeGraphics | GraphAttributes::nodeLabel | GraphAttributes::edgeLabel), 
							m_spacing(15) 
	{ 
		readEdgeList(filename);
		m_src = findNode(srcId);
		m_dst = findNode(dstId);
		assert(m_src && m_dst);
	}

	// Some helper functions: 
	GraphAttributes& attributes() { return m_ga; }
	
	node findNode(int id) {
		auto iter = m_idToNodeMap.find(id);
		if (iter == m_idToNodeMap.end()) {
			return nullptr;
		}
		return iter->second; 
	}
	
	
	void drawLatex(WriteToLatex& wl, std::string style = "") {
		Utils::alignGraphToGrid(m_ga, m_spacing, m_src, m_dst);
		std::stringstream ss;
		ss << "CustomGraph : G=(" << m_Graph.numberOfNodes() << ", " << m_Graph.numberOfEdges() << " ): Colors: " << numColors(); 
		wl.initPage(ss.str());
		wl.drawGraph(m_ga);
	}
	
	// Format: (every edge is a line with three values:
	//  srcId, dstId, colorId)
	//  v_1 v_2 c_1
	//  v_3 v_2 c_2
	// ...
	// colors index from 1 to m
	void readEdgeList(std::string fname) {
		std::ifstream is(fname);
		int maxColorId = 0;
		int uId, vId, colorId;
		std::map<int, node> nodes;
		while (is >> uId >> vId >> colorId) {
			node u = addNodeIfNeeded(uId);
			node v = addNodeIfNeeded(vId);
			edge e = m_Graph.newEdge(u, v);
			m_ga.label(e) = std::to_string(colorId);
			m_Colors[e].insert(colorId);
			maxColorId = max(colorId, maxColorId);
		}
		if (!isParallelFree(m_Graph)) {
			std::cout << "Warning: the graph has parallel edges: LP-solver is known to have issues with parallel edges." << endl;
		}
		numColors(maxColorId + 1);
	}
	
};

#endif // CUSTOM_COLORED_GRAPHS_H
