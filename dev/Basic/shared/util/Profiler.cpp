#include "Profiler.hpp"
#include "logging/Log.hpp"

#include <boost/lockfree/queue.hpp>

boost::lockfree::queue<void* > queue(128);

std::map<const std::string, boost::shared_ptr<sim_mob::BaseProfiler> > sim_mob::BaseProfiler::repo = std::map<const std::string, boost::shared_ptr<sim_mob::BaseProfiler> >();
std::string sim_mob::BaseProfiler::newLine("\n");
sim_mob::BaseProfiler sim_mob::BaseProfiler::instance;
sim_mob::BaseProfiler::BaseProfiler(){}
sim_mob::BaseProfiler::BaseProfiler(std::string id_,bool init){

	reset();
	start = stop = totalTime = 0;
	started = false;
//	output.clear();
	id = id_;
	std::string path = id_ + ".txt";
	if(path.size()){
		InitLogFile(path);
	}
	if(init && !isStarted()){
		startProfiling();
	}
}

sim_mob::BaseProfiler & sim_mob::BaseProfiler::operator[](const std::string &key)
{

	std::map<std::string, boost::shared_ptr<sim_mob::BaseProfiler> >::iterator it = repo.find(key);
	if(it == repo.end()){
		boost::shared_ptr<sim_mob::BaseProfiler> t(new sim_mob::BaseProfiler(key,false));
		repo.insert(std::make_pair(key,t));
	}
	return *repo[key];
}

sim_mob::BaseProfiler::~BaseProfiler(){
	if(LogFile.is_open()){
		flushLog();
		LogFile.close();
	}
	for(outIt it(out_.begin()); it != out_.end(); safe_delete_item(it->second),it++);
}

///whoami
std::string sim_mob::BaseProfiler::getId(){
	return id;
}
///whoami
int sim_mob::BaseProfiler::getIndex(){
	return index;
}

bool sim_mob::BaseProfiler::isStarted(){
	return started;
}

///like it suggests, store the start time of the profiling
void sim_mob::BaseProfiler::startProfiling(){
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
uint32_t sim_mob::BaseProfiler::endProfiling(bool addToTotalTime_){
	if(!started){
		throw std::runtime_error("BaseProfiler Ended before Starting");
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
void sim_mob::BaseProfiler::addToTotalTime(uint32_t value){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
//		Print() << "BaseProfiler "  << "[" << index << ":" << id << "] Adding " << value << " seconds to total time " << std::endl;
	totalTime+=value;
}

void sim_mob::BaseProfiler::flushLog(){
		if ((LogFile.is_open() && LogFile.good())) {
			std::stringstream &out = getOut();
			{
				boost::unique_lock<boost::mutex> lock(flushMutex);
				LogFile << "[" << boost::this_thread::get_id() << "]\n" << out.str();
				LogFile.flush();
				out.str(std::string());
			}
		}
		else{
			Warn() << "pathset profiler log ignored" << std::endl;
		}
}

std::stringstream & sim_mob::BaseProfiler::getOut(){
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

unsigned int & sim_mob::BaseProfiler::getTotalTime(){
	boost::unique_lock<boost::mutex> lock(mutexTotalTime);
	return totalTime;
}

void printTime(struct tm *tm, struct timeval & tv, std::string id){
	sim_mob::Print() << "TIMESTAMP:\t  " << tm->tm_hour << std::setw(2) << ":" <<tm->tm_min << ":" << ":" <<  tm->tm_sec << ":" << tv.tv_usec << std::endl;
}

void sim_mob::BaseProfiler::reset(){
	start = stop = totalTime = 0;
	started = false;
	id = "";
}

void  sim_mob::BaseProfiler::InitLogFile(const std::string& path)
{
	LogFile.open(path.c_str());
}

sim_mob::BaseProfiler&  sim_mob::BaseProfiler::operator<<(StandardEndLine manip) {
	// call the function, but we cannot return it's value
		manip(getOut());
	return *this;
}
