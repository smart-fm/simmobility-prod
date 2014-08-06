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
	if (logFile.is_open()) {
		flushLog();
		logFile.close();
	}
	for (outIt it(out_.begin()); it != out_.end();safe_delete_item(it->second), it++);
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
	threads[id] ++;//for debugging only
	ii++;
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
	if ((logFile.is_open() && logFile.good())){
		std::cout << "Logfile for " << path << "  creatred" << std::endl;
	}
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
	std::stringstream &out = getOut();
	Print() << "Pushing " << &logFile << "  and  " << &out << "  to Q" << std::endl;
	logQueue.push(&out);
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

