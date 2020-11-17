#ifndef COLORED_GRAPH_H
#define COLORED_GRAPH_H
#include <set>
#include <random>
#include <iomanip>
#include <sstream>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <cassert>

using namespace ogdf;
typedef int color;
typedef std::set<color> ColorSet;
typedef std::set<node> NodeSet;
typedef std::set<edge> EdgeSet;
using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::vector;

//-----------------------------
// Set of Utility functions
//-----------------------------
namespace Utils {

static string colorSetToString(ColorSet cs) {
	std::stringstream ss;
	for (int c : cs)
				ss << "c" << c << ",";
	std::string rVal = ss.str();
	return rVal.substr(0, rVal.size() - 1);
}

void runBFSTree(Graph&G, node s, NodeArray<int>& dist, bool directed = false) {
	Queue<node> q;
	NodeArray<bool> visited(G, false);
	q.append(s);
	visited[s] = true;
	dist[s] = 0;
	while (!q.empty()) {
		node u = q.pop();
		for (adjEntry adj : u->adjEntries) {
			edge e = adj->theEdge();
			if (directed && u != e->source())
				continue;
			node v = e->opposite(u);
			if (visited[v] == false) {
				q.append(v);
				visited[v] = true;
				dist[v] = dist[u] + 1;
			}
		}
	}
}

int findFarthestNodePairs(Graph&G, node& s, node& t) {
	int maxDist = 0;
	int stopAfter = G.numberOfNodes();
	if (stopAfter > 1000) 
		stopAfter = 100;
	
	for (node u : G.nodes) {
		NodeArray<int> dist(G, 0);
		runBFSTree(G, u, dist);
		for (node v : G.nodes) {
			if (dist[v] > maxDist) {
				maxDist = dist[v];
				s = u;
				t = v;
			}
		}
		stopAfter--;
		if (stopAfter <= 0)
			break;
	}
	return maxDist;
}

void assignColorsUniform(int numColors, vector<edge>& allEdges,
													EdgeArray<ColorSet>& C) 
{
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, numColors-1);
	for (edge e : allEdges) {
		int colIdx = distribution(generator);
		C[e].insert(colIdx);
	}
}

void assignColorsBinomial(int numColors, vector<edge>& allEdges,
													EdgeArray<ColorSet>& C) 
{
	std::default_random_engine generator;
	std::binomial_distribution<int> distribution(numColors, 0.5);
	for (edge e : allEdges) {
		int colIdx = distribution(generator);
		//assert(colIdx >= 0 && colIdx < numColors);
		C[e].insert(colIdx);
	}
}

void assignColorsGaussian(int numColors, vector<edge>& allEdges,
													EdgeArray<ColorSet>& C) 
{
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0.5, 0.16);
	for (edge e : allEdges) {
		int colIdx = (int)(distribution(generator) * numColors);
		if (colIdx >= 0 && colIdx < numColors)
			C[e].insert(colIdx);
	}
}

// On edge e, sample color i with probability prob[i], and
// append to its colorset if trial is success until e has at most
// limit colors
void assignColorsProb(vector<double>& prob, ColorSet& ce, int limit) 
{
	srand (time(NULL));
	for (int i = 0; i < prob.size(); i++) {
		if (ce.size() >= limit)
			return;
		double sample = randomDouble(0, 1);
		if (sample < prob[i])
			ce.insert(i);
	}
}

void assignColorsUniformProb(int colorUB, ColorSet& ce, int colorLB = 0)
{
	int numColors = colorUB - colorLB;
	double prob = 1.0 / numColors;
	srand (time(NULL));
	for (int i = colorLB; i < colorUB; i++) {
		double sample = randomDouble(0, 1);
		if (sample < prob)
			ce.insert(i);
	}
	//cout << "Assigned : " << ce.size() << " colors" << endl;
}

} // end namespace Utils

//-------------------------------
// Graph with colors on vertices
//-------------------------------
class VertexColoredGraph {
	Graph m_Graph;
	NodeArray<ColorSet> m_Colors;

public:
	VertexColoredGraph() : m_Colors(m_Graph) { } 
	VertexColoredGraph(Graph& g) : m_Graph(g), m_Colors(m_Graph) { } 
	Graph& graph() { return m_Graph; }
	NodeArray<ColorSet>& colors() { return m_Colors; }
	
	// Assign colors to vertices randomly
	void setRandomColors(int numColors) {
		srand (time(NULL));
		for (node n : m_Graph.nodes) {
			int cIdx = rand() % numColors;
			m_Colors[n].insert(cIdx);
		}
	}

	void write(std::string f, GraphIO::AttrWriterFunc wf) {
		GraphAttributes ga(m_Graph, GraphAttributes::nodeLabel);
		ga.directed() = false;
		for (node n : m_Graph.nodes)
			ga.label(n) =  Utils::colorSetToString(m_Colors[n]);
		GraphIO::write(ga, f, wf);
	}

	int numColors() {
		ColorSet s;
		for (node n : m_Graph.nodes)
			s.insert(m_Colors[n].begin(), m_Colors[n].end());
		return s.size();
	}

};

//-------------------------------
// Graph with colors on edges
//-------------------------------
class EdgeColoredGraph {
protected:
	Graph m_Graph;
	EdgeArray<ColorSet> m_Colors;
	node m_src;
	node m_dst;
	int m_NumColors;
	int m_Diameter;

public:
	EdgeColoredGraph() : m_Colors(m_Graph), m_src(0), m_dst(0), m_NumColors(0), m_Diameter(0) { } 
	
	EdgeColoredGraph(Graph& g) : m_Graph(g), m_src(0), m_dst(0), m_Colors(m_Graph), m_Diameter(0) { } 
	
	// Copy constructor
	EdgeColoredGraph(EdgeColoredGraph& cg) : m_Colors(m_Graph) {
		Graph& G = cg.graph();
		NodeArray<node> old2New(G);
		for (node v : G.nodes)
			old2New[v] = m_Graph.newNode();
		EdgeArray<ColorSet>& C = cg.colors();
		for (edge e : G.edges) {
			node u = old2New[e->source()];
			node v = old2New[e->target()];
			edge ep = m_Graph.newEdge(u, v);
			m_Colors[ep] = C[e];
		}
		m_src = old2New[cg.src()];
		m_dst = old2New[cg.dst()];
		m_NumColors = cg.numColors();
		m_Diameter = cg.diameter();
	}

	Graph& graph() { return m_Graph; }
	EdgeArray<ColorSet>& colors() { return m_Colors; }
	node& src() { return m_src; }
	node& dst() { return m_dst; }
	int diameter() { return m_Diameter; }
	int numColors() { return m_NumColors; }
	void numColors(int k) { m_NumColors = k; }
	void setDiameter() { m_Diameter = Utils::findFarthestNodePairs(m_Graph, m_src, m_dst); }
	
	// cYuanEtAl is similar to uniform distribution except that each color gets a chance
	enum DistributionType {cYuanEtAl, cUniformDist, cGaussianDist, cBinomialDist, cNumDistTypes};
	
	// Assign colors on an edge from a distribution until limit colors
	// default: Gaussian distribution
	// limit : color-density (number of colors on each edge), default 3.
	//void assignFromDistribution(int numColors, DistributionType d = cUniformDist, int limit = 1) {
	void assignFromDistribution(int numColors, DistributionType d, int limit) {
		string distrName[cNumDistTypes] = {"YuanEtAl", "Uniform", "Gaussian", "Binomial"};
		cout << "Assigning: " << numColors << " colors as per " << distrName[d] << " distribution" << endl;
		m_NumColors = numColors;
		srand(time(NULL));
		vector<edge> edgeList;
		for (edge e : m_Graph.edges) 
			edgeList.push_back(e);
		for (int i = 0; i < limit; i++) {
			std::random_shuffle(edgeList.begin(), edgeList.end());
			if (d == cGaussianDist)
				Utils::assignColorsGaussian(numColors, edgeList, m_Colors);
			else if (d == cBinomialDist)
				Utils::assignColorsBinomial(numColors, edgeList, m_Colors);
			else
				Utils::assignColorsUniform(numColors, edgeList, m_Colors);
		}
		m_Diameter = Utils::findFarthestNodePairs(m_Graph, m_src, m_dst);
		assert(m_src && m_dst);
	}
	
	// To construct instances as in Yuan et al.
	// I think Yuan et al. simply picked a color uniformly at random and used
	// it on each edge which makes it a Uniform distribution
	// So may be this is obselete and not needed
	void setRandomColors(int numColors) {
		cout << "Assigning: " << numColors << " colors as per Yuan Et al." << endl;
		m_NumColors = numColors;
		for (edge e : m_Graph.edges)
			Utils::assignColorsUniformProb(numColors, m_Colors[e]);
		// some colors appear without any color which is a problem sometimes
		// because in large graphs, we may have more empty edges due to which
		// increasing number of colors !=> increasing optimal number of colors
		for (edge e : m_Graph.edges)
			if (m_Colors[e].size() == 0)
				m_Colors[e].insert(0);
		m_Diameter = Utils::findFarthestNodePairs(m_Graph, m_src, m_dst);
		assert(m_src && m_dst);
	}

	void assignColorsOnEdges(int numColors, DistributionType d = cGaussianDist, int limit = 3) {
		if (d == cYuanEtAl)
			assignFromDistribution(numColors, d, 1);
		else
			assignFromDistribution(numColors, d, limit);
	}

	void write(std::string f, GraphIO::AttrWriterFunc wf) {
		GraphAttributes ga(m_Graph, GraphAttributes::edgeLabel);
		ga.directed() = false;
		for (edge e : m_Graph.edges)
			ga.label(e) = Utils::colorSetToString(m_Colors[e]);
		GraphIO::write(ga, f, wf);
	}

	// Reads from a DOT file of the following format
	// v1 -> v2 [label = "c1"]
	// v3 -> v2 [label = "c2"].. and so on
	void readDOT(std::string fname, GraphAttributes& ga) {
		std::ifstream is(fname);
		GraphIO::readDOT(ga, m_Graph, is);
		int maxColorId = 0;
		for (edge e : m_Graph.edges) {
			string colorId = ga.label(e);
			colorId.erase(0, 1);
			int cId = std::stoi(colorId);
			m_Colors[e].insert(cId);
			maxColorId = max(cId, maxColorId);
		}
		assert(isParallelFree(m_Graph));
		numColors(maxColorId + 1);
		m_Diameter = Utils::findFarthestNodePairs(m_Graph, m_src, m_dst);
		assert(m_src && m_dst);
	}


	double averageColorsPerEdge() {
		int numOcc = 0;
		for (edge e : m_Graph.edges)
			numOcc += m_Colors[e].size();
		return numOcc / (m_Graph.numberOfEdges() * 1.0);
	}
	
	// LP needs bi-directional graphs
	void makeBidirectional() {
		Graph& G = m_Graph;
		vector<edge> allEdges;
		for (edge e : G.edges) 
			allEdges.push_back(e);
		for (edge e : allEdges) {
			//cout << "reversing edge " <<  e << endl;
			node u = e->source();
			node v = e->target();
			edge ep = G.newEdge(v, u);
			m_Colors[ep] = m_Colors[e];
		}
	}

	// Print colors
	void printColorInfo(vector<string> colorNames) {
		vector<int> occ(m_NumColors, 0);
		for (edge e : m_Graph.edges)
			for (int c : m_Colors[e])
				occ[c]++;
		
		// Print distribution as stars
		cout << "Diameter: " << m_Diameter << endl;
		cout << "Average colors per edge: " << averageColorsPerEdge() << 
						" with distribution: " << endl; 
		for (int i = 0; i < occ.size(); i++) {
			if (occ[i] == 0)
				continue;
			cout << colorNames[i];
			int stars = occ[i]/10;
			for (int j = 0; j < stars; j++)
				cout << "*";
			cout << endl;
		}
	}

	void printColorInfo() {
		vector<string> temp;
		for (int i = 0; i < m_NumColors; i++) {
			string colName = "C_" + std::to_string(i);
			colName += " : ";
			temp.push_back(colName);
		}
		printColorInfo(temp);
	}
	
	void printInfo(bool colors = true) {
		cout << "Graph has " << m_Graph.numberOfNodes() << " vertices and " 
				 << m_Graph.numberOfEdges() << " edges and " << numColors() << " colors" <<  endl;
		cout << "   Requested path from :" << m_src << " to " << m_dst << endl;
		cout << "   Is parallel free? : " << isParallelFree(m_Graph) << endl; 
		cout << "   Is loop free? : " << isLoopFree(m_Graph) << endl; 
		if (colors) printColorInfo();
	}
	
};

#endif
