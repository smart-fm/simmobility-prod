#include "Worker.hpp"

using boost::barrier;
using boost::function;

Worker::Worker(function& action, barrier* const barr) : barr(barr), action(action)
{
	//this.active = false;
}



void Worker::start()
{
	//this.active = true;
	this.main_thread = boost::thread(boost::bind(main_loop));
}

void Worker::interrupt()
{
	main_thread.interrupt();
}


/*void Worker::join()
{
	this.active = false;
	this.main_thread.join();
}*/


void Worker::main_loop()
{
	for (;;) {
		this.action();
		this.barr.wait();
	}
}





