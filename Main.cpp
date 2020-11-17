#include <iostream>
#include "RandomGraphModels.h"
#include "kChainGraph.h"
#include "DijkstraBasedAlg.h"
#include "LpBasedAlg.h"
#include <ogdf/basic/Stopwatch.h>
#include "TopologyZoo.h"
#include "CustomColoredGraph.h"

//namespace Globals {
EdgeColoredGraph::DistributionType gDistType(EdgeColoredGraph::cGaussianDist);
int gLimitPerEdge(3);
static bool gLargeInstances(false);
static bool gNoILP(true);
static bool gNoDijkstraBased(false);
//}

struct Result {
	int colorCount;
	int timeMs;
	int pathLength;
	Result() : colorCount(0), timeMs(0), pathLength(0) {}
};

vector<Result> s_graphAlgs(DijkstraBasedAlg::NUM_ALGS);
vector<Result> s_lpAlgs(cNumTypes);

void runAlgorithms(EdgeColoredGraph& cg, GraphAttributes& ga,
									 WriteToLatex& wl, bool draw = false) 
{
	// Print colored graph Info:
	cg.printInfo();
	//return;
		
	vector<edge> pathVec;
	int colorCost = 0;
	vector<edge> bestPathVec;
	int bestColorCost = std::numeric_limits<int>::max();
	if (!gNoDijkstraBased) {
		DijkstraBasedAlg alg(cg, &wl, &ga);
		int numAlgs = DijkstraBasedAlg::NUM_ALGS;
		string pathColors[numAlgs] = {"red", "orange", "blue", "green"};
		for (int algIdx = 0; algIdx < numAlgs; algIdx++) {	
			pathVec.clear();
			StopwatchWallClock timer;
			timer.start();
			DijkstraBasedAlg::AlgType t = (DijkstraBasedAlg::AlgType)algIdx;
			colorCost = alg.runAlgorithm(t, pathVec); 
			timer.stop();
			
			if (colorCost == -1)
				continue;
			
			if (colorCost < bestColorCost) {
				bestColorCost = colorCost;
				bestPathVec = pathVec;
			}

			cout << "Alg: " << alg.getDescStr(t)  <<  " took " << timer.milliSeconds() << "ms" << endl;
			cout << "NumColors: " <<  colorCost << " PathLength: " << pathVec.size()  << endl;
			ColorSet cs = Utils::getColorSet(cg, pathVec);
			cout << "Path Colors:" << Utils::colorSetToString(cs) << endl << endl;
			if (draw) wl.drawPath(ga, pathVec, pathColors[algIdx]);

			s_graphAlgs[algIdx].colorCount += colorCost;
			s_graphAlgs[algIdx].timeMs += timer.milliSeconds();
			s_graphAlgs[algIdx].pathLength += pathVec.size();
		}
	}
	
	if (gNoILP) {
		cout << "Not running ILP solver " << endl;
		return;
	}
	if (draw) wl.addPause();
	int numAlgs = 1; //cNumTypes;
	for (int algIdx = 0; algIdx < numAlgs; algIdx++) {
		pathVec.clear();
		StopwatchWallClock timer;
		timer.start();
		//auto t = (LPformulationType)0;//algIdx;
		//cg.printInfo(false);
		LpBasedAlg lp(cg, cTypeBasic, bestPathVec);
		colorCost = lp.solve(pathVec);
		timer.stop();
		vector<edge> usefulEdges;
		cout << "LPSolver " << algIdx << " took " << timer.milliSeconds() << "ms" << endl;
		cout << "NumColors: " <<  colorCost << " PathLength: " << pathVec.size() << endl;
		if (draw) wl.drawPath(ga, pathVec, "orange");
		ColorSet cs = Utils::getColorSet(cg, pathVec);
		cout << "Path Colors: " << Utils::colorSetToString(cs) << endl << endl;
		//Utils::checkIfFormsPath(pathVec, cg.src(), cg.dst(), usefulEdges);
		//ColorSet ucs = Utils::getColorSet(cg, usefulEdges);
		//cout << "Useful Path Colors:" << Utils::colorSetToString(ucs) << endl << endl;
		if (colorCost > bestColorCost) {
			// If LP solution is worse than a solution we found, something is wrong!
			cg.write("failure.dot", &GraphIO::writeDOT);
			assert(0);
		}

		s_lpAlgs[algIdx].colorCount += colorCost;
		s_lpAlgs[algIdx].timeMs += timer.milliSeconds();
		s_lpAlgs[algIdx].pathLength += pathVec.size();
	}
}

void UnitDiskGraph(WriteToLatex& wl) {
	int numColors = 50;
	int numNodes = 500;
	if (gLargeInstances) {
		numNodes *= 20;
		numColors *= 10;
	}
	
	bool draw = false;
	RandomColoredUnitDiskGraph cg(numNodes, numColors, draw ? &wl : NULL);
	runAlgorithms(cg.coloredGraph(), cg.attributes(), wl, draw);
}

void LayeredGraph(WriteToLatex& wl) {
	int k = 4; // width
	int n = 125; // chain length
	int colors = 50;

	if (gLargeInstances) {
		n *= 20;
		colors *= 10;
	}

	kChainGraph cg(k, n, colors);
	bool draw = false;
	if (draw) {
		cg.drawLatex(wl);
		cg.drawLatex(wl, "dash dot, ");
	}
	runAlgorithms(cg.coloredGraph(), cg.attributes(), wl, draw);
}

void RandomGraph(WriteToLatex& wl) {
	int numColors = 50;
	int numNodes = 1000;
	if (gLargeInstances) {
		numNodes *= 10;
		numColors *= 10;
	}
	
	RandomColoredGraph cg(numNodes, numColors);
	bool draw = false;
	if (draw) cg.drawLatex(wl);
	runAlgorithms(cg.coloredGraph(), cg.attributes(), wl, draw);
}

void UnionGraph(WriteToLatex& wl) {
	int numColors = 50;
	int numNodes = 1000;
	if (gLargeInstances) {
		numNodes *= 10;
		numColors *= 10;
	}
	RandomColoredUnionGraph cg(numNodes, numColors, 20);
	runAlgorithms(cg.coloredGraph(), cg.attributes(), wl, false);
}

void TopologyZooGraph(WriteToLatex& wl) {
	TopologyZoo cg;
	runAlgorithms(cg.coloredGraph(), cg.attributes(), wl, false);
}

void RoadNetworkGraph(WriteToLatex& wl) {
	int numColors = 500;
	ColoredRoadNetwork cg(numColors);
	runAlgorithms(cg.coloredGraph(), cg.attributes(), wl, false);
}

void ReadColoredGraphFromFile(string filename, int srcId, int dstId, WriteToLatex& wl) {
	CustomColoredGraph cg(filename, srcId, dstId);
	cg.drawLatex(wl);
	runAlgorithms(cg, cg.attributes(), wl, false);
}

void printHelpString(string cmd) {
	cout << "Two ways to use:" << endl;
	cout << " 1. Run on your own colored graph" << endl;
	cout << "   (Each edge is a row of format:) vertexId vertexId  colorId" << endl;
	cout << " Usage: " << cmd << " readFromFile filename srcNodeId dstNodeId [options]" << endl;
	cout << endl;
	cout << " 2. Run on existing datasets" << endl;
	cout << " Usage: " << cmd << " dataset-name [options]" << endl;
	cout << " Available datasets:" << "{random  layered unit-disk topology roadnet union}" << endl;
	cout << " Other options: " << endl;
	cout << "   noILP            : Don't run ILP " << endl;
	cout << "   noDijkstraBased  : Only run ILP " << endl;
	cout << "   uniformDist      : assign colors uniformly, default is normal dist " << endl;
	cout << "   -repeat K        : Repeat runs K times, default repeat 1" << endl;
	cout << "Some examples: " << endl;
	cout << " ./mcp readFromFile ./testData/graph.txt 1 10" << endl;
	cout << " ./mcp random noILP -repeat 20" << endl;
	cout << " ./mcp random noILP -repeat 20 runLargeInstances" << endl;
	cout << " ./mcp topology" << endl;
	cout << " ./mcp unit-disk noILP repeat 10" << endl;
}

// Generate graph datasets for minimum color path problems.
int main(int argc, char* argv[])
{	
	if (argc < 2) {
		printHelpString(argv[0]);
		return -1;
	}
	string dataset = argv[1];
	int numTimesRepeat = 1;
	
	// Mode 1: Reading from File
	if (dataset == "readFromFile") {
		if (argc < 5) {
			std::cout << "Error: Missing Filename" << std::endl;
			cout << "Usage: " << argv[0] << " readFromFile filename [options]" << endl;
			return -1;
		}
		WriteToLatex wl;
		ReadColoredGraphFromFile(argv[2], std::atoi(argv[3]), std::atoi(argv[4]), wl);
		return 0;
	}

	// Disable some algorithms if the user is asking for it
	for (int j = 2; j < argc; j++) {
		string disableAlgs = argv[j];
		if (disableAlgs == "noILP") gNoILP = true;
		if (disableAlgs == "noDijkstraBased") gNoDijkstraBased = true;
		if (disableAlgs == "runLargeInstances") gLargeInstances = true;
		if (disableAlgs == "-repeat") numTimesRepeat = std::stoi(argv[++j]);
		if (disableAlgs == "repeat") numTimesRepeat = std::stoi(argv[++j]);
		if (disableAlgs == "yuanEtAlDist") gDistType = EdgeColoredGraph::cYuanEtAl;
		if (disableAlgs == "uniformDist") gDistType = EdgeColoredGraph::cUniformDist;
		if (disableAlgs == "binomialDist") gDistType = EdgeColoredGraph::cBinomialDist;
	}
	
	string distrName[EdgeColoredGraph::cNumDistTypes] = {"YuanEtAl", "Uniform", "Gaussian", "Binomial"};
	cout << "Using Dataset : " <<  dataset << " and repeating averaging results over " << 
			numTimesRepeat << " trials. " << " Color Dist: " << distrName[gDistType] << endl;
	ogdf::setSeed(time(NULL));
	
	for (int i = 0; i < numTimesRepeat; i++) {
		WriteToLatex wl;
		if (dataset == "random")
			RandomGraph(wl);
		else if (dataset == "layered")
			LayeredGraph(wl);
		else if (dataset == "union")
			UnionGraph(wl);
		else if (dataset == "unit-disk")
			UnitDiskGraph(wl);
		else if (dataset == "topology")
			TopologyZooGraph(wl);
		else if (dataset == "roadnet")
			RoadNetworkGraph(wl);	
		else {
			cout << "Unknown dataset : " << dataset << ". Consider running as: " << endl;
			printHelpString(argv[0]);
			return -1;
		}
	}

	cout << "Final Results : " << endl;
	for (int i = 0; i < s_graphAlgs.size(); i++) {
		DijkstraBasedAlg::AlgType t = (DijkstraBasedAlg::AlgType)i;
		double cc = (s_graphAlgs[i].colorCount * 1.0) / numTimesRepeat;
		double tt = (s_graphAlgs[i].timeMs * 1.0) / numTimesRepeat;
		double pl = (s_graphAlgs[i].pathLength * 1.0) / numTimesRepeat;
		if (cc <= 0)
			continue;
		cout << std::setw(20) << DijkstraBasedAlg::getDescStr(t)  <<  " & " << cc << " & " <<  tt << " & " << pl  << " \\\\" << endl;
	}
	for (int i = 0; i < s_lpAlgs.size(); i++) {
		double cc = (s_lpAlgs[i].colorCount * 1.0) / numTimesRepeat;
		double tt = (s_lpAlgs[i].timeMs * 1.0) / numTimesRepeat;
		double pl = (s_lpAlgs[i].pathLength * 1.0) / numTimesRepeat;
		if (cc <= 0)
			continue;
		cout << std::setw(20) << "ILP-" << i+1  <<  " & " << cc << " & " <<  tt << " & " << pl << " \\\\" << endl;
	}
}

