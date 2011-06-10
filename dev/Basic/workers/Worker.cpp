#include "Worker.hpp"

using std::vector;
using boost::barrier;
using boost::function;



Worker::Worker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr, unsigned int endTick)
    : BufferedDataManager(),
      internal_barr(internal_barr), external_barr(external_barr), action(action),
      endTick(endTick),
      active(this, false)  //Passing the "this" pointer is probably ok, since we only use the base class (which is constructed)
{
}


void Worker::start()
{
	active.force(true);
	currTick = 0;
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}

void Worker::join()
{
	main_thread.join();
}

void Worker::interrupt()
{
	if (main_thread.joinable()) {
		main_thread.interrupt();
	}
}

void Worker::addEntity(void* entity)
{
	data.push_back(entity);
}

void Worker::remEntity(void* entity)
{
	vector<void*>::iterator it = std::find(data.begin(), data.end(), entity);
	if (it!=data.end())
		data.erase(it);
}

vector<void*>& Worker::getEntities() {
	return data;
}


void Worker::barrier_mgmt()
{
	for (;active.get();) {
		perform_main();

		if (internal_barr!=NULL)
			internal_barr->wait();

		//Advance local time-step
		if (endTick>0 && ++currTick>=endTick) {
			this->active.set(false);
		}

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
	//Flip all data managed by this worker.
	this->flip();
}





