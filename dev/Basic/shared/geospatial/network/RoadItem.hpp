//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{

class RoadItem
{
private:

	/**Unique identifier for the road item*/
	unsigned int roadItemId;

	/**Indicates the id of the geometry information*/
	unsigned int geometryId;

	/**Indicates the id of the poly-line for the road item*/
	unsigned int polyLineId;

	/**Indicates the id of the road section to which the road item belongs*/
	unsigned int roadSectionId;

public:

	RoadItem(unsigned int id, unsigned int geomteryId, unsigned int polyLineId, unsigned int roadSectionId);

	virtual ~RoadItem();

	unsigned int getRoadItemId() const;
	void setRoadItemId(unsigned int roadItemId);

	unsigned int getGeometryId() const;
	void setGeometryId(unsigned int geometryId);

	unsigned int getPolyLineId() const;
	void setPolyLineId(unsigned int polyLineId);

	unsigned int getRoadSectionId() const;
	void setRoadSectionId(unsigned int roadSectionId);
};
}

