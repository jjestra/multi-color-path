#ifndef LP_BASED_ALG_H
#define LP_BASED_ALG_H
#include <ogdf/lpsolver/LPSolver.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <iostream>
using namespace ogdf;

// Two types of LP formulations
// GuessOpt has one extra constraint, based on a guess on the number 
// of optimum colors LP can use.
enum LPformulationType {cTypeBasic  = 0, cTypeGuessOpt, cNumTypes};

class LpBasedAlg {
	EdgeColoredGraph& m_Graph;
	node m_src;
	node m_dst;
	//the optimal solution column vector
	Array<double> m_opt; 

	int numCols;
	int colorColOffset;
	int edgeColOffset;
	ogdf::LPSolver* m_Solver;
	Array<double> obj;

	// Set up constraints.  We just need to keep track of indices
	// in the column (aka constraintIdx) where col[i] is set to 1 or -1
	typedef std::pair<int, int> IdxValPair;
	vector<vector<IdxValPair> > nzIndices;
	vector<double> rhsVal;
	vector<double> eqnSense;
	int constraintIdx;
	int specialConstraintIdx;
	int numNonzeros;
	LPSolver::OptimizationGoal m_goal;
	bool m_verbose;
	NodeArray<int> m_NodeIdx;
	EdgeArray<int> m_EdgeIdx;
	LPformulationType m_type;
	int m_guess;

public:
	
	LpBasedAlg(EdgeColoredGraph& cg, LPformulationType t, vector<edge>& bestPathVec,
							int guess = 0, bool ilp = true)
							: m_Graph(cg), m_src(cg.src()), m_dst(cg.dst()), m_NodeIdx(cg.graph()), m_EdgeIdx(cg.graph()), m_type(t), m_guess(guess)
	{
		// LP needs the graph to be bidirectional!
		cg.makeBidirectional();
		cg.printInfo(false);
		Graph& G = cg.graph();
		// being loop-free is required for LP formulation to work!
		assert(isLoopFree(G));

		int n = G.numberOfNodes();
		int m = G.numberOfEdges();
		int k = cg.numColors();
		numCols = k + m; // variables, one for each color c_i and edge e_i
		colorColOffset = 0;
		edgeColOffset = k;
		m_Solver = ilp ? new ogdf::ILPSolver() : new ogdf::LPSolver;
		obj.resize(numCols);
		nzIndices.resize(numCols);
		constraintIdx = 0;
		specialConstraintIdx = -1;
		numNonzeros = 0;
		m_verbose = false;
		int nodeId = 0;
		for (node v : G.nodes)
			m_NodeIdx[v] = nodeId++;
		assert(nodeId == n);
		int edgeId = 0;
		for (edge e : G.edges)
			m_EdgeIdx[e] = edgeId++;
		assert(edgeId == m);

		//Graph& G = m_Graph.graph();
		// First check if m_src, m_dst are connected
		NodeArray<int> comps(G);
		connectedComponents(G, comps);
		assert(m_src != m_dst);
		if (comps[m_src] != comps[m_dst]) {
						cout << "LPError: " << m_src << " and " << m_dst << " are not connected." << endl;
						assert(0);
		}

		addFlowConstraints();
		addColorConstraints();
		formulateObjFunction();
		m_opt.init(numCols);
		for (int i = 0; i < m_opt.size(); i++)
			m_opt[i] = 0;

		// only support basic!
		if (m_guess > 0)
			initializeOptimalSolution(bestPathVec);
	}

	~LpBasedAlg() { delete m_Solver; }

	void verbose(bool set = true) { m_verbose = set; }

	// flow conserved at all nodes except s, t
	void addFlowConstraints();

	// c_j >= e_i for all c_j that lie on e_i
	void addColorConstraints();

	// Two types of LP formulations
	// GuessOpt has one extra constraint, based on a guess on the number 
	// of optimum colors LP can use.
	//enum LPformulationType {cTypeBasic  = 0, cTypeGuessOpt, cNumTypes};
	
	// Formulate objective function (and some extra constraints)
	void formulateObjFunction();

	// the entry point
	int solve(vector<edge>& pathVec);

	// initialize optimal solution
	void initializeOptimalSolution(vector<edge>& bestPathVec);
};

// Set up flow constraints (only uses e_i decision variables)
// sum_out e_i - sum_in e_i = 0 for v != s, t
void LpBasedAlg::addFlowConstraints() 
{
	Graph& G = m_Graph.graph();
	for (node v : G.nodes) {
		if (v->degree() == 0) continue;
		std::stringstream ss;
		ss << "Constraint " << constraintIdx << " : ";
		
		for(adjEntry adj : v->adjEntries) {
			edge e = adj->theEdge();
			int colIdx = edgeColOffset + m_EdgeIdx[e];
			if (adj == e->adjSource()) { 
				// e is outgoing
				nzIndices[colIdx].push_back(IdxValPair(constraintIdx, 1));
				numNonzeros++;
				ss << "+ e_" << e << " ";
			}
			else {
				// e is incoming
				nzIndices[colIdx].push_back(IdxValPair(constraintIdx, -1));
				numNonzeros++;
				ss << "- e_" << e << " ";
			}
		}
		
		eqnSense.push_back('E');
		if (v == m_src)
			rhsVal.push_back(1);
		else if (v == m_dst)
			rhsVal.push_back(-1);
		else
			rhsVal.push_back(0);
		ss << " = " << rhsVal[constraintIdx];
		constraintIdx++;
		
		if (m_verbose)
			cout << ss.str() << endl;
	}
}

// Set up color constraints, c_j - e_i >= 0, for all colors c_j that lie on e_i
void LpBasedAlg::addColorConstraints() 
{
	Graph& G = m_Graph.graph();
	EdgeArray<ColorSet>& C = m_Graph.colors();
	for (edge e : G.edges) {
		int edgeColIdx = edgeColOffset + m_EdgeIdx[e];
		for (color c : C[e]) {
			int colorColIdx = colorColOffset + c;
			eqnSense.push_back('G');
			rhsVal.push_back(0);
			nzIndices[colorColIdx].push_back(IdxValPair(constraintIdx, 1));
			nzIndices[edgeColIdx].push_back(IdxValPair(constraintIdx, -1));
			numNonzeros += 2;
			if (m_verbose)
				cout << "Constraint " << constraintIdx << " : c_" << c << " - " << " e_" << e << " >= " << rhsVal[constraintIdx] << endl;
			constraintIdx++;
		}
	}
	assert(constraintIdx == rhsVal.size());
}

// Formulate objective function (and some extra constraints)
void LpBasedAlg::formulateObjFunction() 
{
	// Initialize
	for (int i = 0; i < numCols; i++)
		obj[i] = 0;
	
	if (m_type == cTypeBasic) {
		// minimize \sum_i c_i
		m_goal = LPSolver::OptimizationGoal::Minimize;
		for (int i = 0; i < edgeColOffset; i++) 
			obj[i] = 1;
	}
	else {
		// maximize \sum_i e_i or minimize \sum_i e_i
		//m_goal = LPSolver::OptimizationGoal::Maximize;
		m_goal = LPSolver::OptimizationGoal::Minimize;
		assert(m_type == cTypeGuessOpt);
		for (int i = edgeColOffset; i < numCols; i++)
			obj[i] = 1;
		
		// Add one more constraint on the number of colors LP can use
		std::stringstream ss;
		for (int i = 0; i < edgeColOffset; i++) {
			nzIndices[i].push_back(IdxValPair(constraintIdx, 1));
			ss << "+ c_" << i << " ";
			numNonzeros++;
		}
		specialConstraintIdx = constraintIdx;
		if (m_verbose)
			cout << "Constraint " << constraintIdx << " : " << ss.str() << " <= [1..opt]" << endl;
		constraintIdx++;
		eqnSense.push_back('L');
		rhsVal.push_back(edgeColOffset);
	}
}

void LpBasedAlg::initializeOptimalSolution(vector<edge>& bestPathVec) {
	cout << "Initializing the optimal solution" << endl;
	ColorSet cs = Utils::getColorSet(m_Graph, bestPathVec);
	for (edge e : bestPathVec) {
		 int edgeColIdx = edgeColOffset + m_EdgeIdx[e];
		 m_opt[edgeColIdx] = 1;
	}
	for (auto c : cs)
		m_opt[c] = 1;
}

int LpBasedAlg::solve(vector<edge>& pathVec)
{
	Graph& G = m_Graph.graph();
	int numRows = constraintIdx;
	Array<double> rhs(numRows);
	Array<char> equationSense(numRows);
	for (int i = 0; i < numRows; i++) {
		rhs[i] = rhsVal[i];
		equationSense[i] = eqnSense[i];
	}

	Array<int> mb(numCols);
	Array<int> mc(numCols);
	Array<int> mIdx(numNonzeros);
	Array<double> mVal(numNonzeros);

	// Fill in the sparse form for solver
	int startIdx = 0;
	int nzIndex = 0;
	for (int i = 0; i < numCols; i++) {
		vector<IdxValPair>& xi = nzIndices[i];
		mb[i] = startIdx;
		mc[i] = xi.size();
		startIdx += mc[i];
		for (IdxValPair& iv : xi) {
			mIdx[nzIndex] = iv.first;
			mVal[nzIndex++] = iv.second;
		}
	}

	double optimum; // value of the optimum solution
	//Array<double> x(numCols); // column vector of optimal solution
	Array<double> lowerBound(numCols);
	Array<double> upperBound(numCols);
	for(int i = 0; i < numCols; ++i) {
		lowerBound[i] = 0;
		upperBound[i] = 1;
	}

	
	cout << numCols << " variables and " << constraintIdx << " constraints." << endl;
	
	LPSolver::Status st;
	assert(m_type == cTypeBasic);

	st = m_Solver->optimize(m_goal, obj, mb, mc, mIdx, mVal, rhs, equationSense,
		    						 			lowerBound, upperBound, optimum, m_opt);

	assert(st == LPSolver::Status::Optimal);
	
	// Postprocess the LP result
	for (edge e : G.edges) {
		 int edgeColIdx = edgeColOffset + m_EdgeIdx[e];
		 if (m_opt[edgeColIdx] > 0)
		 	pathVec.push_back(e);
	}

	int usedColors = 0;
	for (int i = 0; i < edgeColOffset; i++)
		if (m_opt[i] > 0)
			usedColors++;
	
	//cout << m_opt << endl;
	return usedColors;
}

#endif



