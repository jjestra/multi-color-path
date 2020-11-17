#ifndef DIJKSTRA_BASED_ALG_H
#define DIJKSTRA_BASED_ALG_H
#include "Utils.h"
#include "kChainGraph.h"
#include <iostream>
#include <ogdf/graphalg/Dijkstra.h>
#include <ogdf/basic/simple_graph_alg.h>

class DijkstraBasedAlg {
	EdgeColoredGraph& m_Graph;
	// the list of edges on edges on which color i appears
	vector<vector<int>> m_edgesWithColor;
	WriteToLatex* m_wl;
	GraphAttributes* m_ga;

public:
	DijkstraBasedAlg(EdgeColoredGraph& cg, WriteToLatex* wl = NULL, GraphAttributes* ga = NULL) : 
										m_Graph(cg), m_wl(wl), m_ga(ga) { } 
	
	
	void initArrays(EdgeArray<int>& weights, vector<vector<edge>>& edgesWithColor) { 
		Graph& G = m_Graph.graph();
		EdgeArray<ColorSet>& C = m_Graph.colors();
		for (edge e : G.edges) {
			ColorSet& cse = C[e];
			weights[e] = cse.size();
			for (color j : cse)
				edgesWithColor[j].push_back(e);
		}
	}


	int runDijkstra(EdgeArray<int>& weights, vector<edge>& pathEdges) {
		Dijkstra<int> d;
		NodeArray<edge> pred;
		NodeArray<int> dist;
	  d.call(m_Graph.graph(), weights, m_Graph.src(), pred, dist);
		if (!pred[m_Graph.dst()]) {
			std::cout << "[**** ERROR *****]: node " << m_Graph.dst()  << 
							" is not reachable from " << m_Graph.src() << endl;
		}
		Utils::getEdgesInPath(m_Graph.src(), m_Graph.dst(), pred, pathEdges);
		//cout << " -- Min wt path so far: " << dist[m_Graph.dst()] << endl;
		//cout << "Path: ";
		//for (edge e : pathEdges) cout << e;
		//cout << endl;
		//cout << pathEdges.size() << " edges in path" << endl;
		return Utils::getColorSet(m_Graph, pathEdges).size();
	}
	
	int runDijkstra(vector<edge>& pathEdges) {
		Graph& G = m_Graph.graph();
		EdgeArray<ColorSet>& C = m_Graph.colors();
		EdgeArray<int> weights(G);
		for (edge e : G.edges)
			weights[e] = C[e].size();
		return runDijkstra(weights, pathEdges);
	}

	// Find an s-t path that uses minimum number of links.
	int runBFS(vector<edge>& pathVec);
	
	// Greedy-Select:
	// Using the number of colors returned by Dijkstra
	// as upperbound, greedily discard colors that occurs 
	// maximum times to find a path
	int runDijkstraDiscardMaxOccurrence(vector<edge>& pathVec);

	// THE SPACOA heuristic. Start with a path from Dijkstra
	// and iterate over all colors, select the color that improves the
	// path by maximum amount. Repeat while path can no longer be improved.
	int runDijkstraDiscardMaxGain(vector<edge>& pathVec);

	// A variant of SPACOA where we only consider colors that lie on the path
	// instead of trying out all colors.
	int runDijkstraDiscardMaxGain2(vector<edge>& pathVec);

	// Greedy-Prune-Select:
	// Here removing colors meaning not using a color and therefore dropping
	// all those colored edges out of the graph.
	// So this way, we remove colors as long as s-t are connected
	// and we stop when we can no longer remove colors.
	// Occasionally, call Greedy-Select on modified graph
	int removeUntilConnected(vector<edge>& pathVec);
	
	// Return the algorithm type and description
	enum AlgType {DIJKSTRA = 0, DIJKSTRA_MAX_OCC, DIJKSTRA_MAX_GAIN, DIJKSTRA_MAX_GAIN2, REMOVE_UNTIL_CONN, NUM_ALGS};

	static string getDescStr(AlgType t) { 
		string descStr[NUM_ALGS] = {"Dijkstra", "SPACOA", "SPACOA-2", "Greedy-Select", "Greedy-Prune-Select"};
		return descStr[t];
	}

	// Handler for running functions
	int runAlgorithm(AlgType t, vector<edge>& pathVec) { 
		typedef int (DijkstraBasedAlg::*ALG_FPTR)(vector<edge>&);
		ALG_FPTR funcs[NUM_ALGS] = {&DijkstraBasedAlg::runDijkstra, 
																&DijkstraBasedAlg::runDijkstraDiscardMaxGain,
																&DijkstraBasedAlg::runDijkstraDiscardMaxGain2,
																&DijkstraBasedAlg::runDijkstraDiscardMaxOccurrence,
																&DijkstraBasedAlg::removeUntilConnected
																};
		return (this->*funcs[t])(pathVec);
	}

};


// The SPACOA heuristic, due to Yuan et al. [https://ieeexplore.ieee.org/document/1498549]
int DijkstraBasedAlg::runDijkstraDiscardMaxGain(vector<edge>& pathVec) {
	cout << "Running SPACOA heuristic." << endl;
	//return -1;
	Graph& G = m_Graph.graph();
	EdgeArray<ColorSet>& C = m_Graph.colors();
	EdgeArray<int> weights(G);
	vector<vector<edge>> edgesWithColor(m_Graph.numColors());
	initArrays(weights, edgesWithColor);
	int numColors = m_Graph.numColors();
	
	// Step 1. Run Dijkstra to find an initial path p
	runDijkstra(weights, pathVec);
	ColorSet cs = Utils::getColorSet(m_Graph, pathVec);
	int ub = cs.size();
	// Initially all colors are up for selection
	ColorSet availableColors;
	for (int i = 0; i < numColors; i++)
		availableColors.insert(i);

	bool canDiscard = true;
	while (canDiscard) {
		double currGain = 0;
		ColorSet::iterator itToDiscard = availableColors.end();
		for (auto it = availableColors.begin(); it != availableColors.end(); ++it) {
			// Try selecting color *it and measure *its impact
			
			if (edgesWithColor[*it].size() == 0) {
				itToDiscard = it;
				break;
			}
			EdgeArray<int> temp(weights);
			for (edge e : edgesWithColor[*it])
					temp[e] = temp[e] - 1;
			// Run dijkstra on the remaining graph.
			vector<edge> newPath;
			int colorCount = runDijkstra(temp, newPath); // includes discarded colors
			double gain = ub - colorCount;
			//cout << "  Considering color " << *it << " that occurs on " << edgesWithColor[*it].size() 
			//			<< " edges, giving a gain: of " << gain << endl;
			if (gain > currGain) {
				currGain = gain;
				itToDiscard = it;
			}
		}
		canDiscard = (itToDiscard != availableColors.end());
		if (canDiscard) {
			int toDiscardIdx = *itToDiscard;
			availableColors.erase(itToDiscard);
			if (edgesWithColor[toDiscardIdx].size() > 0) {
				// Selecting a color = discarding it from graph G
				cout << " - Selecting color " <<  toDiscardIdx <<  " giving a gain of " << currGain << " occuring on edges: " 
							<< edgesWithColor[toDiscardIdx].size() << endl;
				for (edge e : edgesWithColor[toDiscardIdx])
					weights[e] = weights[e] - 1;
				pathVec.clear();
				ub = runDijkstra(weights, pathVec);
			}
		}
	}
	return Utils::getColorSet(m_Graph, pathVec).size();
}

// Greedy Select: Remove the color that occurs on most edges, compute the path
// after removing them from input, keep the best path.
int DijkstraBasedAlg::runDijkstraDiscardMaxOccurrence(vector<edge>& pathVec) {
	Graph& G = m_Graph.graph();
	EdgeArray<int> weights(G);
	vector<vector<edge>> edgesWithColor(m_Graph.numColors());
	initArrays(weights, edgesWithColor);

	// Get upperbound using a dijkstra run
	runDijkstra(weights, pathVec);
	ColorSet cs = Utils::getColorSet(m_Graph, pathVec);
	// Give some preference to colors already in the path
	EdgeArray<ColorSet>& C = m_Graph.colors();
	int ub = cs.size();

	for (int discards = 1; discards <= ub; discards++) {
		// Discard the color that occurs most times
		int maxColor = 0;
		for (int i = 0; i < edgesWithColor.size(); i++) {
			int currColorPref = edgesWithColor[i].size();
			int maxColorPref = edgesWithColor[maxColor].size();
			if (currColorPref > maxColorPref)
				maxColor = i;
		}
		vector<edge>& ce = edgesWithColor[maxColor];
		for (edge e : ce) {
			weights[e] = weights[e] - 1;
		}

		// Run dijkstra on the remaining graph.
		vector<edge> newPath;
		int colorCount = runDijkstra(weights, newPath);
		//std::cout << "Discarding Color: c" << maxColor << " that appears " << 
		//							ce.size() << " times, gain: " << (ub - colorCount) << std::endl;
		ce.clear();
		if (colorCount < ub) {
			ub = colorCount;
			pathVec = newPath;
		}
	}
	return Utils::getColorSet(m_Graph, pathVec).size();
}


// Improved Greedy-prune!
// Pick color to discard based on occurrence on relevant edges!
// repeat this until s-t pairs are no longer connected.
int DijkstraBasedAlg::removeUntilConnected(vector<edge>& pathVec) {
	cout << "Running Greedy-prune-select now" << endl;
	Graph& G = m_Graph.graph();
	EdgeArray<ColorSet>& C = m_Graph.colors();
	EdgeArray<int> weights(G);
	int numColors = m_Graph.numColors();
	vector<vector<edge>> edgesWithColor(numColors);
	initArrays(weights, edgesWithColor);

	int numEdgesInitial = G.numberOfEdges();

	// Initially all colors are up for selection
	ColorSet availableColors;
	vector<int> colorPreference(numColors, 0);
	for (int i = 0; i < numColors; i++) {
		availableColors.insert(i);
		colorPreference[i] = edgesWithColor[i].size();
	}

	// check when graph size has gone down by atleast 20%
	int thresholdCheck = G.numberOfEdges()*0.25;
	cout << "Discard threshold: " << thresholdCheck << endl;

	node src = m_Graph.src();
	node dst = m_Graph.dst();
	Graph::HiddenEdgeSet he(G);
	EdgeArray<bool> isHidden(G, false);
	int rVal = runDijkstraDiscardMaxOccurrence(pathVec);
	int numEdgesWhenLastChecked = G.numberOfEdges();

	// Pick a color to discard from availableColors
	while (!availableColors.empty()) {
		// find the color that occurs on minimum number of edges

		ColorSet::iterator itToDiscard = availableColors.begin();
		for (auto iter = availableColors.begin(); iter != availableColors.end(); ++iter)
			if (colorPreference[*itToDiscard] > colorPreference[*iter])
				itToDiscard = iter;
		
		int discardIdx = *itToDiscard;
		availableColors.erase(itToDiscard);
		if (colorPreference[discardIdx] == 0) // occurs on no useful edges
			continue;

		//cout << "Color to discard: " << discardIdx << " occurring " 
		//		 << colorPreference[discardIdx] << " times" << endl;
		std::set<edge> edgesHiddenInThisIter;
		for (edge e : edgesWithColor[discardIdx]) {
			if (isHidden[e]) continue;
			he.hide(e);
			isHidden[e] = true;
			edgesHiddenInThisIter.insert(e);
			//cout << "hiding " << e << endl;
		}
		
		// Check reachability after hiding, can give more edges to hide.
		bool reach = Utils::isReachableDiscard(G, src, dst, edgesHiddenInThisIter);
		// if reachable discard edges
		if (reach) {
			for (edge e : edgesHiddenInThisIter) {
				for (int c : C[e]) {
					// c has not been discarded yet.
					if (colorPreference[c] > 0)
						colorPreference[c]--;
				}
				if (isHidden[e]) continue;
				he.hide(e);
				isHidden[e] = true;
			}
		}
		else {
			// Restore edges hidden in this iteration
			for (edge e : edgesHiddenInThisIter) {
				he.restore(e);
				isHidden[e] = false;
				//cout << "Un-hiding " << e << endl;
			}
			//continue; // no need to run greedy-select
		}
		
		// Decide whether or not to run Greedy-Select, always run on last iteration!
		int delta = numEdgesWhenLastChecked - G.numberOfEdges();
		if (delta >= thresholdCheck || availableColors.empty()) {
			vector<edge> tempVec;
			int tmpVal = runDijkstraDiscardMaxOccurrence(tempVec);
			cout << " |E| =  " << G.numberOfEdges();
			cout << " [-- Greedy-Select result: " << tmpVal << " size: " << tempVec.size() << endl;
			if (rVal > tmpVal || (rVal == tmpVal && pathVec.size() > tempVec.size())) {
				rVal = tmpVal;
				pathVec = tempVec;
			}
			numEdgesWhenLastChecked = G.numberOfEdges();
		}
	}
	he.restore();
	assert(G.numberOfEdges() == numEdgesInitial);
	return rVal;
}

// A variation of SPACOA where we only try colors from the edges that lie
// on the path. Not included in comparision.
// Observe that this SPACOA variant can discard multiple colors in one iteration
// because it tries ColorSet on all edges of the path as candidates.
int DijkstraBasedAlg::runDijkstraDiscardMaxGain2(vector<edge>& pathVec) {
	//return -1;
	cout << "Running SPACOA-variant now" << endl;
	Graph& G = m_Graph.graph();
	EdgeArray<ColorSet>& C = m_Graph.colors();
	EdgeArray<int> weights(G);
	vector<vector<edge>> edgesWithColor(m_Graph.numColors());
	initArrays(weights, edgesWithColor);
	runDijkstra(weights, pathVec);

	ColorSet cs = Utils::getColorSet(m_Graph, pathVec);
	int ub = cs.size();

	bool canDiscard = true;
	while (canDiscard) {
		edge toDiscard = NULL;
		double currGain = 0;
		for (edge e : pathVec) {
			EdgeArray<int> temp(weights);
			ColorSet& cse = C[e];
			// Discard all colors in C[e] and see if path improved.
			for (int c : cse) {
				// Discard c
				for (edge e : edgesWithColor[c])
					temp[e] = temp[e] - 1;
			}
			// Run dijkstra on the remaining graph.
			vector<edge> newPath;
			int colorCount = runDijkstra(temp, newPath); // includes discarded colors
			double gain = ub - colorCount;
			if (gain > currGain) {
				currGain = gain;
				toDiscard = e;
			}
		}
		canDiscard = (currGain > 0);
		if (canDiscard) {
			cout << " - Selecting colors: " << Utils::colorSetToString(C[toDiscard]) << " giving a gain of: " << currGain << endl;
			for (int c : C[toDiscard]) {
				for (edge e : edgesWithColor[c])
					weights[e] = weights[e] - 1;
				edgesWithColor[c].clear();
			}
			pathVec.clear();
			//cout << " -- Before : " << ub;
			ub = runDijkstra(weights, pathVec);
			//cout << " After : " << ub << endl;
		}
	}
	return Utils::getColorSet(m_Graph, pathVec).size();
}
#endif
