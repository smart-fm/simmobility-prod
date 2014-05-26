#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
// the actual thread pool
struct ThreadPool {
   ThreadPool(std::size_t);
   template<class F>
   void enqueue(F f);
   ~ThreadPool();


   // the io_service we are wrapping
   boost::asio::io_service io_service;
   boost::shared_ptr<boost::asio::io_service::work> work;
   boost::thread_group threads;
};

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t nThreads)
   :io_service()
   ,work(new boost::asio::io_service::work(io_service))
{
   for ( std::size_t i = 0; i < nThreads; ++i ) {
    threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
   }
}
// add new work item to the pool
template<class F>
void ThreadPool::enqueue(F f) {
   io_service.post(f);
}

// the destructor joins all threads
ThreadPool::~ThreadPool() {
work.reset();
io_service.run();
}

//it can be tested via:
/*
void f(int i)
{
    std::cout << "hello " << i << std::endl;
    boost::this_thread::sleep(boost::posix_time::milliseconds(300));
    std::cout << "world " << i << std::endl;
}

int main() {
   // create a thread pool of 4 worker threads
   ThreadPool pool(4);

   // queue a bunch of "work items"
   for( int i = 0; i < 8; ++i ) {
      std::cout << "task " << i << " created" << std::endl;
      pool.enqueue(boost::bind(&f,i));
   }
}
*/
