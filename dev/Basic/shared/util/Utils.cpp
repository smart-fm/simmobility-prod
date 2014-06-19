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

inline void initRandomProvider(boost::thread_specific_ptr<boost::mt19937>& provider) {
    // The first time called by the current thread then just create one.
    if (!provider.get()) {
        provider.reset(new boost::mt19937(static_cast<unsigned> (std::time(0))));
    }
}

float Utils::generateFloat(float min, float max) {
	if (min == max){
		return min;
	}
    initRandomProvider(floatProvider);
    boost::uniform_real<float> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > 
        gen(*(floatProvider.get()), distribution);
    return gen();
}

int Utils::generateInt(int min, int max) {
    initRandomProvider(intProvider);
    boost::uniform_int<int> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > 
        gen(*(intProvider.get()), distribution);
    return gen();
}

double Utils::uRandom() {
//	initRandomProvider(floatProvider);
//	boost::uniform_int<> dist(0, RAND_MAX);
//	long int seed_ = dist(floatProvider.get());
	long int seed_ = Utils::generateInt(0,RAND_MAX);

	const long int M = 2147483647; // M = modulus (2^31)
	const long int A = 48271; // A = multiplier (was 16807)
	const long int Q = M / A;
	const long int R = M % A;
	seed_ = A * (seed_ % Q) - R * (seed_ / Q);
	seed_ = (seed_ > 0) ? (seed_) : (seed_ + M);
	return (double) seed_ / (double) M;
}

double Utils::nRandom(double mean, double stddev) {
	double r1 = uRandom(), r2 = uRandom();
	double r = -2.0 * log(r1);
	if (r > 0.0)
		return (mean + stddev * sqrt(r) * sin(2 * 3.1415926 * r2));
	else
		return (mean);
}

std::vector<std::string> Utils::parseArgs(int argc, char* argv[])
{
	std::vector<std::string> res;
	for (size_t i=0; i<argc; i++) {
		res.push_back(argv[i]);
	}
	return res;
}


void Utils::printAndDeleteLogFiles(const std::list<std::string>& logFileNames)
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


std::pair<double, double> Utils::parseScaleMinmax(const std::string& src)
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

double Utils::toFeet(const double meter) {
    return (meter * 3.2808399);
}

double Utils::toMeter(const double feet) {
    return (feet * 0.3048);
}
void Utils::convertStringToArray(std::string& str,std::vector<double>& array)
{
	std::string splitDelimiter = " ,";
	std::vector<std::string> arrayStr;
//	vector<double> c;

	// remove
	char chars[] = "#abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()-,;";
	for (unsigned int i = 0; i < strlen(chars); ++i)
	{
		str.erase (std::remove(str.begin(), str.end(), chars[i]), str.end());
	}
	boost::trim(str);
	boost::split(arrayStr, str, boost::is_any_of(splitDelimiter),boost::token_compress_on);
	for(int i=0;i<arrayStr.size();++i)
	{
		double res;
		try {
#if 1
			std::cout<<"<"<<arrayStr[i]<<">"<<std::endl;
#endif
				res = boost::lexical_cast<double>(arrayStr[i].c_str());
			}catch(boost::bad_lexical_cast&) {
				std::string str = "can not covert <" +str+"> to double.";
				throw std::runtime_error(str);
			}
			array.push_back(res);
	}
}
double Utils::urandom()
{
	return generateFloat(0,1);
}
int Utils::brandom(double prob)
{
	if (urandom() < prob) return (1);
		   else return 0;
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
