class pt_network_edges{
public:
	pt_network_edges();
	virtual ~pt_network_edges();

private:
	std::String start_stop;
	std::String end_stop;
	std::String r_type;
	int road_index;
	std::String road_edge_id;
	std::String r_service_lines;
	double link_travel_time;
	std::String edge_id;
	double wait_time;
	double walk_time;
	double transit_time;
	double transfer_penalty;
	double day_transit_time;
	double dist;
};

class pt_network_vertices{
public:
	pt_network_vertices();
	virtual ~pt_network_vertices();

private:
	std::String stop_id;
	std::String stop_code;
	std::String stop_name;
	double stop_latitude;
	double stop_longitude;
	std::String ezlink_name;
	std::String stop_type;
	std::String stop_desc;
};
