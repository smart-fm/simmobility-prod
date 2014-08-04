#include "Profiler.hpp"
#include "logging/Log.hpp"

#include <boost/lockfree/queue.hpp>

boost::lockfree::queue<void* > queue(128);

std::map<const std::string, boost::shared_ptr<sim_mob::Profiler> > sim_mob::Profiler::repo = std::map<const std::string, boost::shared_ptr<sim_mob::Profiler> >();
std::string sim_mob::Profiler::newLine("\n");
sim_mob::Profiler sim_mob::Profiler::instance;

boost::lockfree::queue<void* > sim_mob::Profiler::logQueue(128);
boost::atomic<bool> sim_mob::Profiler::pushDone;
boost::shared_ptr<boost::thread> sim_mob::Profiler::flusher;
sim_mob::Profiler::Profiler(){}
sim_mob::Profiler::Profiler(std::string id_){

	reset();
	start = stop = totalTime = 0;
	started = false;
	id = id_;
	std::string path = id_ + ".txt";
	if(path.size()){
		InitLogFile(path);
	}
	//decision point to use which implementation
	initQueued();
}

void sim_mob::Profiler::initDef()
{
	flushLog = boost::bind(&Profiler::flushLogDef,this);
	onExit = boost::bind(&Profiler::onExitDef,this);
}

void sim_mob::Profiler::initQueued()
{
	flushLog = boost::bind(&Profiler::flushLogQueued,this);
	onExit = boost::bind(&Profiler::onExitQueued,this);
	pushDone = false;
	flusher.reset(new boost::thread(boost::bind(&Profiler::flushToFile, this)));
}

sim_mob::Profiler & sim_mob::Profiler::operator[](const std::string &key)
{

	std::map<std::string, boost::shared_ptr<sim_mob::Profiler> >::iterator it = repo.find(key);
	if(it == repo.end()){
		boost::shared_ptr<sim_mob::Profiler> t(new sim_mob::Profiler(key));
		repo.insert(std::make_pair(key,t));
		return *t;
	}
	return *it->second;
}

void sim_mob::Profiler::onExitDef()
{
	if(logFile.is_open()){
		flushLog();
		logFile.close();
	}
	for(outIt it(out_.begin()); it != out_.end(); safe_delete_item(it->second),it++);
}

void sim_mob::Profiler::onExitQueued()
{
	pushDone = true;
	flusher->join();
	if(logFile.is_open()){
		flushLog();
		logFile.close();
	}

}
sim_mob::Profiler::~Profiler(){
	onExit();
}

///whoami
std::string sim_mob::Profiler::getId(){
	return id;
}
///whoami
int sim_mob::Profiler::getIndex(){
	return index;
}

bool sim_mob::Profiler::isStarted(){
	return started;
}

///like it suggests, store the start time of the profiling
void sim_mob::Profiler::startProfiling(){
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
uint32_t sim_mob::Profiler::endProfiling(bool addToTotalTime_){
	if(!started){
		throw std::runtime_error("Profiler Ended before Starting");
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
void sim_mob::Profiler::addToTotalTime(uint32_t value){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
//		Print() << "Profiler "  << "[" << index << ":" << id << "] Adding " << value << " seconds to total time " << std::endl;
	totalTime+=value;
}

void sim_mob::Profiler::flushLogDef()
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


void sim_mob::Profiler::flushToFile()
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

void sim_mob::Profiler::flushLogQueued()
{
//	boost::unique_lock<boost::mutex> lock();todo: between the above and below line, there could be another large amount of data inserted and flushLogQueued() invoked before changing the pointer
	std::stringstream &out = getOut();
	logQueue.push(&out);
}

std::stringstream & sim_mob::Profiler::getOut(){
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

unsigned int & sim_mob::Profiler::getTotalTime(){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
	return totalTime;
}

void printTime(struct tm *tm, struct timeval & tv, std::string id){
	sim_mob::Print() << "TIMESTAMP:\t  " << tm->tm_hour << std::setw(2) << ":" <<tm->tm_min << ":" << ":" <<  tm->tm_sec << ":" << tv.tv_usec << std::endl;
}

void sim_mob::Profiler::reset(){
	start = stop = totalTime = 0;
	started = false;
	id = "";
}

void  sim_mob::Profiler::InitLogFile(const std::string& path)
{
	logFile.open(path.c_str());
}

sim_mob::Profiler&  sim_mob::Profiler::operator<<(StandardEndLine manip) {
	// call the function, but we cannot return it's value
		manip(getOut());
	return *this;
}
