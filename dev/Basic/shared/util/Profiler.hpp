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
 * Although it is mainly a time profiler, it can be used to profile other parameters with some other tool and dump the result as string stream to the object of this BaseProfiler
 *
 * Note: example use case :
 * 	std::string prof("PathSetManagerProfiler");
	sim_mob::BaseProfiler::instance[prof] << "My First" << " test" << std::endl;;
	sim_mob::BaseProfiler::instance[prof] << sim_mob::BaseProfiler::instance["hey"].outPut();
	std::stringstream  s;
	sim_mob::BaseProfiler::instance[prof] << s.str();
 */
class BaseProfiler {

	///total time measured by all profilers
	uint32_t totalTime;
	///total number of profilers
	static int totalProfilers;

	///	used for index
	boost::mutex flushMutex;

	///	used for total time
	boost::mutex mutexTotalTime;

	///	used for output stream
	boost::shared_mutex mutexOutput;

	///stores start and end of profiling
	uint32_t start, stop;

	///the profiling object id
	int index;

	std::string id;

	///is the profiling object started profiling?
	bool started;

	///print output
//	std::stringstream output;
  std::map<boost::thread::id, std::stringstream* > out_;
  typedef std::map<boost::thread::id, std::stringstream* >::iterator outIt;

	///logger
	std::ofstream LogFile;

	///flush the log streams into the file

	static std::map<const std::string, boost::shared_ptr<sim_mob::BaseProfiler> > repo;
	void flushLog();
	///	return the buffer corresponding to the calling thread.If the buffer doesn't exist, this method will create, register and returns a new buffer.
	std::stringstream & getOut();

	/**
	 * @param init Start profiling if this is set to true
	 * @param id arbitrary identification for this object
	 * @param logger file name where the output stream is written
	 */
	BaseProfiler(std::string id_,bool init = false);
	BaseProfiler();
	//needs improvement
	static void printTime(struct tm *tm, struct timeval & tv, std::string id);

	///reset all the parameters
	void reset();

	void InitLogFile(const std::string& path);

	///whoami
	std::string getId();

	///whoami
	int getIndex();

	///is this BaseProfiler started
	bool isStarted();

	//This is the type of std::cout
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
	//This is the function signature of std::endl and some other manipulators
	typedef CoutType& (*StandardEndLine)(CoutType&);

public:
	static std::string newLine;
	static sim_mob::BaseProfiler instance;
	//copy constructor is required by static std::map<std::string, sim_mob::BaseProfiler> repo;
	BaseProfiler(const sim_mob::BaseProfiler& value);
	sim_mob::BaseProfiler & operator[](const std::string &key);
	~BaseProfiler();

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
	/// This method defines an operator<< to take in std::endl
	BaseProfiler& operator<<(StandardEndLine manip);

	template <typename T>
	sim_mob::BaseProfiler & operator<< (const T& val)
	{
		std::stringstream &out = sim_mob::BaseProfiler::getOut();
		out << val;
		if(out.tellp() > 512000/*500KB*/){
			flushLog();
		}
		return *this;
	}
};
typedef BaseProfiler Profiler;
}//namespace
