#ifndef GEOMETRIC_OBJECTS_H
#define GEOMETRIC_OBJECTS_H

#include "WriteLatex.h"

//////////////////////////////////////////////////////
//// Represents an edge in the embedding
//////////////////////////////////////////////////////
template <typename T>
struct Arc {
	// the underlying geometric object
	T* m_object; 
	// Arc = part of boundary when clockwise moving from start to end.
	DPoint start;
	DPoint end;
	// mid point of this arc
	DPoint midPt;
	
	Arc() {}
	Arc(T* obj, DPoint s, DPoint e) : m_object(obj), start(s), end(e) {
		midPt = computeMidpoint();
	}

	// Compute midpoint of this arc
	DPoint computeMidpoint() { return DPoint(); }

	// Compute intersection of this arc with the line through p1,p2
	bool computeIntersection(DPoint p1, DPoint p2, DPoint& iPt, bool direction) { return false; }
	
	//! Writes the string representation of object to output stream os.
	friend std::ostream &operator<<(std::ostream &os, const Arc<T> &a) {
		return os << "Arc : " << *(a.m_object) << " cw: " << a.start << " to " << a.end << " MP: " << a.midPt;
	}
};

	
////////////////////////////////////////////////////////
// Basic data structures for some geometric objects
////////////////////////////////////////////////////////
struct Disk {
	IPoint center;
	int radius;
	
	Disk(IPoint c, int r) : center(c), radius(r) {}
	DPoint getCenter() { return DPoint(center.m_x, center.m_y); }
	void growUnit() { radius++; }

	// strictly inside
	bool contains(DPoint p) {
		double x = p.m_x - center.m_x;
		double y = p.m_y - center.m_y;
		return (x*x + y*y < radius*radius);
	}

	// Assumes p1, p2 lie on d; true if p1 comes earlier in a clockwise ordering
	// sort the points by their angles in polar coordinate
	struct CompareBoundary {
		Disk& m_d;
		CompareBoundary(Disk& d) : m_d(d) {}
		bool operator()(const DPoint& p1, const DPoint& p2) { 
			DPoint shift(m_d.center.m_x, m_d.center.m_y);
			DPoint pp1 = p1 - shift;
			DPoint pp2 = p2 - shift;
			DPoint c(0, 0);
			DPoint posX(1, 0);
			double a1 = c.angle(pp1, posX);
			double a2 = c.angle(pp2, posX);
			return a1 < a2;
		}
	};
	
	// Assumes p1, p2 lie on d; true if p1 comes earlier in a clockwise ordering
	struct CompareArcsAtPivot {
		DPoint m_p;
		CompareArcsAtPivot(DPoint& p) : m_p(p) {}
		DPoint rotate90Degrees(DPoint p,  bool acw) { 
			if (acw)
				return DPoint(-p.m_y, p.m_x);
			else
				return DPoint(p.m_y, -p.m_x);
		}

		bool operator()(Arc<Disk>* a1, Arc<Disk>* a2) { 
			DPoint p1(a1->m_object->center.m_x, a1->m_object->center.m_y);
			DPoint p2(a2->m_object->center.m_x, a2->m_object->center.m_y);
			// Make m_p origin
			DPoint c(0, 0);
			p1 = p1 - m_p; 
			p2 = p2 - m_p;
			// p1, p2 are vectors to centers of circles, the tangent vectors are orthogonal
			p1 = rotate90Degrees(p1, a1->start == m_p);
			p2 = rotate90Degrees(p2, a2->start == m_p);
			DPoint posX(1, 0);
			double aa1 = c.angle(p1, posX);
			double aa2 = c.angle(p2, posX);
			return aa1 < aa2;
		}
	};
	
	static Disk getRandomUnit(int maxCoordX, int maxCoordY) {
		int cx = randomNumber(1, maxCoordX-1);
		int cy = randomNumber(1, maxCoordY-1);
		return Disk(IPoint(cx, cy), 1); 
	}
	
	static Disk getRandom(int maxCoordX, int maxCoordY, int spanLowerBound = 1) {
		int cx = randomNumber(1, maxCoordX-1);
		int cy = randomNumber(1, maxCoordY-1);
		// Making sure we sample a disk that lies in the 
		// bounding box (0,0) (maxCoordX, maxCoordY)
		int maxDist = min(cx, maxCoordX - cx);
		maxDist = min(maxDist, cy);
		maxDist = min(maxDist, maxCoordY - cy);
		int radius = randomNumber(spanLowerBound, maxDist+spanLowerBound);
		return Disk(IPoint(cx, cy), radius); 
	}

	static bool doIntersect(Disk d1, Disk d2) {
		double d = d1.center.distance(d2.center);
		int sradii = d1.radius + d2.radius;
		int dradii = abs(d1.radius - d2.radius);
		// if the circles don't touch or intersect, return
		if (d > sradii || d < dradii)
			return false;
		return true;
	}
	
	static bool getIntersections(Disk d1, Disk d2, vector<DPoint>& pts) {
		// If d1, d2 have same center and radius, ignore their intersections.
		if (d1.center == d2.center && d1.radius == d2.radius)
			return false;
		// Otherwise check if they intersect
		double d = d1.center.distance(d2.center);
		int sradii = d1.radius + d2.radius;
		int dradii = abs(d1.radius - d2.radius);
		// if the circles don't touch or intersect, return
		if (d > sradii || d < dradii)
			return false;

		// Target points p1, p2. Let px be the point where p1p2 intersects c1c2
		// a = segment c1px, h = segment p1px, a and h are orthogonal
		DPoint c1(d1.center.m_x, d1.center.m_y);
		DPoint c2(d2.center.m_x, d2.center.m_y);
		int r1 = d1.radius;
		int r2 = d2.radius;
		double a, h;
		a = (r1*r1 - r2*r2 + d*d)/(2*d);
		h = sqrt(r1*r1 - a*a);
		double af = a / d;
		DPoint px = c1 + ((c2-c1) * af);
		if (h == 0)
			pts.push_back(px); // touching circles
		else {
			double hf = h / d;
			double xdelta = (c2.m_x - c1.m_x) * hf;
			double ydelta = (c2.m_y - c1.m_y) * hf;
			pts.push_back(DPoint(px.m_x + ydelta, px.m_y - xdelta));
			pts.push_back(DPoint(px.m_x - ydelta, px.m_y + xdelta));
	  }
		return true;
	}

	void drawLatex(WriteToLatex& wl, int idx) {
		wl.drawCircle(center, radius, ColorUtils::getColor(idx));
	}

	//! Compare two objects to eliminate duplicates.
	friend bool operator==(const Disk& d1, const Disk& d2) {
		return d1.center == d2.center && d1.radius == d2.radius;
	}
	
	//! Compare two objects to eliminate duplicates.
	friend bool operator<(const Disk& d1, const Disk& d2) {
		if (d1.center == d2.center)
			return d1.radius < d2.radius;
		return d1.center < d2.center;
	}
	
	//! Writes the string representation of object to output stream os.
	friend std::ostream &operator<<(std::ostream &os, const Disk &d) {
		return os << "Disk c: " << d.center << " r: " << d.radius;
	}

};

// Return the side of s on which p lies.
// ordering starts at pivot. sides: 0, 1, 2, 3 clockwise
int incidentSide(IPoint pivot, int side, const DPoint& p1) {
		int pSide = 0;
		if (p1.m_x == pivot.m_x)
			pSide = 0;
		else if (p1.m_x == pivot.m_x + side)
			pSide = 2;
		else if (p1.m_y == pivot.m_y)
			pSide = 3;
		else 
			pSide = 1;
		return pSide;
}
	
struct Square {
	IPoint pivot;
	int side;
	Square(IPoint p, int s) : pivot(p), side(s) {}
	DPoint getCenter() { 
		return DPoint(pivot.m_x + side/2, pivot.m_y + side/2);
	}

	void growUnit() { side++; }
	
	// strictly contains
	bool contains(DPoint p) {
		bool xok = pivot.m_x < p.m_x && p.m_x < pivot.m_x + side;
		bool yok = pivot.m_y < p.m_y && p.m_y < pivot.m_y + side;
		return xok && yok;
	}

	// Sample a random square
	static Square getRandom(int maxCoordX, int maxCoordY, int spanLowerBound = 1) {
		int cx = randomNumber(1, maxCoordX-1);
		int cy = randomNumber(1, maxCoordY-1);
		int maxDist = min(cx, maxCoordX - cx);
		maxDist = min(maxDist, cy);
		maxDist = min(maxDist, maxCoordY - cy);
		int side = randomNumber(spanLowerBound, maxDist+spanLowerBound);
		return Square(IPoint(cx, cy), side);
	}
	
	// Assumes p1, p2 lie on d; true if p1 comes earlier in a clockwise ordering
	struct CompareBoundary {
		Square& m_s;
		CompareBoundary(Square& s) : m_s(s) {}
		bool operator()(const DPoint& p1, const DPoint& p2) { 
			int p1Side = incidentSide(m_s.pivot, m_s.side, p1);
			int p2Side = incidentSide(m_s.pivot, m_s.side, p2);
			if (p1Side == p2Side) {
				int p1dec = p1.m_y;
				int p2dec = p2.m_y;
				if (p1Side == 1 || p1Side == 3) {
					p1dec = p1.m_x;
					p2dec = p2.m_x;
				}
				if (p1Side == 2 || p1Side == 3) {
					// more is less
					p1dec *= -1; 
					p2dec *= -1;
				}
				return p1dec < p2dec;
			}
			else
				return p1Side < p2Side;
		}
	};

	// Order arcs at the pivot m_p
	struct CompareArcsAtPivot {
		DPoint m_p;
		CompareArcsAtPivot(DPoint& p) : m_p(p) {}
		bool operator()(Arc<Square>& a1, Arc<Square>& a2) {
			assert((a1.start == m_p || a1.end == m_p) &&
						 (a2.start == m_p || a2.end == m_p));
			return true;
		}
	};

	static bool getIntersections(Square s1, Square s2, vector<DPoint>& pts) {
		// TODO
		return false;
	}
	
	void drawLatex(WriteToLatex& wl, int idx) {
		wl.drawSquare(pivot, side, ColorUtils::getColor(idx));
	}

	//! Compare two objects to eliminate duplicates.
	friend bool operator<(const Square& s1, const Square& s2) {
		if (s1.pivot == s2.pivot)
			return s1.side < s2.side;
		return s1.pivot < s2.pivot;
	}
	
	//! Compare two objects to eliminate duplicates.
	friend bool operator==(const Square& s1, const Square& s2) {
		return s1.pivot == s2.pivot && s1.side == s2.side;
	}
	
	//! Writes the string representation of object to output stream os.
	friend std::ostream &operator<<(std::ostream &os, const Square& s) {
		return os << "Square bottom-left: " << s.pivot << " side: " << s.side;
	}

};

struct Rectangle {
	IPoint pivotL;
	IPoint pivotR;
	Rectangle(IPoint pl, IPoint pr) : pivotL(pl), pivotR(pr) {}
	
	// Sample a random rectangle
	static Rectangle getRandom(int maxCoordX, int maxCoordY, int spanLowerBound = 1) {
		// TODO
		return Rectangle(IPoint(0, 0), IPoint(1, 1)); 
	}
	//! Writes the string representation of object to output stream os.
	friend std::ostream &operator<<(std::ostream &os, const Rectangle &r) {
		return os << "Rect: bottom-left" << r.pivotL << " top-Right: " << r.pivotR;
	}
};


///////////////////////////////////////////////////////////////
////// Leftover functions
///////////////////////////////////////////////////////////////


template<> DPoint Arc<Square>::computeMidpoint() { 

}

template<> DPoint Arc<Disk>::computeMidpoint() { 
	DPoint shift(m_object->center.m_x, m_object->center.m_y);
	DPoint ss = start - shift;
	DPoint es = end - shift;
	int radius = m_object->radius;
	DPoint c(0, 0);
	double ra = c.angle(es, ss) / 2;
	// Rotate es counterclockwise by 'ra'
	double x = es.m_x * cos(ra) - es.m_y * sin(ra);
	double y = es.m_x * sin(ra) + es.m_y * cos(ra);
	DPoint rVal(x, y);
	return (rVal + shift); 
}

template<> bool Arc<Disk>::computeIntersection(DPoint p1, DPoint p2, DPoint& iPt, bool towardsP1) {
	DPoint shift(m_object->center.m_x, m_object->center.m_y);
	p1 = p1 - shift;
	p2 = p2 - shift;
	int r = m_object->radius;

	// Equation of line in ax + by + c = 0 form
	double a = p2.m_y - p1.m_y;
	double b = p1.m_x - p2.m_x;
	double c = (p2.m_x * p1.m_y) - (p1.m_x * p2.m_y);
	// compute the distance from center (0, 0)
	double dist = abs(c) / sqrt(a*a + b*b);
	if (dist >= r)
		return false;
	// At this point the line intersects the circle, find the two intersection points
	DPoint xp1, xp2;
	// Handle the case when L is horizontal
	if (p1.m_x == p2.m_x) {
		xp1.m_x = xp2.m_x = p1.m_x;
		xp1.m_y = sqrt(r*r - p1.m_x * p1.m_x);
		xp2.m_y = -xp1.m_y;
	}
	else if (p1.m_y == p2.m_y) {
		xp1.m_y = xp2.m_y = p1.m_y;
		xp1.m_x = sqrt(r*r - p1.m_y * p1.m_y);
		xp2.m_x = -xp1.m_x;
	}
	else {
		double m = -a / b;
		double ci = -c / b;
		// y = mx + ci and x^2 + y^2 = r^2
		// Gives: (m^2 + 1) x^2 + 2mx + (ci^2 - r^2)
		// with A = (m^2 + 1), B = 2m,  C = (ci^2 - r^2)
		double A = m*m + 1;
		double B = 2*m*ci;
		double C = ci*ci - r*r;
		double det = sqrt(B*B - 4*A*C);
		xp1.m_x = (-B + det) / (2*A);
		xp2.m_x = (-B - det) / (2*A);
		xp1.m_y = m * xp1.m_x + ci;
		xp2.m_y = m * xp2.m_x + ci;
	}

	// Find which of xp1 or xp2 lies on the arc.
	// To see this, just compute the angle between vectors and if the sums 
	// add up, we are good.
	DPoint origin(0, 0);
	DPoint ss = start - shift;
	DPoint es = end - shift;
	EpsilonTest cmp(0.001);
	// Check if xp1 lies between ss and es (ss to es is clockwise)
	if (cmp.equal(origin.angle(es, xp1) + origin.angle(xp1, ss), origin.angle(es, ss)))
		iPt = xp1;
	else if (cmp.equal(origin.angle(es, xp2) + origin.angle(xp2, ss), origin.angle(es, ss)))
		iPt = xp2;
	else
		return false;

	bool closerToP1 = false;
	if (cmp.equal(p2.distance(iPt) + p1.distance(iPt), p1.distance(p2)) ||
			cmp.equal(p2.distance(iPt), p1.distance(iPt) + p1.distance(p2)))
			closerToP1 = true;
	if ((towardsP1 && !closerToP1) || (!towardsP1 && closerToP1))
		return false;
	iPt += shift;
	//cout << endl << xp1 << " and " << xp2 << " => " << iPt << endl;
	return true;
}


#endif
