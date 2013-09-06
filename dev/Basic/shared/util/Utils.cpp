/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Utils.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on June 12, 2013, 4:59 PM
 */


#include "Utils.hpp"
#include <fstream>
#include <boost/random.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include "util/LangHelpers.hpp"

using namespace sim_mob;

// Thread local random numbers. 
boost::thread_specific_ptr<boost::mt19937> floatProvider;
boost::thread_specific_ptr<boost::mt19937> intProvider;

inline void InitRandomProvider(boost::thread_specific_ptr<boost::mt19937>& provider) {
    // The first time called by the current thread then just create one.
    if (!provider.get()) {
        provider.reset(new boost::mt19937(static_cast<unsigned> (std::time(0))));
    }
}

float Utils::GenerateFloat(float min, float max) {
    InitRandomProvider(floatProvider);
    boost::uniform_real<float> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(*(floatProvider.get()), distribution);
    return gen();
}

int Utils::GenerateInt(int min, int max) {
    InitRandomProvider(intProvider);
    boost::uniform_int<int> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > gen(*(intProvider.get()), distribution);
    return gen();
}


std::vector<std::string> Utils::ParseArgs(int argc, char* argv[])
{
	std::vector<std::string> res;
	for (size_t i=0; i<argc; i++) {
		res.push_back(argv[i]);
	}
	return res;
}


void Utils::PrintAndDeleteLogFiles(const std::list<std::string>& logFileNames)
{
	//This can take some time.
	timeval start_time, end_time;
	gettimeofday(&start_time, nullptr);
	std::cout <<"Merging output files, this can take several minutes...\n";

	//One-by-one.
	std::ofstream out("out.txt", std::ios::trunc|std::ios::binary);
	if (!out.good()) { throw std::runtime_error("Error: Can't write to file."); }
	for (std::list<std::string>::const_iterator it=logFileNames.begin(); it!=logFileNames.end(); it++) {
		std::cout <<"  Merging: " <<*it <<std::endl;
		std::ifstream src(it->c_str(), std::ios::binary);
		if (src.fail()) { throw std::runtime_error("Error: Can't read from file."); }

		//If it's good, this part's easy.
		out <<src.rdbuf();
		src.close();
	}
	out.close();

	gettimeofday(&end_time, nullptr);
	std::cout <<"Files merged; took " <<Utils::diff_ms(end_time, start_time) <<"ms\n";
}


int Utils::diff_ms(timeval t1, timeval t2) {
	return (((t1.tv_sec - t2.tv_sec) * 1000000) + (t1.tv_usec - t2.tv_usec) / 1000);
}

double Utils::diff_ms_db(timeval t1, timeval t2) {
	return (((t1.tv_sec - t2.tv_sec) * 1000000.0) + (t1.tv_usec - t2.tv_usec) / 1000.0);
}


StopWatch::StopWatch() : now(0), end(0), running(false) {
}

void StopWatch::Start() {
    if (!running) {
        //get start time of the simulation.
        time(&now);
        running = true;
    }
}

void StopWatch::Stop() {
    if (running) {
        time(&end);
        running = false;
    }
}

double StopWatch::GetTime() {
    if (!running) {
        return difftime(end, now);
    }
    return -1.0f;
}
