#pragma once
#include <stdint.h>
#include <fstream>
#include <boost/thread/mutex.hpp>
#include "logging/Log.hpp"//todo remove this when unnecessary
namespace sim_mob {
/**
 * Authore: Vahid
 * A simple 'time' Profiling tool to measure the start and end of any operation.
 * Features:
 * unlike the current profiling system, it can be used independent of any specific module.
 * It can return the elapsed time, * it can save any formatted string to the desired output file
 * it can be reused(by calling a reset)
 * same object can be used multiple times (without resetting) to accumulate profiling time of several operations.
 * multiple instances of profiler objects can form a hierarchy and report to the higher level and thereby generate accumulated time from diesired parts of various methods
 * the amount of buffering to the output file can be configured.
 * Although it is mainly a time profiler, it can be used to profile other parameters with some other tool and dump the result as string stream to the object of this Profiler
 *
 * Note: example use case :
 * 	std::string prof("PathSetManagerProfiler");
	sim_mob::Profiler::instance[prof] << "My First" << " test" << std::endl;;
	sim_mob::Profiler::instance[prof] << sim_mob::Profiler::instance["hey"].outPut();
	std::stringstream  s;
	sim_mob::Profiler::instance[prof] << s.str();
 */
class Profiler {

	///total time measured by all profilers
	uint32_t totalTime;
	///total number of profilers
	static int totalProfilers;

	///	used for index
	boost::mutex mutex_;

	///	used for total time
	boost::mutex mutexTotalTime;

	///	used for output stream
	boost::mutex mutexOutput;

	///stores start and end of profiling
	uint32_t start, stop;

	///the profiling object id
	int index;

	std::string id;

	///is the profiling object started profiling?
	bool started;

	///print output
	std::stringstream output;

	///for efficiency
	int outputSize;

	///logger
	std::ofstream LogFile;

	///flush the log streams into the file

	static std::map<const std::string, boost::shared_ptr<sim_mob::Profiler> > repo;
	void flushLog();

	/**
	 * @param init Start profiling if this is set to true
	 * @param id arbitrary identification for this object
	 * @param logger file name where the output stream is written
	 */
	Profiler(std::string id_,bool init = false);
	Profiler();
	//needs improvement
	static void printTime(struct tm *tm, struct timeval & tv, std::string id);

	///reset all the parameters
	void reset();

	void InitLogFile(const std::string& path);

	///whoami
	std::string getId();

	///whoami
	int getIndex();

	///is this Profiler started
	bool isStarted();
	///getoutput stream
	std::stringstream & outPut();

	//Type of cout.
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
	//Type of std::endl and some other manipulators.
	typedef CoutType& (*StandardEndLine)(CoutType&);

public:
	static std::string newLine;
	static sim_mob::Profiler instance;
	//copy constructor is required by static std::map<std::string, sim_mob::Profiler> repo;
	Profiler(const sim_mob::Profiler& value);
	sim_mob::Profiler & operator[](const std::string &key);
	~Profiler();

	///like it suggests, store the start time of the profiling
	void startProfiling();

	/**
	 * save the ending time
	 * @param  addToTotalTime if true, add the return value to the total time also
	 * @return the elapsed time since the last call to startProfiling()
	 */
	uint32_t endProfiling(bool addToTotalTime = false);

	/**
	 *add the given time to the total time
	 *@param  value add to totalTime meber variable
	 */
	void addToTotalTime(uint32_t value);

	unsigned int & getTotalTime();

	Profiler& operator<<(StandardEndLine manip);

	template <typename T>
	sim_mob::Profiler & operator<< (const T& val)
	{
		boost::unique_lock<boost::mutex> lock(mutexOutput);
		output << val;
		if(output.tellp() > 512000/*500KB*/){
			flushLog();
		}
		return *this;
	}

};
}//namespace
