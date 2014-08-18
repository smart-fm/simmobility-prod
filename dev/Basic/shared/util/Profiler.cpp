#include "Profiler.hpp"
#include "logging/Log.hpp"

#include <boost/lockfree/queue.hpp>
#include <boost/foreach.hpp>
/* **********************************
 *     Basic Logger Implementation
 * **********************************
 */
std::string sim_mob::BasicLogger::newLine("\n");
sim_mob::Logger sim_mob::Logger::log;

std::map <boost::thread::id, int> sim_mob::BasicLogger::threads= std::map <boost::thread::id, int>();//for debugging only
int sim_mob::BasicLogger::flushCnt = 0;
unsigned long int sim_mob::BasicLogger::ii = 0;

sim_mob::BasicLogger::BasicLogger(std::string id_){
//	reset();
//	start = stop = totalTime = 0;
//	started = false;
	id = id_;
	std::string path = id_ + ".txt";
	if(path.size()){
		initLogFile(path);
	}
}

sim_mob::BasicLogger::~BasicLogger(){

	if(!profilers_.empty()){
		std::map<const std::string, Profiler>::iterator it(profilers_.begin()),itEnd(profilers_.end());
		for(;it != itEnd; it++)
		{
			*this << it->first << ": [total AddUp time: " << it->second.getAddUp() << "],[total time : " << it->second.end() << "]" << newLine;
		}
	}

	if (logFile.is_open()) {
		flushLog();
		logFile.close();
	}
	for (outIt it(out_.begin()); it != out_.end();safe_delete_item(it->second), it++);
}

///like it suggests, store the start time of the profiling
uint32_t sim_mob::Profiler::begin(){
	started = true;

	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);

	return (start = lastTick = (tm->tm_hour * 3600 * 1000 * 1000) + (tm->tm_min * 60 * 1000 * 1000) +
		(tm->tm_sec * 1000 * 1000) + (tv.tv_usec));
}

uint32_t sim_mob::Profiler::tick(bool addToTotalTime_){
	if(!started){
		Warn() << "Profiler ticked before starting, starting it now" << std::endl;
		begin();
		return 0;
	}

	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
	uint32_t thisTick;
	thisTick = ((tm->tm_hour * 3600 * 1000 * 1000) + (tm->tm_min * 60 * 1000 * 1000) +
			(tm->tm_sec * 1000 * 1000) + (tv.tv_usec));

	uint32_t elapsed = thisTick - lastTick;
	if(addToTotalTime_){
		addUp(elapsed);
	}
	lastTick = thisTick;
	return elapsed;
}

uint32_t sim_mob::Profiler::end(){
	uint32_t temp = 0;
	uint32_t tick_;
	tick_ = tick();
	started = 0;
	return (tick_ > start ? tick_ - start : 0);
}

void sim_mob::Profiler::reset()
{
	start = lastTick = total = 0;
	started = 0;
}


///add the given time to the total time
uint32_t sim_mob::Profiler::addUp(uint32_t &value){
//	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
	total+=value;
	return total;
}

///add the given time to the total time
uint32_t sim_mob::Profiler::getAddUp(){
//	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
	return total;
}

std::stringstream * sim_mob::BasicLogger::getOut(bool renew){
	boost::upgrade_lock<boost::shared_mutex> lock(mutexOutput);
	std::stringstream *res = nullptr;
	outIt it;
	boost::thread::id id = boost::this_thread::get_id();
	threads[id] ++;//for debugging only
	ii++;
	if((it = out_.find(id)) == out_.end()){
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock2(lock);
		res = new std::stringstream();
		out_.insert(std::make_pair(id, res));
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

sim_mob::Profiler & sim_mob::BasicLogger::prof(const std::string id)
{
	std::map<const std::string, Profiler>::iterator it(profilers_.find(id));
	if(it != profilers_.end())
	{
		return it->second;
	}
	else
	{
		profilers_.insert(std::pair<const std::string, Profiler>(id, Profiler()));
		it = profilers_.find(id);
	}
	return it->second;
}

uint32_t sim_mob::BasicLogger::endProfiler(const std::string id)
{
	uint32_t temp = 0;
	std::map<const std::string, Profiler>::iterator it(profilers_.find(id));
	if(it != profilers_.end())
	{
		temp = it->second.end();
		profilers_.erase(it);
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
		std::stringstream out("");
		out << item->first;
		if("{Not-any-thread}" == out.str() ){
			break;
		}
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

