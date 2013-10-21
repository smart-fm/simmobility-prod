//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Utils.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on June 12, 2013, 4:59 PM
 */


#include "Utils.hpp"

#include <fstream>
#include <stdexcept>

#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/algorithm/string.hpp>

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
	StopWatch sw;
	sw.start();
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

	sw.stop();
	std::cout <<"Files merged; took " <<sw.getTime() <<"s\n";
}


std::pair<double, double> Utils::parse_scale_minmax(const std::string& src)
{
	//Find and split on colons, spaces.
	std::vector<std::string> words;
	boost::split(words, src, boost::is_any_of(": "), boost::token_compress_on);

	//Double-check.
	if (words.size()!=2) {
		throw std::runtime_error("Scale min/max paramters not formatted correctly.");
	}

	//Now prepare our return value.
	double min = boost::lexical_cast<double>(words.front());
	double max = boost::lexical_cast<double>(words.back());
	return std::make_pair(min, max);
}



StopWatch::StopWatch() : now(0), end(0), running(false) {
}

void StopWatch::start() {
    if (!running) {
        //get start time of the simulation.
        time(&now);
        running = true;
    }
}

void StopWatch::stop() {
    if (running) {
        time(&end);
        running = false;
    }
}

double StopWatch::getTime() const {
    if (!running) {
        return difftime(end, now);
    }
    return -1.0f;
}
