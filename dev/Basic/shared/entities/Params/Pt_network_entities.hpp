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
	float link_travel_time;
	std::String edge_id;
	float wait_time;
	float walk_time;
	float transit_time;
	float transfer_penalty;
	float day_transit_time;
	float dist;
};

class pt_network_vertices{
public:
	pt_network_edges();
	virtual ~pt_network_edges();

private:
	std::String stop_id;
	std::String stop_code;
	std::String stop_name;
	double stop_latitude;
	double stop_longitude;
	std::String ezlink_name;

};
