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
 * functions without infor
 */
void generate_required_parameters_without_infor();
void do_map_decompose_without_infor();
void result_analysis_without_infor();
void map_decompose_without_infor();

/**
 * functions with infor
 */
void generate_required_parameters_with_infor();
void do_map_decompose_with_infor();
void result_analysis_with_infor();
void map_decompose_with_infor();

/**
 * other supporting functions
 */
void show_menu();
bool check_user_inputs(char* argv[]);

int main(int argc, char* argv[])
{
	if (argc != 7)
	{
		show_menu();
		throw "Parameters in main function is not enough, program exit";
	}

	if (check_user_inputs(argv) == false)
	{
		show_menu();
		throw "Parameters setting not correct, program exit";
	}

	//generate decomposition parameters and store them in file
	if (infor_file.compare("NULL") == 0 || infor_file.compare("null") == 0)
	{
		map_decompose_without_infor();
	}
	else
	{
		map_decompose_with_infor();
	}
}

void map_decompose_without_infor()
{
	generate_required_parameters_without_infor();
	do_map_decompose_without_infor();
	result_analysis_without_infor();
}

void map_decompose_with_infor()
{
	generate_required_parameters_with_infor();
	do_map_decompose_with_infor();
	result_analysis_with_infor();
}

/**
 * without infor
 */
void generate_required_parameters_without_infor()
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

	if (system(command_hmetis_input.c_str()) != 0) throw "command FlowAndInforToHMETIS failed";

	if (system(command_metis_input.c_str()) != 0) throw "command FlowAndInforToMETIS failed";
}

void do_map_decompose_without_infor()
{
	string command_hmetis = "resources/hmetis2.0pre1";

	stringstream ss_0_1;
	ss_0_1 << nparts;

	if (nparts <= 16)
	{
		int loops = log2(nparts);
		double refined_load_imbalance = pow(load_imbalance, 1.0 / loops);
		if (nparts == 2) refined_load_imbalance = (load_imbalance - 1.0) / 2 + 1.0;
		int imbalance = (int) ((refined_load_imbalance - 1.0) * 100);

		stringstream ss_0;
		ss_0 << imbalance;

		command_hmetis += " -ufactor=" + ss_0.str();
		command_hmetis += " -otype=cut";
		command_hmetis += " -reconst";
		command_hmetis += " -kwayrefine";
	}
	else
	{
		int imbalance = (int) ((load_imbalance - 1.0) * 100);
		imbalance = imbalance < 5 ? 5 : imbalance;

		stringstream ss_0;
		ss_0 << imbalance;

		command_hmetis += " -ufactor=" + ss_0.str();
		command_hmetis += " -otype=cut";
		command_hmetis += " -ptype=kway";
		command_hmetis += " -rtype=kpfast";
	}

	command_hmetis += " -nruns=20";

	command_hmetis += " " + output_folder + "/hmetis.input";
	command_hmetis += " " + ss_0_1.str();

	if (system(command_hmetis.c_str()) != 0) throw "command hmetis2 failed";

	string command_metis = "resources/gpmetis ";

	int metis_imbalance = (int) ((load_imbalance - 1.0) * 1000);
	stringstream ss_1;
	ss_1 << metis_imbalance;
	command_metis += " -ufactor=" + ss_1.str();
	command_metis += " " + output_folder + "/metis.input";
	command_metis += " " + ss_0_1.str();

	std::cout << command_metis << std::endl;

	if (system(command_metis.c_str()) != 0) throw "command gpmetis failed";
}

void result_analysis_without_infor()
{
	stringstream ss_0_1;
	ss_0_1 << nparts;

	string command_analysis_hmetis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_hmetis += " " + node_file;
	command_analysis_hmetis += " " + flow_file;
	command_analysis_hmetis += " NULL";
	command_analysis_hmetis += " " + output_folder + "/hmetis.input.part." + ss_0_1.str();
	command_analysis_hmetis += " " + output_folder + "/hmetis.analysis";

	if (system(command_analysis_hmetis.c_str()) != 0) throw "command FlowAndInforPartitionResultAnalysis failed";

	string command_analysis_metis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_metis += " " + node_file;
	command_analysis_metis += " " + flow_file;
	command_analysis_metis += " NULL";
	command_analysis_metis += " " + output_folder + "/metis.input.part." + ss_0_1.str();
	command_analysis_metis += " " + output_folder + "/metis.analysis";

	if (system(command_analysis_metis.c_str()) != 0) throw "command FlowAndInforPartitionResultAnalysis failed";
}

/**
 * With infor
 */
void generate_required_parameters_with_infor()
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

	if (system(command_hmetis_input.c_str()) != 0) throw "command FlowAndInforToHMETIS failed";

	if (system(command_metis_input.c_str()) != 0) throw "command FlowAndInforToMETIS failed";
}

void do_map_decompose_with_infor()
{
	string command_hmetis = "resources/hmetis2.0pre1";

	if (nparts == 2)
	{
		int imbalance = (int) ((load_imbalance - 1.0) * 100 / 2);
		stringstream ss_0;
		ss_0 << imbalance;

		command_hmetis += " -ufactor=" + ss_0.str();
		command_hmetis += " -otype=cut";
		command_hmetis += " -reconst";
		command_hmetis += " -kwayrefine";
	}
	else
	{
		int imbalance = (int) ((load_imbalance - 1.0) * 100);
		imbalance = imbalance < 5 ? 5 : imbalance;

		stringstream ss_0;
		ss_0 << imbalance;

		command_hmetis += " -ufactor=" + ss_0.str();
		command_hmetis += " -otype=soed";
		command_hmetis += " -ptype=kway";
		command_hmetis += " -rtype=kpfast";
	}

	command_hmetis += " -nruns=20";
	command_hmetis += " " + output_folder + "/hmetis.input";

	stringstream ss_0_1;
	ss_0_1 << nparts;
	command_hmetis += " " + ss_0_1.str();

	if (system(command_hmetis.c_str()) != 0) throw "command hmetis2 failed";

	string command_metis = "resources/gpmetis ";

	int metis_imbalance = (int) ((load_imbalance - 1.0) * 1000);
	stringstream ss_1;
	ss_1 << metis_imbalance;
	command_metis += " -ufactor=" + ss_1.str();
	command_metis += " " + output_folder + "/metis.input";
	command_metis += " " + ss_0_1.str();

	if (system(command_metis.c_str()) != 0) throw "command gpmetis failed";
}

void result_analysis_with_infor()
{
	stringstream ss_0_1;
	ss_0_1 << nparts;

	string command_analysis_hmetis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_hmetis += " " + node_file;
	command_analysis_hmetis += " " + flow_file;
	command_analysis_hmetis += " " + infor_file;
	command_analysis_hmetis += " " + output_folder + "/hmetis.input.part." + ss_0_1.str();

	command_analysis_hmetis += " " + output_folder + "/hmetis.analysis";

	if (system(command_analysis_hmetis.c_str()) != 0) throw "command FlowAndInforPartitionResultAnalysis failed";

	string command_analysis_metis = "java -jar resources/FlowAndInforPartitionResultAnalysis.jar";
	command_analysis_metis += " " + node_file;
	command_analysis_metis += " " + flow_file;
	command_analysis_metis += " " + infor_file;
	command_analysis_metis += " " + output_folder + "/metis.input.part." + ss_0_1.str();

	command_analysis_metis += " " + output_folder + "/metis.analysis";

	if (system(command_analysis_metis.c_str()) != 0) throw "command FlowAndInforPartitionResultAnalysis failed";
}

bool is_file_or_folder_exist(char* filename)
{
	struct stat buffer;
	if (stat(filename, &buffer) == 0) return true;
	return false;
}

/* argv[1] : node.weight
 * argv[2] : flow.data.size
 * argv[3] : infor.data.size
 * argv[4] : output folder
 * argv[5] : nparts          (>=2)
 * argv[6] : load imbalance  (>1)
 */

void show_menu()
{
	std::cout << "-------------------------------------------" << std::endl;
	std::cout << "Required Parameters" << std::endl;
	std::cout << "argv[1] : the input file describing node.weight" << std::endl;
	std::cout << "argv[2] : the input file describing flow.data" << std::endl;
	std::cout << "argv[3] : the input file describing infor.data, if no such file, set NULL" << std::endl;
	std::cout << "argv[4] : the output folder" << std::endl;
	std::cout << "argv[5] : nparts >= 2" << std::endl;
	std::cout << "argv[6] : load imbalance >= 1" << std::endl;
}

bool check_user_inputs(char* argv[])
{

	//input processing
	node_file = (argv[1]);
	flow_file = (argv[2]);
	infor_file = (argv[3]);
	output_folder = (argv[4]);

	nparts = boost::lexical_cast<int>(argv[5]);
	load_imbalance = boost::lexical_cast<double>(argv[6]);

	if (nparts < 2)
	{
		std::cout << "nparts can not < 2" << std::endl;
		return false;
	}

	if (load_imbalance < 1)
	{
		std::cout << "load_imbalance can not < 1" << std::endl;
		return false;
	}

	//file and folder check
	if (is_file_or_folder_exist(argv[1]) == false)
	{
		std::cout << "node_file not existing" << std::endl;
		return false;
	}

	if (is_file_or_folder_exist(argv[2]) == false)
	{
		std::cout << "flow_file not existing" << std::endl;
		return false;
	}

	if (infor_file.compare("NULL") != 0 && infor_file.compare("null") != 0)
	{
		if (is_file_or_folder_exist(argv[3]) == false)
		{
			std::cout << "output_folder not existing" << std::endl;
			return false;
		}
	}

	if (is_file_or_folder_exist(argv[4]) == false)
	{
		std::cout << "output_folder not existing" << std::endl;
		return false;
	}

	return true;
}
