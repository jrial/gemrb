/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "Polygon.h"

#include "win32def.h"

#include "Interface.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace GemRB {

Gem_Polygon::Gem_Polygon(const Point* points, unsigned int cnt, Region *bbox)
: vertices(points, points + cnt)
{
	if(bbox) BBox=*bbox;
	else RecalcBBox();

	std::vector<Trapezoid> trapezoids = ComputeTrapezoids();
	std::vector<Trapezoid>::iterator iter;
	for (iter = trapezoids.begin(); iter != trapezoids.end(); ++iter)
	{
		int y_top = iter->y1 - BBox.y; // inclusive
		int y_bot = iter->y2 - BBox.y; // exclusive

		if (y_top < 0) y_top = 0;
		if (y_bot >= BBox.h) y_bot = BBox.h - 1;
		if (y_top >= y_bot) continue; // clipped

		int ledge = iter->left_edge;
		int redge = iter->right_edge;
		const Point& a = vertices[ledge];
		const Point& b = vertices[(ledge+1)%(Count())];
		const Point& c = vertices[redge];
		const Point& d = vertices[(redge+1)%(Count())];

		rasterData.reserve(rasterData.size() + (y_bot*2));

		for (int y = y_top; y < y_bot; ++y) {
			int py = y + BBox.y;

			int lt = (b.x * (py - a.y) + a.x * (b.y - py))/(b.y - a.y);
			int rt = (d.x * (py - c.y) + c.x * (d.y - py))/(d.y - c.y) + 1;

			lt -= BBox.x;
			rt -= BBox.x;

			if (lt < 0) lt = 0;
			if (rt >= BBox.w) rt = BBox.w - 1;
			if (lt >= rt) { continue; } // clipped

			// Draw a line from (y,lt) to (y,rt)
			rasterData.push_back(Point(lt, y));
			rasterData.push_back(Point(rt, y));
		}
	}
}

void Gem_Polygon::RecalcBBox()
{
	if (vertices.empty()) {
		BBox.x=BBox.y=BBox.w=BBox.h=0;
		return;
	}
	BBox.x = vertices[0].x;
	BBox.y = vertices[0].y;
	BBox.w = vertices[0].x;
	BBox.h = vertices[0].y;
	for(size_t i=1; i<vertices.size(); i++) {
		if(vertices[i].x<BBox.x) {
			BBox.x = vertices[i].x;
		}
		if(vertices[i].x>BBox.w) {
			BBox.w = vertices[i].x;
		}
		if(vertices[i].y<BBox.y) {
			BBox.y = vertices[i].y;
		}
		if(vertices[i].y>BBox.h) {
			BBox.h = vertices[i].y;
		}
	}
	BBox.w-=BBox.x;
	BBox.h-=BBox.y;
}

bool Gem_Polygon::PointIn(const Point &p) const
{
	if(!BBox.PointInside(p) ) return false;
	return PointIn(p.x, p.y);
}

bool Gem_Polygon::PointIn(int tx, int ty) const
{
	int  yflag0, yflag1, xflag0 , index;
	bool inside_flag = false;
	size_t count = vertices.size();

	if (count < 3) {
		return false;
	}
	index = 0;

	const Point* vtx0 = &vertices[count - 1];
	yflag0 = ( vtx0->y >= ty );
	const Point* vtx1 = &vertices[index];

	for (size_t j = count + 1; --j ;) {
		yflag1 = ( vtx1->y >= ty );
		if (yflag0 != yflag1) {
			xflag0 = ( vtx0->x >= tx );
			if (xflag0 == ( vtx1->x >= tx )) {
				if (xflag0)
					inside_flag = !inside_flag;
			} else {
				if (( vtx1->x -
					( vtx1->y - ty ) * ( vtx0->x - vtx1->x ) /
					( vtx0->y - vtx1->y ) ) >= tx) {
					inside_flag = !inside_flag;
				}
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
		vtx1 = &vertices[++index];
	}
	return inside_flag;
}

// returns twice the area of triangle a, b, c.
// (can also be negative depending on orientation of a,b,c)
static inline int area2(const Point& a, const Point& b, const Point& c)
{
	return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}


// return (c is to the left of a-b)
static inline bool left(const Point& a, const Point& b, const Point& c)
{
	return (area2(a, b, c) > 0);
}

// { return (c is collinear with a-b)
static inline bool collinear(const Point& a, const Point& b, const Point& c)
{
	return (area2(a, b, c) == 0);
}

// Find the intersection of two segments, if any.
// If the intersection is one of the endpoints, or the lines are
// parallel, false is returned.
// The point returned has the actual intersection coordinates rounded down
// to integers
static bool intersectSegments(const Point& a, const Point& b, const Point& c, const Point& d, Point& s)
{
	if (collinear(a, b, c) || collinear(a, b, d) ||
		collinear(c, d, a) || collinear(c, d, b))
		return false;

	if (!((left(a, b, c) != left(a, b, d)) &&
		(left(c, d, a) != left(c, d, b))))
		return false;

	long int A1 = area2(c, d, a);
	long int A2 = area2(d, c, b);

	s.x = (short) ((b.x*A1 + a.x*A2) / (A1 + A2));
	s.y = (short) ((b.y*A1 + a.y*A2) / (A1 + A2));

	return true;
}

// find the intersection of a segment with a horizontal scanline, if any
static bool intersectSegmentScanline(const Point& a, const Point& b, int y, int& x)
{
	int y1 = a.y - y;
	int y2 = b.y - y;

	if (y1 * y2 > 0) return false;
	if (y1 == 0 && y2 == 0) return false;

	x = a.x + ((b.x - a.x)*y1)/(y1-y2);
	return true;
}


struct ScanlineInt {
	int x;
	int pi;
	const Gem_Polygon* p;

	bool operator<(const ScanlineInt& i2) const
	{
		if (x < i2.x)
			return true;

		if (x > i2.x)
			return false;

		const Point& a = p->vertices[pi];
		const Point& b = p->vertices[(pi+1)%(p->Count())];
		const Point& c = p->vertices[i2.pi];
		const Point& d = p->vertices[(i2.pi+1)%(p->Count())];

		int dx1 = a.x - b.x;
		int dx2 = c.x - d.x;
		int dy1 = a.y - b.y;
		int dy2 = c.y - d.y;

		if (dy1 < 0) {
			dy1 *= -1;
			dx1 *= -1;
		}

		if (dy2 < 0) {
			dy2 *= -1;
			dx2 *= -1;
		}

		if (dx1 * dy2 > dx2 * dy1) return true;

		return false;
	}

};

std::vector<Trapezoid> Gem_Polygon::ComputeTrapezoids() const
{
	std::vector<Trapezoid> trapezoids;
	size_t count = vertices.size();

	if (count < 3) return trapezoids;

	std::vector<int> ys;
	ys.reserve(2*count);

	// y coords of vertices
	unsigned int i;

	for (i = 0; i < count; ++i)
		ys.push_back(vertices[i].y);

	Point p;
	// y coords of self-intersections
	for (unsigned int i1 = 0; i1 < count; ++i1) {
		const Point& a = vertices[i1];
		const Point& b = vertices[(i1+1)%count];

		// intersections with horizontal lines don't matter
		if (a.y == b.y) continue;

		for (unsigned int i2 = i1+2; i2 < count; ++i2) {
			const Point& c = vertices[i2];
			const Point& d = vertices[(i2+1)%count];
			
			// intersections with horizontal lines don't matter
			if (c.y == d.y) continue;

			if (intersectSegments(a, b, c, d, p)) {
				ys.push_back(p.y);
			}
		}
	}

	std::sort(ys.begin(), ys.end());

	std::vector<ScanlineInt> ints;
	ints.reserve(count);

	Trapezoid t;
	ScanlineInt is;
	is.p = this;
	std::vector<Trapezoid>::iterator iter;

	int cury = ys[0];

	// TODO: it's possible to keep a set of 'active' edges and only check
	// scanline intersections of those edges.

	for (size_t yi = 0; yi < ys.size() - 1;) {
		while (yi < ys.size() && ys[yi] == cury) ++yi;
		if (yi == ys.size()) break;
		int nexty = ys[yi];

		t.y1 = cury;
		t.y2 = nexty;

		// Determine all scanline intersections at level nexty.
		// This includes edges which have their lower vertex at nexty,
		// but excludes edges with their upper vertex at nexty.
		// (We're taking the intersections along the 'upper' edge of 
		// the nexty scanline.)
		ints.clear();
		for (i = 0; i < count; ++i) {
			const Point& a = vertices[i];
			const Point& b = vertices[(i+1)%count];

			if (a.y == b.y) continue;

			if (a.y == nexty) {
				if (b.y - nexty < 0) {
					is.x = a.x;
					is.pi = i;
					ints.push_back(is);			
				}
			} else if (b.y == nexty) {
				if (a.y - nexty < 0) {
					is.x = b.x;
					is.pi = i;
					ints.push_back(is);	
				}
			} else {
				int x;
				if (intersectSegmentScanline(a, b, nexty, x)) {
					is.x = x;
					is.pi = i;
					ints.push_back(is);
				}
			}
		}

		std::sort(ints.begin(), ints.end());
		unsigned int newtcount = (unsigned int) (ints.size() / 2);

		for (i = 0; i < newtcount; ++i) {
			t.left_edge = ints[2*i].pi;
			t.right_edge = ints[2*i+1].pi;

			
			bool found = false;

			// merge trapezoids with old one if it's just a continuation
			for (iter = trapezoids.begin(); iter != trapezoids.end(); ++iter) {
				Trapezoid& oldt = *iter;
				if (oldt.y2 == cury &&
					oldt.left_edge == t.left_edge &&
					oldt.right_edge == t.right_edge)
				{
					oldt.y2 = nexty;
					found = true;
					break;
				}
			}

			if (!found)
				trapezoids.push_back(t);
		}

		// Done with this strip
		cury = nexty;
	}

	return trapezoids;
}


// wall polygons
void Wall_Polygon::SetBaseline(const Point &a, const Point &b)
{
	if ((a.x<b.x) || ((a.x==b.x) && (a.y<b.y)) ) {
		base0=a;
		base1=b;
		return;
	}
	base0=b;
	base1=a;
}

bool Wall_Polygon::PointCovered(const Point &p) const
{
	if (wall_flag&WF_DISABLED)
		return false;
	if (wall_flag&WF_BASELINE) {
		if (base0.x > base1.x)
			return left(base0, base1, p);
		else
			return left(base1, base0, p);
	}
	return true;
}

bool Wall_Polygon::PointCovered(int tx, int ty) const
{
	Point p((short) tx, (short) ty);
	return PointCovered(p);
}


}
