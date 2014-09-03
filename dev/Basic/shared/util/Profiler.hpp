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
	sim_mob::Logger::log[prof] << "My First" << " test" << std::endl;;
	sim_mob::Logger::log[prof] << sim_mob::Logger::log["hey"].outPut();
	std::stringstream  s;
	sim_mob::Logger::log[prof] << s.str();

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

/**********************************
 ******* Ticking class ************
 **********************************/
class Profiler{
	///stores start and end of profiling
	boost::atomic_uint32_t start, lastTick,total;
	///is the profiling object started profiling?
	boost::atomic_bool started;
	const std::string id;
	///	returns the current time in us
	const uint32_t getTime();
public:
	///	begin: should begin time or not
	Profiler(const std::string id, bool begin_ = true);
	Profiler(const Profiler &t);
	/// return the elapse time since begin() and disable profiling unless explicitly bein()'ed
	uint32_t end();
	///	reset all the members to 0
	void reset();
	/**
	 * return the difference between current time and previous call to tick()(or begin().
	 * optionally(as in the argument list):
	 * 1- accumulate this difference(addUp)
	 * 2- dont tick anymore
	 * so measuring the time elapsed in executing a method can be done as follows:
	 * tick();
	 * operation();
	 * elapsed_time = tick();
	 *
	 * measuring time elapsed for multiple operations is as follows:
	 * reset()//optional
	 * tick();
	 * operation_1();
	 * tick(true);
	 *...
	 *tick();
	 *operation_2();
	 *tick(true);
	 *...
	 *tick();
	 *operation_3();
	 *tick(true);
	 *elapsed_time = getAddUp();
	 */
	uint32_t tick(bool addToTotal = false);
	///	add the given time to total time variable
	uint32_t addUp(const uint32_t value);
	///	manually set the total value
	void setAddUp(const uint32_t value);
	///	return the total(accumulated) time
	uint32_t getAddUp();

};

/**********************************
 ******* Basic Logging Engine******
 *********************************/
class BasicLogger {
private:

	///total time measured by all profilers
	uint32_t totalTime;

	///	used for index
	boost::mutex flushMutex;

	///	used for output stream
	boost::shared_mutex mutexOutput;

	///	the mandatory id given to this BasicLogger
	std::string id;

	///	profilers container
	std::map<const std::string, Profiler> profilers;

	///	one buffer is assigned to each thread writing to the file
	std::map<boost::thread::id, std::stringstream*> out;

	/// Sentry class objects are created at each line and are destryped upon when the statement ends(";")
	/// this will allow grouping of multiple << operators without worrying about multithreading issues.
	class Sentry
		{
			std::stringstream &out;
			BasicLogger &basicLogger;
			public:
			Sentry(BasicLogger & basicLogger_,std::stringstream &out_):out(out_),basicLogger(basicLogger_){};
			Sentry(const Sentry& t):basicLogger(t.basicLogger), out(t.out){}

			//This is the type of std::cout
			typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
			//This is the function signature of std::endl and some other manipulators
			typedef CoutType& (*StandardEndLine)(CoutType&);
			///	operator overload for std::endl
			Sentry& operator<<(StandardEndLine manip) {
				manip(out);
				return *this;
			}
			template <typename T>
			///	operator overload
			Sentry & operator<< (const T& val)
			{
				out << val;
				return *this;
			}

			~Sentry()
			{
				// by some googling this estimated hardcode value promises less cycles to write to a file
				if(out.tellp() > 512000/*500KB*/)
				{
					basicLogger.flushLog();
				}
			}
		};

protected:

	///	easy reading
	typedef std::map<boost::thread::id, std::stringstream*>::iterator outIt;

	///	flush the log streams into the output buffer-Default version
	virtual void flushLog();

	/**
	 * return the buffer corresponding to the calling thread.If the buffer doesn't exist, this method will create, register and returns a new buffer.
	 * \param renew should the buffer be replaced with a new one
	 */
	std::stringstream * getOut(bool renew= false);

	/**
	 * removes the profiler ,with the given id,
	 * from the list of profilers.
	 * returns the total elapsed time since it was started profiling
	 */
	uint32_t endProfiler(const std::string id);

	///	print time in HH:MM::SS::uS todo:needs improvement
	static void printTime(struct tm *tm, struct timeval & tv, std::string id);

	void initLogFile(const std::string& path);

	///logger
	std::ofstream logFile;

public:
	/**
	 * @param id arbitrary identification for this object
	 */
	BasicLogger(std::string id);

	///	copy constructor
	BasicLogger(const sim_mob::BasicLogger& value);

	///	destructor
	virtual ~BasicLogger();

	/**
	 * simple interface to log a profiling output with a simple message
	 */
	inline void profileMsg(const std::string msg, uint32_t value)
	{
		*this << msg << " : " << value << "\n";
	}

	/**
	 * returns a profiler based on id,
	 * if not found, a new profiler will be : generated, started and returned
	 * timer: should start timer or prefer to start it manually
	 */
	sim_mob::Profiler & prof(const std::string id, bool timer = true);

	//This is the type of std::cout
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
	//This is the function signature of std::endl and some other manipulators
	typedef CoutType& (*StandardEndLine)(CoutType&);

	///	operator overload for std::endl(just if someone starts as std::endl as the first input to << operator)
	Sentry operator<<(StandardEndLine manip) {
		std::stringstream *out = getOut();
		manip(*out);
		return Sentry(*this,*out);
	}

	///	operator overload.	write the log items to buffer
	template <typename T>
	Sentry operator<< (const T& val)
	{
		//Sentry t(*this,*getOut());
		//return(t << val);
		// return t;
		return (Sentry(*this,*getOut())<< val);
	}
	//for debugging purpose only
	static std::map <boost::thread::id, int> threads;
	static unsigned long int ii;
	static int flushCnt;
};

/**********************************
 ******* Queued Logging Engine*****
 **********************************/
class QueuedLogger :public BasicLogger
{
	///	queued buffer
	typedef boost::lockfree::queue<std::stringstream *> LockFreeQ;
	LockFreeQ logQueue;
	///	indicates if all the loggerq objects have died and it is time for the flusher thread to join
	boost::atomic<bool> logDone;
	///	logQueue's consumer thread
	boost::shared_ptr<boost::thread> flusher;
public:
	QueuedLogger(std::string id);
	~QueuedLogger();
	///	flush the log streams into the output buffer-Default version
	void flushLog();
	///	flush the file into the file from the queue
	void flushToFile();
};


/**********************************
 ******* Logging Wrapper **********
 **********************************/
class Logger {

protected:
	///	repository of profilers. each profiler is distinguished by a file name!
	std::map<const std::string, boost::shared_ptr<sim_mob::BasicLogger> > repo;
public:
	static sim_mob::Logger log;
	virtual sim_mob::BasicLogger & operator[](const std::string &key);
	virtual ~Logger();
};
/*************************************************************************
 * Change the value of this typedef to enable the selected Logging module
 * Currently available modules are:
 * BasicLogger
 * QueuedLogger
 **************************************************************************/
typedef BasicLogger LogEngine;
}//namespace
