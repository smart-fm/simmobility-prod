#include "Profiler.hpp"
#include "logging/Log.hpp"
std::map<const std::string, boost::shared_ptr<sim_mob::Profiler> > sim_mob::Profiler::repo = std::map<const std::string, boost::shared_ptr<sim_mob::Profiler> >();
std::string sim_mob::Profiler::newLine("\n");
sim_mob::Profiler sim_mob::Profiler::instance;
sim_mob::Profiler::Profiler(){}
sim_mob::Profiler::Profiler(std::string id_,bool init){

	reset();
	start = stop = totalTime = 0;
	started = false;
	output.clear();
	outputSize = 0;
	id = id_;
	std::string path = id_ + ".txt";
	if(path.size()){
		InitLogFile(path);
	}
	if(init && !isStarted()){
		startProfiling();
	}
}

sim_mob::Profiler & sim_mob::Profiler::operator[](const std::string &key)
{

	std::map<std::string, boost::shared_ptr<sim_mob::Profiler> >::iterator it = repo.find(key);
	if(it == repo.end()){
		boost::shared_ptr<sim_mob::Profiler> t(new sim_mob::Profiler(key,false));
		repo.insert(std::make_pair(key,t));
	}
	return *repo[key];
}

sim_mob::Profiler::~Profiler(){
	if(LogFile.is_open()){
		flushLog();
		LogFile.close();
	}
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
std::stringstream & sim_mob::Profiler::outPut(){
	boost::unique_lock<boost::mutex> lock(mutexOutput);
	return output;
}

void sim_mob::Profiler::flushLog(){
		if ((LogFile.is_open() && LogFile.good())) {
			//boost::unique_lock<boost::mutex> lock(mutexOutput);
			LogFile << output.str();
			LogFile.flush();
			output.str("");
			output.clear();
			outputSize = 0;
		}
		else{
			Warn() << "pathset profiler log ignored" << std::endl;
		}

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
	output.clear();
	id = "";
}

void  sim_mob::Profiler::InitLogFile(const std::string& path)
{
	//1. first type of implementation
	LogFile.open(path.c_str());
//	if (LogFile.fail()) {
//		log_handle =  &std::cout;
//	}
//	log_handle = (!LogFile.fail() ? &LogFile : &std::cout);
}

////Type of cout.
//typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
////Type of std::endl and some other manipulators.
//typedef CoutType& (*StandardEndLine)(CoutType&);

sim_mob::Profiler&  sim_mob::Profiler::operator<<(StandardEndLine manip) {
		manip(output);
	return *this;
}
