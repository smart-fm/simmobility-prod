#include "Profiler.hpp"
#include "logging/Log.hpp"

#include <boost/lockfree/queue.hpp>

boost::lockfree::queue<void* > queue(128);

std::map<const std::string, boost::shared_ptr<sim_mob::Logger> > sim_mob::Logger::repo = std::map<const std::string, boost::shared_ptr<sim_mob::Logger> >();
std::string sim_mob::Logger::newLine("\n");
sim_mob::Logger sim_mob::Logger::instance;

boost::lockfree::queue<void* > sim_mob::Logger::logQueue(128);
boost::atomic<bool> sim_mob::Logger::pushDone;
boost::shared_ptr<boost::thread> sim_mob::Logger::flusher;
sim_mob::Logger::Logger(){}
sim_mob::Logger::Logger(std::string id_){

	reset();
	start = stop = totalTime = 0;
	started = false;
	id = id_;
	std::string path = id_ + ".txt";
	if(path.size()){
		InitLogFile(path);
	}
	//decision point to use which implementation
	initDef();
}


sim_mob::Logger & sim_mob::Logger::operator[](const std::string &key)
{

	std::map<std::string, boost::shared_ptr<sim_mob::Logger> >::iterator it = repo.find(key);
	if(it == repo.end()){
		boost::shared_ptr<sim_mob::Logger> t(new sim_mob::Logger(key));
		repo.insert(std::make_pair(key,t));
		return *t;
	}
	return *it->second;
}

sim_mob::Logger::~Logger(){
	if(onExit.empty()){
		std::cout << "onExit is Empty" << std::endl;
	}
	onExit();
}

///whoami
std::string sim_mob::Logger::getId(){
	return id;
}
///whoami
int sim_mob::Logger::getIndex(){
	return index;
}

bool sim_mob::Logger::isStarted(){
	return started;
}

///like it suggests, store the start time of the profiling
void sim_mob::Logger::startProfiling(){
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
uint32_t sim_mob::Logger::endProfiling(bool addToTotalTime_){
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
void sim_mob::Logger::addToTotalTime(uint32_t value){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
//		Print() << "Logger "  << "[" << index << ":" << id << "] Adding " << value << " seconds to total time " << std::endl;
	totalTime+=value;
}

std::stringstream & sim_mob::Logger::getOut(){
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

unsigned int & sim_mob::Logger::getTotalTime(){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
	return totalTime;
}

void printTime(struct tm *tm, struct timeval & tv, std::string id){
	sim_mob::Print() << "TIMESTAMP:\t  " << tm->tm_hour << std::setw(2) << ":" <<tm->tm_min << ":" << ":" <<  tm->tm_sec << ":" << tv.tv_usec << std::endl;
}

void sim_mob::Logger::reset(){
	start = stop = totalTime = 0;
	started = false;
	id = "";
}

void  sim_mob::Logger::InitLogFile(const std::string& path)
{
	logFile.open(path.c_str());
}

sim_mob::Logger&  sim_mob::Logger::operator<<(StandardEndLine manip) {
	// call the function, but we cannot return it's value
		manip(getOut());
	return *this;
}

/* *****************************
 *     Default Implementation
 * *****************************
 */
void sim_mob::Logger::initDef()
{
	flushLog = boost::bind(&Logger::flushLogDef,this);
	onExit = boost::bind(&Logger::onExitDef,this);
	if(onExit.empty()){
		Print() << "onExit is Empty in initialization" << std::endl;
	}
}

void sim_mob::Logger::onExitDef()
{
	if(logFile.is_open()){
		flushLog();
		logFile.close();
	}
	for(outIt it(out_.begin()); it != out_.end(); safe_delete_item(it->second),it++);
}

void sim_mob::Logger::flushLogDef()
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

/* *****************************
 *     Queued Implementation
 * *****************************
 */
void sim_mob::Logger::flushToFile()
{
    void *value;
    while (!pushDone) {
        while (logQueue.pop(value)){
        	std::stringstream *out = static_cast<std::stringstream *>(value);
        	if(out){
        		logFile << out->str();
        		safe_delete_item(out);
        	}
        }
        boost::this_thread::sleep(boost::posix_time::seconds(0.5));
    }
    //same thing, just to clear the queue after pushDone is set to true
    while (logQueue.pop(value)){
    	std::stringstream *out = static_cast<std::stringstream *>(value);
    	if(out)
    	{
    		logFile << out->str();
    	}
    }
}

void sim_mob::Logger::flushLogQueued()
{
//	boost::unique_lock<boost::mutex> lock();todo: between the above and below line, there could be another large amount of data inserted and flushLogQueued() invoked before changing the pointer
	std::stringstream &out = getOut();
	logQueue.push(&out);
}

void sim_mob::Logger::initQueued()
{
	flushLog = boost::bind(&Logger::flushLogQueued,this);
	onExit = boost::bind(&Logger::onExitQueued,this);
	pushDone = false;
	flusher.reset(new boost::thread(boost::bind(&Logger::flushToFile, this)));
}

void sim_mob::Logger::onExitQueued()
{
	pushDone = true;
	flusher->join();
	if(logFile.is_open()){
		flushLog();
		logFile.close();
	}

}
