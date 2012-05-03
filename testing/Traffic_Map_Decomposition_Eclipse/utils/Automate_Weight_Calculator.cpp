/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Automate_Weight_Calculator.hpp"

namespace sim_mob_partitioning {

void Automate_Weight_Calculator::generate_node_weight(std::string node_weight_file, std::string node_file, std::string section_file)
{
	vector<std::string> all_nodes;
	map<std::string, int> node_weight;

	//init node weight
	std::string buffer_line;
	ifstream infile;
	infile.open(node_file.c_str());

	while (!infile.eof())
	{
		getline(infile, buffer_line); // Saves the line in STRING.

		std::vector<std::string> strs;
		boost::split(strs, buffer_line, boost::is_any_of(","));

		if(strs.size() < 2)
			continue;

		node_weight[strs[0]] = 0;
		all_nodes.push_back(strs[0]);
	}

	infile.close();

	//update node weight
	ifstream sectionfile;
	sectionfile.open(section_file.c_str());
	while (!sectionfile.eof())
	{
		getline(sectionfile, buffer_line); // Saves the line in STRING.

		std::vector<std::string> strs;
		boost::split(strs, buffer_line, boost::is_any_of(","));

		if (strs.size() < 7)
			continue;

		int lane = boost::lexical_cast<int>(strs[2]);
		double length = boost::lexical_cast<double>(strs[7]);

		int weight_add = (int) (0.5 * lane * length);

		map<std::string, int>::iterator itr_l = node_weight.find(strs[5]);
		if (itr_l != node_weight.end())
			node_weight[strs[5]] += weight_add;

		map<std::string, int>::iterator itr_r = node_weight.find(strs[6]);
		if (itr_r != node_weight.end())
			node_weight[strs[6]] += weight_add;
	}

	sectionfile.close();

	FileOutputHelper file_helper;
	file_helper.openFile(node_weight_file);

	for (std::vector<std::string>::iterator it = all_nodes.begin(); it != all_nodes.end(); ++it)
	{
		std::string node_id = *(it);
		int weight = node_weight[node_id];

		stringstream ss;
		ss << node_id << "," << weight;
		file_helper.output_to_file(ss.str());
	}

	file_helper.closeFile();

}

void Automate_Weight_Calculator::generate_section_weight(std::string section_weight_file, std::string node_file, std::string section_file)
{
	FileOutputHelper file_helper;
	file_helper.openFile(section_weight_file);

	std::string buffer_line;
	ifstream infile;
	infile.open(section_file.c_str());

	while (!infile.eof())
	{
		getline(infile, buffer_line); // Saves the line in STRING.

		if (boost::starts_with(buffer_line, "#"))
			continue;

		stringstream ss;

		std::vector<std::string> strs;
		boost::split(strs, buffer_line, boost::is_any_of(","));

		if (strs.size() < 7)
			continue;

		//from node + to node + size of lanes
		ss << strs[5] << "," << strs[6] << "," << strs[2];
		file_helper.output_to_file(ss.str());
	}

	file_helper.closeFile();
	infile.close();
}
}
