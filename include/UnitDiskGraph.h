#ifndef UNIT_DISK_GRAPH_H
#define UNIT_DISK_GRAPH_H

#include "GeometricObjects.h"
#include "WriteLatex.h"
#include <dirent.h>

class UnitDiskGraph {
	Graph& m_Graph;
	int m_NumDisks;
	int xLimit;
	int yLimit;
public:
	UnitDiskGraph(Graph& g, int numDisks, WriteToLatex* wl) : m_Graph(g), m_NumDisks(numDisks) {
		xLimit = 10;
		yLimit = numDisks*0.1;
		construct(wl);
	}
	
	void construct(WriteToLatex* wl) {
		vector<Disk> objects;
		vector<node> objectToNode;
		for (int i = 0; i < m_NumDisks; i++) {
			objects.push_back(Disk::getRandomUnit(xLimit, yLimit));
			objectToNode.push_back(m_Graph.newNode());
		}
	
		if (wl) {
			// Draw!
			std::stringstream ss;
			ss << "UDG : " << objects.size() << " Objects";
			wl->initPage(ss.str());
	
			// Draw BBox
			std::stringstream bbox;
			bbox << "\\draw[ dashed, black] (0, 0) rectangle (" << xLimit << "," << yLimit << ");";
			wl->addLine(bbox.str());
	
			for (int i = 0; i < objects.size(); i++) {
				Disk& d =  objects[i];
				d.drawLatex(*wl, i);
				wl->drawPoint(d.center.m_x, d.center.m_y);
			}	
		}

		for (int i = 0; i < m_NumDisks; i++)
			for (int j = i+1; j < m_NumDisks; j++) 
				if (Disk::doIntersect(objects[i], objects[j])) {
					Disk& di =  objects[i];
					Disk& dj =  objects[j];
					m_Graph.newEdge(objectToNode[i], objectToNode[j]);
					if (wl)
						wl->drawLine(di.center, dj.center);
				}
		cout << m_Graph.numberOfNodes() << " nodes and " << m_Graph.numberOfEdges() << " edges" << endl;
	}	
};

#endif
