class pt_network_edges{
public:
	pt_network_edges();
	virtual ~pt_network_edges();

	double getDayTransitTime() const {
		return day_transit_time;
	}

	void setDayTransitTime(double dayTransitTime) {
		day_transit_time = dayTransitTime;
	}

	double getDist() const {
		return dist;
	}

	void setDist(double dist) {
		this->dist = dist;
	}

	int getEdgeId() const {
		return edge_id;
	}

	void setEdgeId(int edgeId) {
		edge_id = edgeId;
	}

	std::String getEndStop() const {
		return end_stop;
	}

	void setEndStop(std::String endStop) {
		end_stop = endStop;
	}

	double getLinkTravelTime() const {
		return link_travel_time;
	}

	void setLinkTravelTime(double linkTravelTime) {
		link_travel_time = linkTravelTime;
	}

	std::String getServiceLines() const {
		return r_service_lines;
	}

	void setServiceLines(std::String serviceLines) {
		r_service_lines = serviceLines;
	}

	std::String getR_Type() const {
		return r_type;
	}

	void setR_Type(std::String type) {
		r_type = type;
	}

	std::String getRoadEdgeId() const {
		return road_edge_id;
	}

	void setRoadEdgeId(std::String roadEdgeId) {
		road_edge_id = roadEdgeId;
	}

	int getRoadIndex() const {
		return road_index;
	}

	void setRoadIndex(int roadIndex) {
		road_index = roadIndex;
	}

	std::String getStartStop() const {
		return start_stop;
	}

	void setStartStop(std::String startStop) {
		start_stop = startStop;
	}

	double getTransferPenalty() const {
		return transfer_penalty;
	}

	void setTransferPenalty(double transferPenalty) {
		transfer_penalty = transferPenalty;
	}

	double getTransitTime() const {
		return transit_time;
	}

	void setTransitTime(double transitTime) {
		transit_time = transitTime;
	}

	double getWaitTime() const {
		return wait_time;
	}

	void setWaitTime(double waitTime) {
		wait_time = waitTime;
	}

	double getWalkTime() const {
		return walk_time;
	}

	void setWalkTime(double walkTime) {
		walk_time = walkTime;
	}

private:
	std::String start_stop;  // Alphanumeric id
	std::String end_stop;    // Alphanumeric id
 	std::String r_type;      // Service Line type, can be "BUS","LRT","WALK"
	int road_index;          // Index for road type 0 for BUS , 1 for LRT , 2 for Walk
	std::String road_edge_id; // Strings of passing road segments Ex: 4/15/35/43
	std::String r_service_lines;
	double link_travel_time;
	int  edge_id;
	double wait_time;
	double walk_time;
	double transit_time;
	double transfer_penalty;
	double day_transit_time;
	double dist;
};

class Pt_network_vertices{
public:
	Pt_network_vertices();
	virtual ~Pt_network_vertices();

	std::String getEzlinkName() const {
		return ezlink_name;
	}

	void setEzlinkName(std::String ezlinkName) {
		ezlink_name = ezlinkName;
	}

	std::String getStopCode() const {
		return stop_code;
	}

	void setStopCode(std::String stopCode) {
		stop_code = stopCode;
	}

	std::String getStopDesc() const {
		return stop_desc;
	}

	void setStopDesc(std::String stopDesc) {
		stop_desc = stopDesc;
	}

	std::String getStopId() const {
		return stop_id;
	}

	void setStopId(std::String stopId) {
		stop_id = stopId;
	}

	double getStopLatitude() const {
		return stop_latitude;
	}

	void setStopLatitude(double stopLatitude) {
		stop_latitude = stopLatitude;
	}

	double getStopLongitude() const {
		return stop_longitude;
	}

	void setStopLongitude(double stopLongitude) {
		stop_longitude = stopLongitude;
	}

	std::String getStopName() const {
		return stop_name;
	}

	void setStopName(std::String stopName) {
		stop_name = stopName;
	}

	int getStopType() const {
		return stop_type;
	}

	void setStopType(int stopType) {
		stop_type = stopType;
	}

private:
	std::String stop_id;
	std::String stop_code;
	std::String stop_name;
	double stop_latitude;
	double stop_longitude;
	std::String ezlink_name;
	int stop_type;
	std::String stop_desc;
};
