#include "Profiler.hpp"
#include "logging/Log.hpp"

#include <boost/lockfree/queue.hpp>

/* **********************************
 *     Basic Logger Implementation
 * **********************************
 */
std::string sim_mob::BasicLogger::newLine("\n");
sim_mob::Logger sim_mob::Logger::log;

sim_mob::BasicLogger::BasicLogger(std::string id_){
	reset();
	start = stop = totalTime = 0;
	started = false;
	id = id_;
	std::string path = id_ + ".txt";
	if(path.size()){
		InitLogFile(path);
	}
}
//sim_mob::Logger::Logger(sim_mob::Logger const&v)
//{}

sim_mob::BasicLogger::~BasicLogger(){
	onExit();
}

void sim_mob::BasicLogger::onExit()
{
	if(logFile.is_open()){
		flushLog();
		logFile.close();
	}
	for(outIt it(out_.begin()); it != out_.end(); safe_delete_item(it->second),it++);
}

///whoami
std::string sim_mob::BasicLogger::getId(){
	return id;
}

bool sim_mob::BasicLogger::isStarted(){
	return started;
}

///like it suggests, store the start time of the profiling
void sim_mob::BasicLogger::startProfiling(){
	started = true;

	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);

	start = tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		tm->tm_sec * 1000 + tv.tv_usec / 1000;
}

///save the ending time ...and .. if add==true add the value to the total time;
uint32_t sim_mob::BasicLogger::endProfiling(bool addToTotalTime_){
	if(!started){
		throw std::runtime_error("Logger Ended before Starting");
	}

	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);

	stop = tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		tm->tm_sec * 1000 + tv.tv_usec / 1000;

	uint32_t elapsed = stop - start;
	if(addToTotalTime_){
		addToTotalTime(elapsed);
	}
	return elapsed;
}

///add the given time to the total time
void sim_mob::BasicLogger::addToTotalTime(uint32_t value){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
//		Print() << "Logger "  << "[" << index << ":" << id << "] Adding " << value << " seconds to total time " << std::endl;
	totalTime+=value;
}

std::stringstream & sim_mob::BasicLogger::getOut(){
	boost::upgrade_lock<boost::shared_mutex> lock(mutexOutput);
	outIt it;
	boost::thread::id id = boost::this_thread::get_id();
	if((it = out_.find(id)) == out_.end()){
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock2(lock);
		std::stringstream* strm(new std::stringstream());
		out_.insert(std::make_pair(id, strm));
		return *strm;
	}
	return *(it->second);
}

unsigned int & sim_mob::BasicLogger::getTotalTime(){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
	return totalTime;
}

void printTime(struct tm *tm, struct timeval & tv, std::string id){
	sim_mob::Print() << "TIMESTAMP:\t  " << tm->tm_hour << std::setw(2) << ":" <<tm->tm_min << ":" << ":" <<  tm->tm_sec << ":" << tv.tv_usec << std::endl;
}

void sim_mob::BasicLogger::reset(){
	start = stop = totalTime = 0;
	started = false;
	id = "";
}

void  sim_mob::BasicLogger::InitLogFile(const std::string& path)
{
	logFile.open(path.c_str());
}

sim_mob::BasicLogger&  sim_mob::BasicLogger::operator<<(StandardEndLine manip) {
	// call the function, but we cannot return it's value
		manip(getOut());
	return *this;
}

void sim_mob::BasicLogger::flushLog()
{
	if ((logFile.is_open() && logFile.good()))
	{
		std::stringstream &out = getOut();
		{
			boost::unique_lock<boost::mutex> lock(flushMutex);
			logFile << out.str();
			logFile.flush();
			out.str(std::string());
		}
	}
	else
	{
		Warn() << "pathset profiler log ignored" << std::endl;
	}
}

/* ****************************************
 *     Default Logger wrap Implementation
 * ****************************************
 */
sim_mob::BasicLogger & sim_mob::Logger::operator[](const std::string &key)
{

	std::map<std::string, boost::shared_ptr<sim_mob::BasicLogger> >::iterator it = repo.find(key);
	if(it == repo.end()){
		boost::shared_ptr<sim_mob::BasicLogger> t(new sim_mob::BasicLogger(key));
		repo.insert(std::make_pair(key,t));
		return *t;
	}
	return *it->second;
}

sim_mob::Logger::~Logger(){
//	onExit();
}

/* *****************************
 *     Queued Implementation
 * *****************************
 */

//namespace{
//boost::lockfree::queue<std::pair<char*, std::ofstream * > > logQueue1(128);
//}
//
////boost::lockfree::queue<std::pair<void *, std::stringstream * > > sim_mob::LoggerQ::logQueue(128);
////boost::atomic<bool> sim_mob::LoggerQ::logDone(false);
////boost::shared_ptr<boost::thread> sim_mob::LoggerQ::flusher(new boost::thread(boost::bind(&LoggerQ::flushToFile)));
//sim_mob::LoggerQ::LoggerQ():Logger() ,logQueue(0),logDone(true){}//only for the instance
//sim_mob::LoggerQ::LoggerQ(sim_mob::LoggerQ const& v){}
//sim_mob::LoggerQ::LoggerQ(std::string id_):Logger(id_) ,logQueue(128),logDone(false){}
//sim_mob::LoggerQ::~LoggerQ()
//{
//	onExit();
//}
//
//sim_mob::Logger & sim_mob::LoggerQ::operator[](const std::string &key)
//{
//	boost::shared_ptr<sim_mob::Logger> res;
//	std::map<std::string, boost::shared_ptr<sim_mob::Logger> >::iterator it = repo.find(key);
//	res = it->second;
//	if(it == repo.end()){
//		res.reset(new sim_mob::LoggerQ(key));
//		repo.insert(std::make_pair(key,res));
//		//shifting the following from constructor. let's not start a thread unless it is expected to be required.
//		if(!flusher)
//		{
//			flusher.reset(new boost::thread(boost::bind(&LoggerQ::flushToFile,this)));
//		}
//	}
//	return *res;
//}
//
//void sim_mob::LoggerQ::flushToFile()
//{
////    void *value;
//	std::pair<void *, std::stringstream * > value;
//    while (!logDone)
//    {
//        while (logQueue.pop(value)){
//
//        	Print() << "poped out  " << value.first << "  and  " << value.second << "  to Q" << std::endl;
//        	std::ofstream *file = static_cast<std::ofstream *>(value.first);
//        	std::stringstream *buffer = /*static_cast<std::stringstream *>*/(value.second);
//        	if(buffer){
//        		buffer->str();
//        		*file << buffer->str();
//        		safe_delete_item(buffer);
//        	}
//        }
//        boost::this_thread::sleep(boost::posix_time::seconds(0.5));
//    }
//    //same thing as above , just to clear the queue after logFileCnt is set to true
//    while (logQueue.pop(value)){
//    	std::ofstream *file = static_cast<std::ofstream *>(value.first);
//    	std::stringstream *buffer = /*static_cast<std::stringstream *>*/(value.second);
//    	if(buffer){
//    		*file << buffer->str();
//    		safe_delete_item(buffer);
//    	}
//    }
//    Print() << "Out of flushToFile" << std::endl;
//}
//
//void sim_mob::LoggerQ::flushLog()
//{
////	boost::unique_lock<boost::mutex> lock();todo: between the above and below line, there could be another large amount of data inserted and flushLogQueued() invoked before changing the pointer
//	std::stringstream &out = getOut();
//	Print() << "Pushing " << &logFile << "  and  " << &out << "  to Q" << std::endl;
//	logQueue.push(std::make_pair(&logFile,&out));
//}
//
//void sim_mob::LoggerQ::onExit()
//{
//	logDone = true;
//	if(flusher){
//		flusher->join();
//
//	//test
//
//	if(logFile.is_open()){
//		flushLog();
//		logFile.close();
//	}
////	for(outIt it(out_.begin()); it != out_.end(); safe_delete_item(it->second),it++);
//	}
//}
