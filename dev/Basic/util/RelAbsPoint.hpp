/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "geospatial/Point2D.hpp"
#include "DynamicVector.hpp"

namespace sim_mob
{

class AgentPackageManager;
class RoadNetworkPackageManager;

/**
 * Class which wraps an absolute value of a Point and automatically converts to/from relative coordinates.
 *
 * \note
 * I'm making this header-only for now to make it simpler to develop. Will need to migrate to its own
 * separate *.cpp file if nothing breaks.
 */
class RelAbsPoint {
public:
	//Retrieval functions
	double getAbsX() const { return abs.x; }
	double getAbsY() const { return abs.y; }
	double getRelX() const { return rel.x; }
	double getRelY() const { return rel.y; }

	//Mutators operate on pairs of points.
	// Note that setting relative will re-generate absolute, and vice-versa.
	void setAbs(double x, double y) {
		abs.x = x;
		abs.y = y;
		rebuild_rel();
	}
	void setRel(double x, double y) {
		rel.x = x;
		rel.y = y;
		rebuild_abs();
	}

	//Helper mutators which operate on single components. Generally you should prefer setting both
	// at once.
	void setAbsX(double x) { setAbs(x, getAbsY()); }
	void setAbsY(double y) { setAbs(getAbsX(), y); }
	void setRelX(double x) { setRel(x, getRelY()); }
	void setRelY(double y) { setRel(getRelX(), y); }

	//Change our coordinate definition.
	// This should be called each time the current polyline changes.
	void changeCoords(const sim_mob::Point2D& polyStart, const sim_mob::Point2D& polyEnd) {
		build_scale_dir(polyStart, polyEnd);
		//rebuild_rel();
	}

private:
	//Rebuild the absolute component
	void rebuild_abs() {
		abs.x = rel.x*scaleDir.x-rel.y*scaleDir.y;
		abs.y = rel.x*scaleDir.y+rel.y*scaleDir.x;
	}

	//Rebuild the relative component
	void rebuild_rel() {
		rel.x =  abs.x*scaleDir.x+abs.y*scaleDir.y;
		rel.y = -abs.x*scaleDir.y+abs.y*scaleDir.x;
	}

	//Rebuild our "directional" components and save into scaleDir
	void build_scale_dir(const sim_mob::Point2D& polyStart, const sim_mob::Point2D& polyEnd) {
		double xDir = polyEnd.getX()-polyStart.getX();
		double yDir = polyEnd.getY()-polyStart.getY();
		int polyLineSegLength = sqrt(xDir*xDir+yDir*yDir);
		if (polyLineSegLength==0) {
			scaleDir.x = scaleDir.y = 0;
		} else {
			scaleDir.x = xDir/polyLineSegLength;
			scaleDir.y = yDir/polyLineSegLength;
		}
	}

private:
	//We store both updated representations at all times.
	DPoint abs;
	DPoint rel;

	//How we scale these points
	DPoint scaleDir;

	//add by xuyan
public:
	friend class AgentPackageManager;
	friend class RoadNetworkPackageManager;
};


}


