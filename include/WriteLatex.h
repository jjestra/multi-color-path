#ifndef WRITE_LATEX_H
#define WRITE_LATEX_H

#include <ogdf/basic/geometry.h>
#include "ColoredGraph.h"

////////////////////////////////////////////////
// UTILITY functions for getting colors etc
////////////////////////////////////////////////
class ColorUtils {
public:
	// We only support 127 colors for drawing geometric objects
	static Color getColor(int index) {
		int idx = index % 127;
		Color::Name cn = (Color::Name) idx;
		return Color(cn);
	}
};


//////////////////////////////////////////////////////////////
// Class that writes graphs/geometric objects to a latex file
// Generates a file called: GeneratedGraphs.tex
//////////////////////////////////////////////////////////////
class WriteToLatex {
	std::ofstream m_file;
	bool m_isOpenPage;
	// x-coord unit = m_scaleX cms
	double m_scaleX;
	// y-coord unit = m_scaleY cms 
	double m_scaleY;

public:
	
	WriteToLatex(string s = "GeneratedGraphs.tex") : m_file(s), 
																				m_isOpenPage(false), 
																				m_scaleX(0.06),
																				m_scaleY(0.06)
	{
		m_file << "\\documentclass{beamer}" << endl;
		m_file << "\\usepackage[utf8]{inputenc}" << endl;
		m_file << "\\usepackage{tikz}" << endl;
		m_file << "\\begin{document}" << endl;
	}

	~WriteToLatex() {
		clearPage();
		m_file << "\\end{document}" << endl;
		m_file.close();
		cout << "Run 'make latex' to view pdf" << endl;
	}

	void clearPage() {
		if (m_isOpenPage) {
			m_file << "\\end{tikzpicture} \\end{center}" << endl;
			m_file << "\\end{frame}" << endl;
		}
		m_isOpenPage = false;
	}
	
	void initPage(string title = "") {
		clearPage();
		m_file << "\\begin{frame} {" << title << "}" << endl;
		m_file << "\\begin{center}\\begin{tikzpicture}[x=" << m_scaleX << "cm, y=" << m_scaleY << "cm]" << endl;
		m_isOpenPage = true;
	}

	void setScale(double sx, double sy) {
		assert(!m_isOpenPage);
		m_scaleX = sx;
		m_scaleY = sy;
	}

	void drawNode(GraphAttributes& ga, node v, std::string color = "black") {
		assert(m_isOpenPage && ga.has(GraphAttributes::nodeGraphics));
		std::string caption = ga.has(GraphAttributes::nodeLabel) ?  ga.label(v) : "";
		m_file << "\\draw[" << color << "] (" << ga.x(v) << "," << ga.y(v) << ") node[dotted, draw] {\\tiny "
					 <<  caption << "};" << endl;
	}

	void drawLine(IPoint src, IPoint dst, double opacity = 0.5) {
		std::string arrow = "";
		m_file << "\\draw[" << arrow << ", opacity=" << opacity << "] (" << src.m_x  << "," << src.m_y << ") -- (" 
					 << dst.m_x << "," << dst.m_y << ");" << endl; 
	}

	void drawEdge(GraphAttributes& ga, edge e, string color, double opacity = 0.5) {
		assert(m_isOpenPage && ga.has(GraphAttributes::nodeGraphics));
		std::string caption = ga.has(GraphAttributes::edgeLabel) ?  ga.label(e) : "";
		node u = e->source();
		node v = e->target();
		std::string arrow = "";
		if (ga.directed())
			arrow = "->,";
		m_file << "\\draw[" << arrow <<  color << ", opacity=" << opacity << "] (" << ga.x(u) + 2 << "," << ga.y(u) << ") -- (" 
					 << ga.x(v) - 2 << "," << ga.y(v) << ");" << endl; 
	}
	
	void drawColoredEdge(GraphAttributes& ga, edge e, Color c, double opacity = 0.5, std::string style = "") {
		int r = c.red(); int g = c.green(); int b = c.blue();
		m_file << "\\definecolor{currColor}{RGB}{" << r << "," << g << "," << b << "}" << endl;
		style += "currColor";
		drawEdge(ga, e, style, opacity);
	}

	void addPause() {
		m_file << "\\pause" << endl;
	}
	
	void drawPath(GraphAttributes& ga, vector<edge>& path, std::string color = "red") {
		assert(m_isOpenPage);
		std::string cdesc = "ultra thick," + color;
		for (edge e : path)
			drawEdge(ga, e, cdesc, 0.4);
	}
	
	void drawGraph(GraphAttributes& ga, bool newPage = true) {
		if (!m_isOpenPage || newPage)
			initPage();
		for (node v : ga.constGraph().nodes)
			drawNode(ga, v);
		for (edge e : ga.constGraph().edges)
			drawEdge(ga, e, "gray");
	}

	void drawPoint(double x, double y, string color = "black") {
		assert(m_isOpenPage);
		m_file << "\\filldraw [" <<  color << "](" <<  x << ", " << y << ") " <<  "circle (0.5pt);" << endl;
	}
	
	void drawSegment(double x1, double y1, double x2, double y2, string color = "black") {
		assert(m_isOpenPage);
		m_file << "\\draw [" <<  color << "](" <<  x1 << ", " << y1 << ") -- (" << x2 << ", " << y2 << ");" << endl;
	}

	void addLine(string s) {
		m_file << s  << endl;	
	}

	void drawCircle(IPoint ctr, int radius, Color c, string attr = "opacity=0.4") {
		assert(m_isOpenPage);
		int r = c.red(); int g = c.green(); int b = c.blue();
		m_file << "\\definecolor{currColor}{RGB}{" << r << "," << g << "," << b << "}" << endl;
		m_file << "\\draw[black, " << attr <<  ", fill=currColor]" << ctr << "circle (" << radius << ");" << endl;
	}

	void drawSquare(IPoint pivot, int side, Color c, string attr = "opacity=0.4") {
		assert(m_isOpenPage);
		int r = c.red(); int g = c.green(); int b = c.blue();
		m_file << "\\definecolor{currColor}{RGB}{" << r << "," << g << "," << b << "}" << endl;
		IPoint tr(pivot.m_x + side, pivot.m_y + side);
		m_file << "\\draw[black, " << attr << ", fill=currColor]" << pivot << "rectangle " << tr << ";" << endl;
	}
	
	void drawRectangle(IPoint bl, IPoint tr, Color c, string attr = "opacity=0.6") {
		assert(m_isOpenPage);
		int r = c.red(); int g = c.green(); int b = c.blue();
		m_file << "\\definecolor{currColor}{RGB}{" << r << "," << g << "," << b << "}" << endl;
		m_file << "\\draw[black, " << attr << ", fill=currColor]" << bl << "rectangle " << tr << ";" << endl;
	}
};


#endif
