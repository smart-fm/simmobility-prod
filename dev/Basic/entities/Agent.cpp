/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Agent.hpp"

#include <cstdlib>

#include "util/OutputUtil.hpp"
#include "partitions/PartitionManager.hpp"
#include "entities/UpdateParams.hpp"

using namespace sim_mob;

using std::vector;
using std::priority_queue;

//#ifndef SIMMOB_DISABLE_DYNAMIC_DISPATCH
//boost::mutex sim_mob::Agent::all_agents_lock;
StartTimePriorityQueue sim_mob::Agent::pending_agents;
//#endif

vector<Entity*> sim_mob::Agent::all_agents;

//Implementation of our comparison function for Agents by start time.
bool sim_mob::cmp_agent_start::operator()(const Entity* x, const Entity* y) const {
	//We want a lower start time to translate into a higher priority.
	return x->getStartTime() > y->getStartTime();
}

unsigned int sim_mob::Agent::next_agent_id = 0;
unsigned int sim_mob::Agent::GetAndIncrementID(int preferredID) {
	//If the ID is valid, modify next_agent_id;
	if (preferredID > static_cast<int> (next_agent_id)) {
		next_agent_id = static_cast<unsigned int> (preferredID);
	}

#ifndef SIMMOB_DISABLE_MPI
	if (ConfigParams::GetInstance().is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		int mpi_id = partitionImpl.partition_config->partition_id;
		int cycle = partitionImpl.partition_config->maximum_agent_id;
		return (next_agent_id++) + cycle * mpi_id;
	} else {
#endif
		return next_agent_id++;
#ifndef SIMMOB_DISABLE_MPI
	}
#endif
}

sim_mob::Agent::Agent(const MutexStrategy& mtxStrat, int id) : Entity(GetAndIncrementID(id)),
	originNode(nullptr), destNode(nullptr), xPos(mtxStrat, 0), yPos(mtxStrat, 0),
	fwdVel(mtxStrat, 0), latVel(mtxStrat, 0), xAcc(mtxStrat, 0), yAcc(mtxStrat, 0)
{
	firstFrameTick = true;
	toRemoved = false;
	dynamic_seed = id;
}

sim_mob::Agent::~Agent()
{

}

void sim_mob::Agent::update(frame_t frameNumber)
{
	//First, we need to retrieve an UpdateParams subclass appropriate for this Agent.
	unsigned int currTimeMS = frameNumber * ConfigParams::GetInstance().baseGranMS;
	UpdateParams& params = make_frame_tick_params(frameNumber, currTimeMS);

	//Has update() been called early?
	if(currTimeMS < getStartTime()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			std::stringstream msg;
			msg << "Agent(" << getId() << ") specifies a start time of: " << getStartTime()
					<< " but it is currently: " << currTimeMS
					<< "; this indicates an error, and should be handled automatically.";
			throw std::runtime_error(msg.str().c_str());
		}
		return;
	}

	//Has update() been called too late?
	if (isToBeRemoved()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("Agent is already done, but hasn't been removed.");
		}
		return;
	}

	//Is this the first frame tick for this Agent?
	if (firstFrameTick) {
		//Helper check; not needed once we trust our Workers.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			if (abs(currTimeMS-parent->getStartTime())>=ConfigParams::GetInstance().baseGranMS) {
				std::stringstream msg;
				msg << "Agent was not started within one timespan of its requested start time.";
				msg << "\nStart was: " << getStartTime() << ",  Curr time is: " << currTimeMS << "\n";
				msg << "Agent ID: " << getId() << "\n";
				throw std::runtime_error(msg.str().c_str());
			}
		}

		frame_init(params);
		firstFrameTick = false;
	}

	//Now perform the main update tick
	if (!isToBeRemoved()) {
		frame_tick(params);
	}

	//Finally, save the output
	if (!isToBeRemoved()) {
		frame_tick_output(params);
	}
}

void sim_mob::Agent::buildSubscriptionList()
{
	subscriptionList_cached.push_back(&xPos);
	subscriptionList_cached.push_back(&yPos);
	subscriptionList_cached.push_back(&fwdVel);
	subscriptionList_cached.push_back(&latVel);
	subscriptionList_cached.push_back(&xAcc);
	subscriptionList_cached.push_back(&yAcc);
	//subscriptionList_cached.push_back(&currentLink);
	//subscriptionList_cached.push_back(&currentCrossing);

}

bool sim_mob::Agent::isToBeRemoved() {
	return toRemoved;
}

void sim_mob::Agent::setToBeRemoved() {
	toRemoved = true;
}


#ifndef SIMMOB_DISABLE_MPI
void sim_mob::Agent::package(PackageUtils& packageUtil) {
	//std::cout << "Agent package Called" <<this->getId()<< std::endl;

	packageUtil.packageBasicData(id);
	packageUtil.packageBasicData(isSubscriptionListBuilt);
	packageUtil.packageBasicData(startTime);

	packageUtil.packageNode(originNode);
	packageUtil.packageNode(destNode);

	packageUtil.packageBasicData(xPos.get());
	packageUtil.packageBasicData(yPos.get());
	packageUtil.packageBasicData(fwdVel.get());
	packageUtil.packageBasicData(latVel.get());
	packageUtil.packageBasicData(xAcc.get());
	packageUtil.packageBasicData(yAcc.get());

	packageUtil.packageBasicData(toRemoved);
	packageUtil.packageBasicData(dynamic_seed);
}

void sim_mob::Agent::unpackage(UnPackageUtils& unpackageUtil) {

	id = unpackageUtil.unpackageBasicData<int> ();
	//std::cout << "Agent unpackage Called:" <<this->getId() << std::endl;
	isSubscriptionListBuilt = unpackageUtil.unpackageBasicData<bool> ();
	startTime = unpackageUtil.unpackageBasicData<int> ();

	originNode = const_cast<Node*>(unpackageUtil.unpackageNode());
	destNode = const_cast<Node*>(unpackageUtil.unpackageNode());

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	x_pos = unpackageUtil.unpackageBasicData<int> ();
	y_pos = unpackageUtil.unpackageBasicData<int> ();
	x_acc = unpackageUtil.unpackageBasicData<double> ();
	y_acc = unpackageUtil.unpackageBasicData<double> ();
	x_vel = unpackageUtil.unpackageBasicData<double> ();
	y_vel = unpackageUtil.unpackageBasicData<double> ();

	xPos.force(x_pos);
	yPos.force(y_pos);
	xAcc.force(x_acc);
	yAcc.force(y_acc);
	fwdVel.force(x_vel);
	latVel.force(y_vel);

	toRemoved = unpackageUtil.unpackageBasicData<bool> ();
	dynamic_seed = unpackageUtil.unpackageBasicData<int> ();
}

void sim_mob::Agent::packageProxy(PackageUtils& packageUtil)
{
	packageUtil.packageBasicData(id);
	packageUtil.packageBasicData(isSubscriptionListBuilt);
	packageUtil.packageBasicData(startTime);

	packageUtil.packageNode(originNode);
	packageUtil.packageNode(destNode);

	packageUtil.packageBasicData(xPos.get());
	packageUtil.packageBasicData(yPos.get());
	packageUtil.packageBasicData(fwdVel.get());
	packageUtil.packageBasicData(latVel.get());
	packageUtil.packageBasicData(xAcc.get());
	packageUtil.packageBasicData(yAcc.get());

	packageUtil.packageBasicData(toRemoved);
	packageUtil.packageBasicData(dynamic_seed);
}

void sim_mob::Agent::unpackageProxy(UnPackageUtils& unpackageUtil) {
	id = unpackageUtil.unpackageBasicData<int> ();
	isSubscriptionListBuilt = unpackageUtil.unpackageBasicData<bool> ();
	startTime = unpackageUtil.unpackageBasicData<int> ();

	originNode = const_cast<Node*> (unpackageUtil.unpackageNode());
	destNode = const_cast<Node*> (unpackageUtil.unpackageNode());

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	x_pos = unpackageUtil.unpackageBasicData<int> ();
	y_pos = unpackageUtil.unpackageBasicData<int> ();
	x_acc = unpackageUtil.unpackageBasicData<double> ();
	y_acc = unpackageUtil.unpackageBasicData<double> ();
	x_vel = unpackageUtil.unpackageBasicData<double> ();
	y_vel = unpackageUtil.unpackageBasicData<double> ();

	xPos.force(x_pos);
	yPos.force(y_pos);
	xAcc.force(x_acc);
	yAcc.force(y_acc);
	fwdVel.force(x_vel);
	latVel.force(y_vel);

	toRemoved = unpackageUtil.unpackageBasicData<bool> ();
	dynamic_seed = unpackageUtil.unpackageBasicData<int> ();
}

int sim_mob::Agent::getOwnRandomNumber() {
	int one_try = -1;
	int second_try = -2;
	int third_try = -3;
	//		int forth_try = -4;

	while (one_try != second_try || third_try != second_try) {
		srand(dynamic_seed);
		one_try = rand();

		srand(dynamic_seed);
		second_try = rand();

		srand(dynamic_seed);
		third_try = rand();
	}

	dynamic_seed = one_try;
	return one_try;
}
#endif
