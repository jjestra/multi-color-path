# Min-color-path
## Computing a minimum color path in an Edge colored graph

We study the problem of computing a min-color path in an edge-colored graph. More
precisely, we are given a graph G = (V, E), source s, target t, an assignment χ : E → 2 C of edges
to a set of colors in C, and we want to find a path from s to t such that the number of unique
colors on this path is minimum over all possible s-t paths. 

We design and implement two simple greedy heuristics for this problem, and analyze their performance
on an extensive set of synthetic and real world datasets. 
From our experiments, we found that our heuristics perform significantly better
than the best previous heuristic algorithm for the problem on all datasets, both in terms of path
quality and the running time. We also implement an ILP formulation for the problem using Gurobi
MIP solver (version 8.0.1 Academic License).

This code supplements the [paper](http://sud03r.github.io/papers/sea-19.pdf). The final conference version
can be found at [springer link.](https://link.springer.com/chapter/10.1007/978-3-030-34029-2_3)

## Setting it up:
Depends on [OGDF](https://github.com/ogdf/ogdf) and [Gurobi](http://www.gurobi.com/).
Once we have both those libraries available, setting it up is straightforward : just need to
make some minor changes in Makefile to point to your OGDF and Gurobi installations, and then
run make afterwards.

`make`  (Creates an executable called `mcp`)



## How to run
```
Two ways to use:
 
1. Run on your own colored graph
   (Each edge is a row of format: (see ./testdata/graph.txt for an example)
	  vertexId vertexId  colorId
		...
 Usage: ./mcp readFromFile filename srcNodeId dstNodeId [options]

2. Run on existing datasets
 Usage: ./mcp dataset-name [options]
 Available datasets:{random  layered unit-disk topology roadnet union}
 Other options: 
   noILP            : Don't run ILP 
   noDijkstraBased  : Only run ILP 
   uniformDist      : assign colors uniformly, default is normal dist 
   -repeat K        : Repeat runs K times, default repeat 1
```
Some examples:

```
./mcp readFromFile ./testData/graph.txt 1 10      (Read from file and find min-color path between nodes 1 and 10)
./mcp random noILP -repeat 20                     (Run random instances 20 times without ILP)
./mcp random noILP -repeat 20 runLargeInstances   (Run large random instances 20 times without ILP)
./mcp topology                                    (Run the topology-zoo instance)
./mcp unit-disk noILP repeat 10                   (Run the unit-disk instance)
```
## More on Datasets

+ **Layered Graphs (layered):** These graphs comprise of n nodes arranged in a k × (n/k) grid.
comprises of n nodes arranged in a k × (n/k) grid. A node v ij in coloumn j is connected
to all nodes v lj+1 in the next column and the for all l = {1, . . . , k}. All vertices in the
first column are connected to the source s, and the last column are connected to t. 

+ **Unit Disk Intersection Graphs (unit-disk)** These graphs comprise of a collection of n unit disks
randomly arranged in a rectangular region 10 x n/10. The graph is defined as usual, each disk
corresponds to a vertex and is connected to all the other disks it intersects. Since the
edges only exist between vertices that are close to each other, disk intersection graphs
tend to have large diameters proportional to the dimensions of the rectangular region.

+ **Topology zoo (topology)**: The internet topology dataset from [here](http://www.topology-zoo.org/).
	The service providers correspond to colors.

+ **CA Road Network (roadnet)**: Graph dataset taken from [here](https://snap.stanford.edu/data/roadNet-CA.html).

+ **Union Graphs (union)**: Union of 20 random graphs connected in series, each with n/20 nodes.

+ **Random Graphs (random)**: Graphs created under G(n, p) model with uniformly assigned colors.


Unless otherwise stated, colors are assigned randomly as per a given distribution. The 
default is a truncated normal distribution scaled by number of colors
and rounded up to nearest integer.

