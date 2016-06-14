#include "Profiler.hpp"
#include "logging/Log.hpp"

#include <boost/lockfree/queue.hpp>
#include <boost/foreach.hpp>
//#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/chrono/system_clocks.hpp>

using namespace boost::posix_time;

/* **********************************
 *     Basic Logger Implementation
 * **********************************
 */
boost::shared_mutex sim_mob::Logger::instanceMutex;
std::map <boost::thread::id, int> sim_mob::BasicLogger::threads= std::map <boost::thread::id, int>();//for debugging only
int sim_mob::BasicLogger::flushCnt = 0;
unsigned long int sim_mob::BasicLogger::ii = 0;

void printTime(boost::chrono::system_clock::time_point t)
{
    using namespace boost::chrono;
    time_t c_time = boost::chrono::system_clock::to_time_t(t);
    std::tm* tmptr = std::localtime(&c_time);
    boost::chrono::system_clock::duration d = t.time_since_epoch();
    std::cout << tmptr->tm_hour << ':' << tmptr->tm_min << ':' << tmptr->tm_sec
              << '.' << (d - boost::chrono::duration_cast<boost::chrono::microseconds>(d)).count() << "\n";
}

sim_mob::Profiler::Profiler(const Profiler &t):
		start(t.start),lastTick(t.lastTick),
		totalTime(t.totalTime),total(t.total.load()), started(t.started.load()), id(t.id)
{
}



sim_mob::Profiler::Profiler(const std::string id, bool begin_):id(id){
	boost::shared_mutex instanceMutex;
	started = 0;
	total = 0;
	if(begin_)
	{
		start = lastTick = boost::chrono::system_clock::now();
	}
//	printTime(start);
}

//uint64_t sim_mob::Profiler::tick(bool addToTotal){
	std::pair <boost::chrono::microseconds,
	boost::chrono::microseconds> sim_mob::Profiler::tick(bool addToTotal){
	boost::chrono::system_clock::time_point thisTick = getTime();
	boost::chrono::microseconds elapsedStart = boost::chrono::duration_cast<boost::chrono::microseconds>(thisTick - start);
	boost::chrono::microseconds elapsedLastTick = boost::chrono::duration_cast<boost::chrono::microseconds>(thisTick - lastTick);
	std::pair <boost::chrono::microseconds, boost::chrono::microseconds> res = std::make_pair(elapsedStart,elapsedLastTick);

	if(addToTotal){
		addUpTime(elapsedLastTick);
	}
	lastTick = thisTick;
	return res;
}

//todo in end, provide output to accumulated total also
boost::chrono::microseconds sim_mob::Profiler::end(){
	boost::chrono::nanoseconds lapse = getTime() - start;
	return boost::chrono::duration_cast<boost::chrono::microseconds>(lapse);
}

boost::chrono::microseconds sim_mob::Profiler::addUpTime(const boost::chrono::microseconds value){
	totalTime+=value;
	return totalTime;
}

uint32_t sim_mob::Profiler::addUp(uint32_t value)
{
	total +=value;
	return total;

}

boost::chrono::microseconds sim_mob::Profiler::getAddUpTime(){
	return totalTime;
}

uint32_t sim_mob::Profiler::getAddUp(){
	return total;
}

boost::chrono::system_clock::time_point sim_mob::Profiler::getTime()
{
	return boost::chrono::system_clock::now();
}


sim_mob::Sentry::Sentry(BasicLogger & basicLogger_,std::stringstream *out_):out(*out_),basicLogger(basicLogger_),copy(false){};
sim_mob::Sentry::Sentry(const Sentry& t):basicLogger(t.basicLogger), out(t.out),copy(true){}

sim_mob::Sentry& sim_mob::Sentry::operator<<(StandardEndLine manip)
{
	manip(out);
	return *this;
}


sim_mob::Sentry::~Sentry()
{
	//if the buffer size has reached its limit, dump it to the file otherwise leave it to accumulate.
	// by some googling this estimated hard-code value promises less cycles to write to a file
	if(out.tellp() > 512000/*500KB*/ && copy)
	{
		basicLogger.flushLog(out);
	}
}

sim_mob::BasicLogger::BasicLogger(std::string id_){
	id = id_;
	//simple check to see if the id can be used like a file name with a 3 letter extension, else append .txt
	std::string path = (id_[id.size() - 4] == '.' ? id_ : (id_ + ".txt"));
	if(path.size()){
		initLogFile(path);
	}
}

sim_mob::BasicLogger::~BasicLogger(){

	if(!profilers.empty()){
		std::map<const std::string, Profiler>::iterator it(profilers.begin()),itEnd(profilers.end());
		for(;it != itEnd; it++)
		{
			boost::chrono::microseconds lifeTime = it->second.end() ;
			*this << it->first << ": [totalTime AddUp : " << it->second.getAddUpTime().count() << "],[total accumulator : " << it->second.getAddUp() << "]  total object lifetime : " <<  lifeTime.count() << "\n";
		}
	}
	if(id == "real_time_travel_time"){
		std::cout << "~BasicLogger() " << id << std::endl;
	}
	flush();
	if (logFile.is_open()) {
		logFile.close();
	}
	for (outIt it(out.begin()); it != out.end();safe_delete_item(it->second), it++);
}

std::stringstream * sim_mob::BasicLogger::getOut(bool renew){
	boost::upgrade_lock<boost::shared_mutex> lock(mutexOutput);
	std::stringstream *res = nullptr;
	outIt it;
	boost::thread::id id = boost::this_thread::get_id();
	if((it = out.find(id)) == out.end()){
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock2(lock);
		res = new std::stringstream();
		out.insert(std::make_pair(id, res));
		if(this->id == std::string("realtime_travel_time"))
		{
			std::cout << "realtime_travel_time buffer new size = " << out.size() << std::endl;
		}
	}
	else
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock2(lock);
		res = it->second;
		if (renew)
		{
			res = it->second = new std::stringstream();			
		}
	}
	return res;
}

sim_mob::Profiler & sim_mob::BasicLogger::prof(const std::string id, bool timer)
{
	std::map<const std::string, Profiler>::iterator it(profilers.find(id));
	if(it != profilers.end())
	{
		return it->second;
	}
	else
	{
		profilers.insert(std::pair<const std::string, Profiler>(id, Profiler(id, timer)));
		it = profilers.find(id);
	}
	return it->second;
}

//void printTime(struct tm *tm, struct timeval & tv, std::string id){
//	sim_mob::Print() << "TIMESTAMP:\t  " << tm->tm_hour << std::setw(2) << ":" <<tm->tm_min << ":" << ":" <<  tm->tm_sec << ":" << tv.tv_usec << std::endl;
//}

void  sim_mob::BasicLogger::initLogFile(const std::string& path)
{
	logFile.open(path.c_str());
//	if ((logFile.is_open() && logFile.good())){
//		std::cout << "Logfile for " << path << "  creatred" << std::endl;
//	}
}

void sim_mob::BasicLogger::flushLog(std::stringstream &out)
{
	if ((logFile.is_open() && logFile.good()))
	{
		{
			boost::unique_lock<boost::mutex> lock(flushMutex);
			logFile << out.str();
			logFile.flush();
		}
			out.str(std::string());
	}
	else
	{
		Warn() << "pathset profiler log ignored" << std::endl;
	}
}

/* *****************************
 *     Queued Implementation
 * *****************************
 */

sim_mob::QueuedLogger::QueuedLogger(std::string id_):BasicLogger(id_) ,logQueue(128),logDone(false)
{
	flusher.reset(new boost::thread(boost::bind(&QueuedLogger::flushToFile,this)));
}
sim_mob::QueuedLogger::~QueuedLogger()
{
	logDone = true;
	if(flusher){
		flusher->join();
	}
}

void sim_mob::QueuedLogger::flushToFile()
{
	std::stringstream * buffer;
    while (!logDone)
    {
        while (logQueue.pop(buffer)){

        	// std::cout << "poped out  " << buffer << "  to Q" << std::endl;
        	if(buffer){
        		buffer->str();
        		logFile << buffer->str();
        		logFile.flush();
        		safe_delete_item(buffer);
        	}
        }
        boost::this_thread::sleep(boost::posix_time::seconds(0.5));
    }
    //same thing as above , just to clear the queue after logFileCnt is set to true
    while (logQueue.pop(buffer)){
    	if(buffer){
    		logFile << buffer->str();
    		logFile.flush();
    		safe_delete_item(buffer);
    	}
    }
}

void sim_mob::QueuedLogger::flushLog()
{
	std::stringstream *out = getOut(true);
	logQueue.push(out);
}


void sim_mob::BasicLogger::flush()
{
	if(id == "real_time_travel_time"){
		std::cout << "flush()  " << id << std::endl;
	}
	if (logFile.is_open()) {
		outIt it = out.begin();
		if(it== out.end())
		{
			std::cout << "flush()  No outputs for   " << id << std::endl;
		}
		for(; it!= out.end(); it++)
		{
			flushLog(*(it->second));
		}
	}
	else
	{
		std::cout << "flush()  logFile not open   " << id << std::endl;
	}
}

/* ****************************************
 *     Default Logger wrap Implementation
 * ****************************************
 */
sim_mob::Logger::~Logger()
{
	typedef std::map<std::string, boost::shared_ptr<sim_mob::BasicLogger> >::value_type Pair;
	BOOST_FOREACH(Pair&item,repo)
	{
		item.second.reset();
	}
	repo.clear();
}
sim_mob::BasicLogger & sim_mob::Logger::operator()(const std::string &key)
{
	std::map<std::string, boost::shared_ptr<sim_mob::BasicLogger> >::iterator it = repo.find(key);
	if(it == repo.end()){
		boost::shared_ptr<sim_mob::BasicLogger> t(new sim_mob::LogEngine(key));
		repo.insert(std::make_pair(key,t));
		if(key == "real_time_travel_time"){
			std::cout << "creating "  << repo.size() << "th Logger for " << key << std::endl;
		}
		return *t;
	}
	return *it->second;
}

