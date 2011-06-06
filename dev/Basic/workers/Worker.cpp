#include "Worker.hpp"

using std::vector;
using boost::barrier;
using boost::function;



Worker::Worker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr)
    : internal_barr(internal_barr), external_barr(external_barr), action(action), active(false)
{
}


void Worker::start()
{
	active.force(true);
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}


void Worker::interrupt()
{
	main_thread.interrupt();
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
	//TODO: This is currently incorrect; it flips multiple times because there are
	//      multiple workers, and BufferedDataManager is static. We need a way to
	//      associate a BufferedDataManager with each worker, especially if we
	//      later intend to lay these objects out in memory (for a faster flip)
	sim_mob::BufferedDataManager::GetInstance().flip();
}




