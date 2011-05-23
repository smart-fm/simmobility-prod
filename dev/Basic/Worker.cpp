#include "Worker.hpp"

using std::vector;
using boost::barrier;
using boost::function;



Worker::Worker(function& action*, barrier* barr) : barr(barr), action(action)
{
	//this.active = false;
}


void Worker::start()
{
	//this.active = true;
	this.main_thread = boost::thread(boost::bind(main_loop));
}


//Worker::Worker(const Worker& copy) {}
//Worker& Worker::operator=(const Worker& rhs) { return *this; }


void Worker::interrupt()
{
	main_thread.interrupt();
}


void Worker::addEntity(void* entity)
{
	entities.push_back(entity);
}

void Worker::remEntity(void* entity)
{
	vector<void*>::iterator it = std::find(entities.begin(), entities.end(), entity);
	if (it!=entities.end())
		entities.erase(it);
}



void Worker::main_loop()
{
	for (;;) {
		this->action();

		if (barrier!=NULL)
			this->barr.wait();
	}
}





