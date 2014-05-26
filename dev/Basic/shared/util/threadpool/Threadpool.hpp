#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
namespace sim_mob {
/**
 * An implementation of threadpool using boost features: asio and threadgroup
 * this pool can run several types of methods at the same time.
 *
 * how to use:
 * for assigning a method of a class to threadpool 'pool' just call  pool.enqueue(&MyClass::myMethod, pointer-toObject, arg1,arg2,arg3...);//usually pointer-toObject is nothing but 'this'.
 *
 * how it works:
 * 'io_service' itself is assigned to 'boost::asio::io_service::work' so that it doesn't stop running after its current task is over(to be available for the next job).
 * a group of threads run io_service.run() simultaneously.
 * multithreaded tasks are assigned to the pool using io_service.post() which will manage assignment of tasks to the threads.
 * or
 *
 *
 */
class ThreadPool {
public:
	ThreadPool(std::size_t);
	template<class F>
	void enqueue(F f);
	~ThreadPool();
private:
	boost::asio::io_service io_service;
	boost::shared_ptr<boost::asio::io_service::work> work;
	boost::thread_group threads;
};

/**
 * the constructor just launches some amount of workers
 * @param nThreads number of threads running in this thread pool
 */
ThreadPool::ThreadPool(size_t nThreads) :
		io_service(), work(new boost::asio::io_service::work(io_service)) {
	for (std::size_t i = 0; i < nThreads; ++i) {
		threads.create_thread(
				boost::bind(&boost::asio::io_service::run, &io_service));
	}
}
/**
 * add new work item to the pool
 * @param f function and its arguments
 */
template<class F>
void ThreadPool::enqueue(F f) {
	io_service.post(f);
}

/// the destructor joins all threads
ThreadPool::~ThreadPool() {
	//kill the work so that io_service can be stopped
	work.reset();
	//stop after all your jobs are done(all your threads joined)
	io_service.run();
}

/*
 //it can be tested via:
 void f(int i){
 std::cout << "hello " << i << std::endl;
 boost::this_thread::sleep(boost::posix_time::milliseconds(300));
 std::cout << "world " << i << std::endl;
 }

 int main() {
 ThreadPool pool(4);
 for( int i = 0; i < 8; ++i ) {
 std::cout << "task " << i << " created" << std::endl;
 pool.enqueue(boost::bind(&f,i));
 }
 }
 */

/**
 * second version of threadpool is used only for special cases where user insists to complete different batches of tasks
 * serially using the same thread pool. In such a case, user expects the thread pool to notify the user when all the threads
 * are over. This is simply done by invoking the origina tasks along with a condition variable. As soon as a thread concludes,
 * a notification is generated. The user can act accordingly when ALL threads have concluded their jobs.(i purposefully used
 * the term 'conclude' rather that 'join' as the threads simply running io_service not the input tasks. Input tasks are executed
 * by the io_services running within the threads.
 */
namespace batched {
class ThreadPool {
public:
	ThreadPool(std::size_t);
	template<class F>
	void enqueue(F f);
	template<class F>
	void wrapper(boost::tuple<F> f);
	~ThreadPool();
private:
	// the io_service we are wrapping
	boost::asio::io_service io_service;
	boost::shared_ptr<boost::asio::io_service::work> work;
	boost::thread_group threads;
};

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t nThreads) :
		io_service(), work(new boost::asio::io_service::work(io_service)) {
	for (std::size_t i = 0; i < nThreads; ++i) {
		threads.create_thread(
				boost::bind(&boost::asio::io_service::run, &io_service));
	}
}
// add new work item to the pool
template<class F>
void ThreadPool::enqueue(F f) {
	void (ThreadPool::*ff)(boost::tuple<F>) = &ThreadPool::wrapper<F>;
	io_service.post(boost::bind(ff, this, boost::make_tuple(f))); //using a tuple seems to be the only practical way. it is mentioned in boost examples.
}

template<class F>
void ThreadPool::wrapper(boost::tuple<F> f) {
	std::cout << "Calling a Wrapper" << std::endl;
	boost::get<0>(f)();
	std::cout << "Wrapped up" << std::endl;
}

// the destructor joins all threads
ThreadPool::~ThreadPool() {
	//kill the work so that io_service can be stopped
	work.reset();
	//stop after all your jobs are done(all your threads joined)
	io_service.run();
}
}
}

