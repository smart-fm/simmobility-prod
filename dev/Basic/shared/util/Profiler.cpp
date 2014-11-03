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
boost::shared_ptr<sim_mob::Logger> sim_mob::Logger::log_;
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
	printTime(start);
}

//uint64_t sim_mob::Profiler::tick(bool addToTotal){
boost::chrono::microseconds sim_mob::Profiler::tick(bool addToTotal){
	boost::chrono::system_clock::time_point thisTick = getTime();
	boost::chrono::microseconds elapsed = boost::chrono::duration_cast<boost::chrono::microseconds>(thisTick - lastTick);
	if(addToTotal){
		addUpTime(elapsed);
	}
	lastTick = thisTick;
	return elapsed;
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

	if (logFile.is_open()) {
		outIt it = out.begin();
		for(; it!= out.end(); it++)
		{
			flushLog(*(it->second));
		}		
		logFile.close();
	}
	for (outIt it(out.begin()); it != out.end();safe_delete_item(it->second), it++);
}

std::stringstream * sim_mob::BasicLogger::getOut(bool renew){
	boost::upgrade_lock<boost::shared_mutex> lock(mutexOutput);
	std::stringstream *res = nullptr;
	outIt it;
	boost::thread::id id = boost::this_thread::get_id();
//	//debugging only
//	{
//		std::map <boost::thread::id, int>::iterator it_thr = threads.find(id);
//		if(it_thr != threads.end())
//		{
//			threads.at(id) ++;
//			ii++;
//		}
//		else
//		{
//			std::cerr << "WARNING : thread[" << id << "] not registered\n";
//		}
//	}
	if((it = out.find(id)) == out.end()){
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock2(lock);
		res = new std::stringstream();
		out.insert(std::make_pair(id, res));
	}
	else
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock2(lock);
		res = it->second;
		if (renew)
		{
			res = it->second = new std::stringstream();			
			//std::cout << "renewing out= " << it->second << "  " << res << std::endl;
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
			flushCnt++;
			out.str(std::string());
		}
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
        		flushCnt++;
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
    		flushCnt++;
    		safe_delete_item(buffer);
    	}
    }
}

void sim_mob::QueuedLogger::flushLog()
{
	std::stringstream *out = getOut(true);
	logQueue.push(out);
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
		std::cout << "creating a new Logger for " << key << std::endl;
		boost::shared_ptr<sim_mob::BasicLogger> t(new sim_mob::LogEngine(key));
//		std::map<const std::string, boost::shared_ptr<sim_mob::BasicLogger> > repo_;
//		repo_.insert(std::make_pair(key,t));
		repo.insert(std::make_pair(key,t));
		return *t;
	}
	return *it->second;
}

