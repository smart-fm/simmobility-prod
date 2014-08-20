#include "Profiler.hpp"
#include "logging/Log.hpp"

#include <boost/lockfree/queue.hpp>
#include <boost/foreach.hpp>
/* **********************************
 *     Basic Logger Implementation
 * **********************************
 */
std::string sim_mob::Logger::newLine("\n");
sim_mob::Logger sim_mob::Logger::log;

std::map <boost::thread::id, int> sim_mob::BasicLogger::threads= std::map <boost::thread::id, int>();//for debugging only
int sim_mob::BasicLogger::flushCnt = 0;
unsigned long int sim_mob::BasicLogger::ii = 0;

sim_mob::BasicLogger::BasicLogger(std::string id_){
	id = id_;
	std::string path = id_ + ".txt";
	if(path.size()){
		initLogFile(path);
	}
}

sim_mob::BasicLogger::~BasicLogger(){

	if(!profilers.empty()){
		std::map<const std::string, Profiler>::iterator it(profilers.begin()),itEnd(profilers.end());
		for(;it != itEnd; it++)
		{
			uint32_t addup = it->second.getAddUp() ;
			*this << it->first << ": [total AddUp : " << addup << "],[total time : " << it->second.end() << "]" << Logger::newLine;
		}
	}

	if (logFile.is_open()) {
		flushLog();
		logFile.close();
	}
	for (outIt it(out.begin()); it != out.end();safe_delete_item(it->second), it++);
}

const uint32_t sim_mob::Profiler::getTime()
{

	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);

	return (tm->tm_hour * 3600000000/*3600 * 1000 * 1000*/) +
	(tm->tm_min * 60000000/*60 * 1000 * 1000*/) +
	(tm->tm_sec * 1000000/*1000 * 1000*/) +
	(tv.tv_usec);
}

///like it suggests, store the start time of the profiling
uint32_t sim_mob::Profiler::begin(){
	return (start = lastTick = getTime());
}

uint32_t sim_mob::Profiler::tick(bool addToTotal, bool end){
	if(!started){
		return 0;
	}
	uint32_t thisTick = getTime();
	uint32_t elapsed = thisTick - lastTick;
	if(addToTotal){
		addUp(elapsed);
	}
	lastTick = thisTick;
	if(end)
	{
		started = false;
	}
	return elapsed;
}

uint32_t sim_mob::Profiler::end(){
	if(!started)
	{
		std::cout << "WARNING:profiler " << id << " is not started" << std::endl;
		return 0;
	}
	uint32_t tick_;
	tick_ = getTime();
	started = 0;
	if(tick_ <= start ){
		std::cout << "WARNING:profiler " << id << " Start time " << start << " and end time = " << tick_ << std::endl;
	}

	return (tick_ > start ? tick_ - start : 0);
}

void sim_mob::Profiler::reset()
{
	start = lastTick = total = 0;
	started = 0;
}

uint32_t sim_mob::Profiler::addUp(const uint32_t value){
	total+=value;
	return total;
}

uint32_t sim_mob::Profiler::getAddUp(){
	return total;
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
			it->second = new std::stringstream();
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

uint32_t sim_mob::BasicLogger::endProfiler(const std::string id)
{
	uint32_t temp = 0;
	std::map<const std::string, Profiler>::iterator it(profilers.find(id));
	if(it != profilers.end())
	{
		temp = it->second.end();
		profilers.erase(it);
	}
	return temp;

}

void printTime(struct tm *tm, struct timeval & tv, std::string id){
	sim_mob::Print() << "TIMESTAMP:\t  " << tm->tm_hour << std::setw(2) << ":" <<tm->tm_min << ":" << ":" <<  tm->tm_sec << ":" << tv.tv_usec << std::endl;
}

void  sim_mob::BasicLogger::initLogFile(const std::string& path)
{
	logFile.open(path.c_str());
	if ((logFile.is_open() && logFile.good())){
		std::cout << "Logfile for " << path << "  creatred" << std::endl;
	}
}

sim_mob::BasicLogger&  sim_mob::BasicLogger::operator<<(StandardEndLine manip) {
	// call the function, but we cannot return it's value
		manip(*getOut());
	return *this;
}

void sim_mob::BasicLogger::flushLog()
{
	if ((logFile.is_open() && logFile.good()))
	{
		std::stringstream *out = getOut();
		{
			boost::unique_lock<boost::mutex> lock(flushMutex);
			logFile << out->str();
			logFile.flush();
			flushCnt++;
			out->str(std::string());
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

        	std::cout << "poped out  " << buffer << "  to Q" << std::endl;
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
    std::cout << "Out of flushToFile" << std::endl;
}

void sim_mob::QueuedLogger::flushLog()
{
	std::stringstream *out = getOut(true);
	Print() << "Pushing " << out << "  to Q" << std::endl;
	logQueue.push(out);
}


/* ****************************************
 *     Default Logger wrap Implementation
 * ****************************************
 */
sim_mob::Logger::~Logger()
{
	//debug code

	std::cout << "Number of threads used: " << sim_mob::BasicLogger::threads.size() << std::endl;
	for(std::map <boost::thread::id, int>::iterator item = sim_mob::BasicLogger::threads.begin(); item != sim_mob::BasicLogger::threads.end(); item++)
	{
		std::cout << "Thread[" << item->first << "] called out " << item->second << "  times" << std::endl;
	}
	std::cout << "Total calls to getOut: " << sim_mob::BasicLogger::ii << std::endl;
	std::cout << "Number of flushes to files " << sim_mob::BasicLogger::flushCnt  << std::endl;
	//debug...
	std::pair<std::string, boost::shared_ptr<sim_mob::BasicLogger> > item;
	BOOST_FOREACH(item,repo)
	{
		item.second.reset();
	}
	repo.clear();
}
sim_mob::BasicLogger & sim_mob::Logger::operator[](const std::string &key)
{

	std::map<std::string, boost::shared_ptr<sim_mob::BasicLogger> >::iterator it = repo.find(key);
	if(it == repo.end()){
		boost::shared_ptr<sim_mob::BasicLogger> t(new sim_mob::LogEngine(key));
		repo.insert(std::make_pair(key,t));
		return *t;
	}
	return *it->second;
}

