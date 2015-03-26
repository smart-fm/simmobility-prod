#include "Threadpool.hpp"

sim_mob::ThreadPool::ThreadPool(size_t nThreads) :
		io_service(), work(new boost::asio::io_service::work(io_service)) {
	for (std::size_t i = 0; i < nThreads; ++i) {
		threads.create_thread(
				boost::bind(&boost::asio::io_service::run, &io_service));
	}
}

sim_mob::ThreadPool::~ThreadPool() {
	//kill the work so that io_service can be stopped
	work.reset();
	//stop after all your jobs are done(all your threads joined)
	io_service.run();
}


sim_mob::batched::ThreadPool::ThreadPool(size_t nThreads) :sim_mob::ThreadPool(nThreads),nTasks(0){
}


void sim_mob::batched::ThreadPool::wait(){
	boost::unique_lock<boost::mutex> lock(mutex_);
	while(nTasks){
		cond.timed_wait(lock,boost::get_system_time()+ boost::posix_time::milliseconds(100));
	}
}
