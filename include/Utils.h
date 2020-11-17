#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <ogdf/graphalg/Dijkstra.h>

namespace Utils {

void shuffleAdjEntries(Graph& G) {
	for (node v : G.nodes) {
		List<adjEntry> adjList;
		v->allAdjEntries(adjList);
		adjList.permute();
		G.sort(v, adjList);
	}
}

void checkIfFormsPath(vector<edge>& path, node src, node dst, vector<edge>& useful) {
	std::set<edge> pathEdges(path.begin(), path.end());
	node curr = src;
	while (curr != dst) {
		edge ep = 0;
		for (edge e : pathEdges) {
			if (curr == e->source()) {
				ep = e;
				break;
			}
		}
		assert(ep);
		cout << ep << " --> ";
		pathEdges.erase(ep);
		curr = ep->target();
		useful.push_back(ep);
	}
	for (edge e : pathEdges) {
		cout << "Uh-oh: PROBLEM " << e << endl;
	}
	cout << " Path test: OK" << endl;
}

// Given `pred` of all vertices, get a list of all edges in the path
void getEdgesInPath(node s, node t, NodeArray<edge> pred, vector<edge>& path) {
	node curNode = t;
	while (curNode != s) {
		edge e = pred[curNode];
		if (!e)
			break;
		path.push_back(e);
		curNode = e->opposite(curNode);
	}
}

node findNodeAtHopDistance(node v, int d) {
	int hopCount = 0;
	while (hopCount < d) {
		v = v->firstAdj()->twinNode();
		hopCount++;
	}
	return v;
}

// Get colorset corresponding to a set of edges
ColorSet getColorSet(EdgeColoredGraph& cg, vector<edge>& edgeList) {
	EdgeArray<ColorSet>& cs = cg.colors();
	ColorSet rVal;
	for (edge e : edgeList)
		rVal.insert(cs[e].begin(), cs[e].end());
	return rVal;
}


// try to see if t is reachable from s in G (Undirected)
bool isReachable(Graph&G, node s, node t) {
	// Use BFS
	Queue<node> q;
	NodeArray<bool> visited(G, false);
	q.append(s);
	visited[s] = true;
	while (!q.empty()) {
		node u = q.pop();
		if (u == t)
			return true;
		for (adjEntry adj : u->adjEntries) {
			edge e = adj->theEdge();
			node v = e->opposite(u);
			if (visited[v] == false) {
				q.append(v);
				visited[v] = true;
			}
		}
	}
	return false;
}

// try to see if s, t lie in same component 'comp'
// Discard all edges that dont lie in 'comp'
// store them in toDelete
// Use BFS
bool isReachableDiscard(Graph&G, node s, node t, std::set<edge>& toDelete) {
	Queue<node> q;
	NodeArray<bool> visited(G, false);
	q.append(s);
	visited[s] = true;
	while (!q.empty()) {
		node u = q.pop();
		for (adjEntry adj : u->adjEntries) {
			edge e = adj->theEdge();
			node v = e->opposite(u);
			if (visited[v] == false) {
				q.append(v);
				visited[v] = true;
			}
		}
	}

	// return if t is unvisited
	if (!visited[t]) 
		return false;

	//If t is visited, discard all edges that are not in same component as s, t
	for (node v : G.nodes) {
		if (!visited[v])
			for(adjEntry adj : v->adjEntries)
				toDelete.insert(adj->theEdge());
	}
	return true;
}

bool alignGraphToGrid(GraphAttributes& ga, int spacing, node src = NULL, node dst = NULL) {
	const Graph& G = ga.constGraph();
	int size = 10; // embed nodes into a 10 x j grid for drawing
	ga.label(src) = std::to_string(src->index()); 
	ga.label(dst) = std::to_string(dst->index()); 
	ga.x(src) = 0;
	ga.y(src) = ga.y(dst) = (size/2 + 1) * spacing;
	node curr = G.firstNode()->succ();
	int x = 1;
	int y = 0;
	for (node v : G.nodes) {
		if (v == src || v == dst)
			continue;
		if (y == size) {
			x++;
			y = 0;
		}
		ga.x(v) = x * spacing;
		ga.y(v) = y * spacing;
		y++;
		ga.label(v) = std::to_string(v->index()); 
	}
	if (y != 0)
		x++;
	ga.x(dst) = x * spacing;
}

} // end namespace Utils

#endif // UTILS_H
