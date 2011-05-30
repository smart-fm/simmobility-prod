#include "Worker.hpp"

using std::vector;
using boost::barrier;
using boost::function;



Worker::Worker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr)
    : internal_barr(internal_barr), external_barr(external_barr), action(action)
{
}


void Worker::start()
{
	//this.active = true;
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}


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

vector<void*>& Worker::getEntities() {
	return entities;
}


void Worker::barrier_mgmt()
{
	for (;;) {
		perform_main();

		if (internal_barr!=NULL)
			internal_barr->wait();

		perform_flip();

		if (external_barr!=NULL)
			external_barr->wait();
	}
}


void Worker::perform_main()
{
	if (action!=NULL)
		(*action)(this);
}

void Worker::perform_flip()
{
}




