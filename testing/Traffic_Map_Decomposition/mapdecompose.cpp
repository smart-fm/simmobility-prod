/*
 * mapdecompose.cpp
 *
 *  Created on: 21-Mar-2012
 *      Author: xuyan
 */

/**
 * argv[1] : node.weight
 * argv[2] : flow.data.size
 * argv[3] : infor.data.size
 * argv[4] : output folder
 * argv[5] : nparts          (>=2)
 * argv[6] : load imbalance  (>1)
 */

#include "all_includes.hpp"

using namespace std;

std::string node_file;
std::string flow_file;
std::string infor_file;
std::string output_folder;
int nparts;
double load_imbalance;

bool is_file_or_folder_exist(char* filename);

/**
 * functions without flow
 */
void generate_required_parameters_without_flow();
void do_map_decompose_without_flow();
void result_analysis_without_flow();
void map_decompose_without_flow();

/**
 * functions with flow
 */
void generate_required_parameters_with_flow();
void do_map_decompose_with_flow();
void result_analysis_with_flow();
void map_decompose_with_flow();

int main(int argc, char* argv[])
{
	if (argc != 7) {
		throw "Parameters in main function is not enough";
	}

	//input processing
	node_file = (argv[1]);
	flow_file = (argv[2]);
	infor_file = (argv[3]);
	output_folder = (argv[4]);

	nparts = boost::lexical_cast<int>(argv[5]);
	load_imbalance = boost::lexical_cast<double>(argv[6]);

	//file and folder check
	if (is_file_or_folder_exist(argv[1]) == false) {
		throw "node_file not existing";
	}

	if (is_file_or_folder_exist(argv[2]) == false) {
		throw "flow_file not existing";
	}

	if (is_file_or_folder_exist(argv[4]) == false) {
		throw "output_folder not existing";
	}

	//generate decomposition parameters and store them in file
	if (infor_file.compare("NULL") == 0 || infor_file.compare("null") == 0) {
		map_decompose_without_flow();
	}
	else {
		if (is_file_or_folder_exist(argv[3]) == false) {
			throw "infor_file not existing";
		}

		map_decompose_with_flow();
	}
}

void map_decompose_without_flow()
{
	generate_required_parameters_without_flow();
	do_map_decompose_without_flow();
	result_analysis_without_flow();
}

void map_decompose_with_flow()
{
	generate_required_parameters_with_flow();
	do_map_decompose_with_flow();
	result_analysis_with_flow();
}

/**
 * without flow
 */
void generate_required_parameters_without_flow()
{
	string command_hmetis_input = "java -jar resources/FlowAndInforToHMETIS.jar";
	command_hmetis_input += " " + node_file;
	command_hmetis_input += " " + flow_file;
	command_hmetis_input += " NULL";
	command_hmetis_input += " " + output_folder + "//hmetis.input";

	string command_metis_input = "java -jar resources/FlowAndInforToMETIS.jar";
	command_metis_input += " " + node_file;
	command_metis_input += " " + flow_file;
	command_metis_input += " " + output_folder + "//metis.input";

	if (system(command_hmetis_input.c_str()) != 0)
		throw "command failed";

	if (system(command_metis_input.c_str()) != 0)
		throw "command failed";
}

void do_map_decompose_without_flow()
{
	string command_hmetis = "resources/hmetis2.0pre1";

	stringstream ss_0_1;
	ss_0_1 << nparts;

	//	command_hmetis += " -ptype=kway";

	if (nparts <= 16) {
		int loops = log2(nparts);
		double refined_load_imbalance = pow(load_imbalance, 1.0 / loops);

		if(nparts == 2)
			refined_load_imbalance = (load_imbalance - 1.0) / 2 + 1.0;

		int imbalance = (int) ((refined_load_imbalance - 1.0) * 100);

		stringstream ss_0;
		ss_0 << imbalance;

		command_hmetis += " -ufactor=" + ss_0.str();
		command_hmetis += " -otype=cut";
		command_hmetis += " -reconst";
		command_hmetis += " -kwayrefine";
	}
	else {
		int imbalance = (int) ((load_imbalance - 1.0) * 100);
		imbalance = imbalance < 5 ? 5 : imbalance;

		stringstream ss_0;
		ss_0 << imbalance;

		command_hmetis += " -ufactor=" + ss_0.str();
		command_hmetis += " -otype=cut";
		command_hmetis += " -ptype=kway";
		command_hmetis += " -rtype=kpfast";
	}

	command_hmetis += " -nruns=10";

	command_hmetis += " " + output_folder + "/hmetis.input";
	command_hmetis += " " + ss_0_1.str();

	if (system(command_hmetis.c_str()) != 0)
		throw "command failed";

	string command_metis = "resources/gpmetis ";

	int metis_imbalance = (int) ((load_imbalance - 1.0) * 1000);
	stringstream ss_1;
	ss_1 << metis_imbalance;
	command_metis += " -ufactor=" + ss_1.str();
	command_metis += " " + output_folder + "/metis.input";
	command_metis += " " + ss_0_1.str();

	std::cout << command_metis << std::endl;

	if (system(command_metis.c_str()) != 0)
		throw "command failed";
}

void result_analysis_without_flow()
{
	stringstream ss_0_1;
	ss_0_1 << nparts;

	string command_analysis_hmetis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_hmetis += " " + node_file;
	command_analysis_hmetis += " " + flow_file;
	command_analysis_hmetis += " NULL";
	command_analysis_hmetis += " " + output_folder + "/hmetis.input.part." + ss_0_1.str();
	command_analysis_hmetis += " " + output_folder + "/hmetis.analysis";

	if (system(command_analysis_hmetis.c_str()) != 0)
		throw "command failed";

	string command_analysis_metis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_metis += " " + node_file;
	command_analysis_metis += " " + flow_file;
	command_analysis_metis += " NULL";
	command_analysis_metis += " " + output_folder + "/metis.input.part." + ss_0_1.str();
	command_analysis_metis += " " + output_folder + "/metis.analysis";

	if (system(command_analysis_metis.c_str()) != 0)
		throw "command failed";
}

/**
 * With Flow
 */
void generate_required_parameters_with_flow()
{
	string command_hmetis_input = "java -jar resources/FlowAndInforToHMETIS.jar";
	command_hmetis_input += " " + node_file;
	command_hmetis_input += " " + flow_file;
	//	command_hmetis_input += " NULL";// + infor_file;
	command_hmetis_input += " " + infor_file;
	command_hmetis_input += " " + output_folder + "//hmetis.input";

	string command_metis_input = "java -jar resources/FlowAndInforToMETIS.jar";
	command_metis_input += " " + node_file;
	command_metis_input += " " + flow_file;
	command_metis_input += " " + output_folder + "//metis.input";

	if (system(command_hmetis_input.c_str()) != 0)
		throw "command failed";

	if (system(command_metis_input.c_str()) != 0)
		throw "command failed";
}

void do_map_decompose_with_flow()
{
	string command_hmetis = "resources/hmetis2.0pre1";

	int imbalance = (int) ((load_imbalance - 1.0) * 100);
	imbalance = imbalance < 5 ? 5 : imbalance;

	stringstream ss_0;
	ss_0 << imbalance;

	stringstream ss_0_1;
	ss_0_1 << nparts;

	command_hmetis += " -ufactor=" + ss_0.str();
	//	command_hmetis += " -ptype=kway";

	if (nparts == 2) {
		command_hmetis += " -otype=cut";
		command_hmetis += " -reconst";
		command_hmetis += " -kwayrefine";
	}
	else {
		command_hmetis += " -otype=soed";
		command_hmetis += " -ptype=kway";
		command_hmetis += " -rtype=kpfast";
	}

	command_hmetis += " -nruns=20";

	command_hmetis += " " + output_folder + "/hmetis.input";
	command_hmetis += " " + ss_0_1.str();

	if (system(command_hmetis.c_str()) != 0)
		throw "command failed";

	string command_metis = "resources/gpmetis ";

	int metis_imbalance = (int) ((load_imbalance - 1.0) * 1000);
	stringstream ss_1;
	ss_1 << metis_imbalance;
	command_metis += " -ufactor=" + ss_1.str();
	command_metis += " " + output_folder + "/metis.input";
	command_metis += " " + ss_0_1.str();

	if (system(command_metis.c_str()) != 0)
		throw "command failed";
}

void result_analysis_with_flow()
{
	stringstream ss_0_1;
	ss_0_1 << nparts;

	string command_analysis_hmetis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_hmetis += " " + node_file;
	command_analysis_hmetis += " " + flow_file;
	command_analysis_hmetis += " " + infor_file;
	command_analysis_hmetis += " " + output_folder + "/hmetis.input.part." + ss_0_1.str();

	command_analysis_hmetis += " " + output_folder + "/hmetis.analysis";

	if (system(command_analysis_hmetis.c_str()) != 0)
		throw "command failed";

	string command_analysis_metis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_metis += " " + node_file;
	command_analysis_metis += " " + flow_file;
	command_analysis_metis += " " + infor_file;
	command_analysis_metis += " " + output_folder + "/metis.input.part." + ss_0_1.str();

	command_analysis_metis += " " + output_folder + "/metis.analysis";

	if (system(command_analysis_metis.c_str()) != 0)
		throw "command failed";
}

bool is_file_or_folder_exist(char* filename)
{
	struct stat buffer;
	if (stat(filename, &buffer) == 0)
		return true;
	return false;
}
