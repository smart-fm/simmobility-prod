#pragma once
#include <stdint.h>
#include <fstream>
#include <boost/thread/mutex.hpp>
#include "logging/Log.hpp"//todo remove this when unnecessary
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
namespace sim_mob {
/**
 * Authore: Vahid
 * A class to log custom messages and measure the start and end of any operation.
 * Features:
 * unlike the current profiling system, it can be used independent of any specific module.
 * It can return the elapsed time, * it can save any formatted string to the desired output file
 * same object can be used multiple times to accumulate loging information of several operations.
 *
 * Note: example use case :
 * 	std::string prof("PathSetManagerProfiler");
	sim_mob::Logger::instance[prof] << "My First" << " test" << std::endl;;
	sim_mob::Logger::instance[prof] << sim_mob::Logger::instance["hey"].outPut();
	std::stringstream  s;
	sim_mob::Logger::instance[prof] << s.str();

 * Some implementation Details on the lockfree version
 * this class accepts buffers of data submitted to it via different threads and pushes them into
 * a circular (lock free) queue to be dumped into the destination file.
 * A thread is waiting for the other side of the queue waiting for the buffers to arrive(pop) and dump the
 * buffers into their corresponding file. This way we can achieve:
 * 1- A separate thread doing IO without holding up other threads.
 * 2- Circular queue does not require locking when pushing into the queue.
 * 3- If the need be, there can be multiple threads poping out of the queue. boost::lockfree::queue implementation is multi-producer/multi-consumer version.
 * 4- Needless to mention that the Logger handles one buffer per thread rather than using one buffer, therefore there is no locking(ecxcept for minimal
 * thread management)
 *
 * Note:Alternative approach:
 * Instead of having a queue handling buffers and files, there is an alternative approach to create one file per thread and join the file whenever required.
 */

class Logger {
private:
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
	std::ofstream logFile;

	//lock free version members
	///	queued buffer
	static boost::lockfree::queue<void* > logQueue;
	///	indicates end of logging in the application
	static boost::atomic<bool> pushDone;
	///	logQueue's sole consumer thread
	static boost::shared_ptr<boost::thread> flusher;
	//	lock free version members...

	///	function pointer to the corrct version of flusher
	boost::function<void(void)> flushLog;
	///	initializer used to configure the object for the desirable implementation
	boost::function<void(void)> onExit;

	///	repository of profilers. each profiler is distinguished by a file name!
	static std::map<const std::string, boost::shared_ptr<sim_mob::Logger> > repo;
	///	flush the log streams into the output buffer-Default version
	void flushLogDef();
	///	flush the log streams into the output buffer-Default version
	void flushLogQueued();
	///	flush the file into the file from the queue
	void flushToFile();
	///	return the buffer corresponding to the calling thread.If the buffer doesn't exist, this method will create, register and returns a new buffer.
	std::stringstream & getOut();

	/**
	 * @param init Start profiling if this is set to true
	 * @param id arbitrary identification for this object
	 * @param logger file name where the output stream is written
	 */
	Logger(std::string id);
	Logger();
	//needs improvement
	static void printTime(struct tm *tm, struct timeval & tv, std::string id);

	///reset all the parameters
	void reset();

	void InitLogFile(const std::string& path);

	///whoami
	std::string getId();

	///whoami
	int getIndex();

	///is this Logger started
	bool isStarted();

	//This is the type of std::cout
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
	//This is the function signature of std::endl and some other manipulators
	typedef CoutType& (*StandardEndLine)(CoutType&);
public:
	static std::string newLine;
	static sim_mob::Logger instance;
	//copy constructor is required by static std::map<std::string, sim_mob::Logger> repo;
	Logger(const sim_mob::Logger& value);
	void initDef();
	void initQueued();
	sim_mob::Logger & operator[](const std::string &key);
	void onExitDef();
	void onExitQueued();
	virtual ~Logger();

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
	Logger& operator<<(StandardEndLine manip);

	template <typename T>
	sim_mob::Logger & operator<< (const T& val)
	{
		//debug
		if(onExit.empty()){
			Print() << "OnExit empty" << std::endl;
		}
		std::stringstream &out = sim_mob::Logger::getOut();
		out << val;
		if(out.tellp() > 512000/*500KB*/){
			flushLog();
		}
		else{
			Print() << out.tellp() << std::endl;
		}
		return *this;
	}
};

}//namespace
