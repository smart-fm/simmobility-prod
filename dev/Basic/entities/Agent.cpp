/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Agent.hpp"

#include "util/OutputUtil.hpp"
#include "partitions/PartitionManager.hpp"
#include <cstdlib>

#ifndef SIMMOB_DISABLE_MPI
#include "geospatial/Node.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using namespace sim_mob;

using std::vector;
using std::priority_queue;


StartTimePriorityQueue sim_mob::Agent::pending_agents;
vector<Entity*> sim_mob::Agent::all_agents;

//Implementation of our comparison function for Agents by start time.
bool sim_mob::cmp_agent_start::operator()(const PendingEntity& x, const PendingEntity& y) const {
	//We want a lower start time to translate into a higher priority.
	return x.start > y.start;
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
	toRemoved = false;
	dynamic_seed = id;
}

sim_mob::Agent::~Agent()
{

}

void sim_mob::Agent::buildSubscriptionList(vector<BufferedBase*>& subsList)
{
	subsList.push_back(&xPos);
	subsList.push_back(&yPos);
	subsList.push_back(&fwdVel);
	subsList.push_back(&latVel);
	subsList.push_back(&xAcc);
	subsList.push_back(&yAcc);
	//subscriptionList_cached.push_back(&currentLink);
	//subscriptionList_cached.push_back(&currentCrossing);

}

bool sim_mob::Agent::isToBeRemoved() {
	return toRemoved;
}

void sim_mob::Agent::setToBeRemoved() {
	toRemoved = true;
}

void sim_mob::Agent::clearToBeRemoved() {
	toRemoved = false;
}


#ifndef SIMMOB_DISABLE_MPI
void sim_mob::Agent::pack(PackageUtils& packageUtil) {
	//std::cout << "Agent package Called" <<this->getId()<< std::endl;

	packageUtil.packBasicData(id);
	//packageUtil.packBasicData(isSubscriptionListBuilt);
	packageUtil.packBasicData(startTime);

	sim_mob::Node::pack(packageUtil, originNode);
	sim_mob::Node::pack(packageUtil, destNode);

//	packageUtil.packNode(originNode);
//	packageUtil.packNode(destNode);

	packageUtil.packBasicData(xPos.get());
	packageUtil.packBasicData(yPos.get());
	packageUtil.packBasicData(fwdVel.get());
	packageUtil.packBasicData(latVel.get());
	packageUtil.packBasicData(xAcc.get());
	packageUtil.packBasicData(yAcc.get());

	packageUtil.packBasicData(toRemoved);
	packageUtil.packBasicData(dynamic_seed);
}

void sim_mob::Agent::unpack(UnPackageUtils& unpackageUtil) {

	id = unpackageUtil.unpackBasicData<int> ();
	//std::cout << "Agent unpackage Called:" <<this->getId() << std::endl;
	//isSubscriptionListBuilt = unpackageUtil.unpackBasicData<bool> ();
	startTime = unpackageUtil.unpackBasicData<int> ();

	originNode = Node::unpack(unpackageUtil);
	destNode = Node::unpack(unpackageUtil);

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	x_pos = unpackageUtil.unpackBasicData<int> ();
	y_pos = unpackageUtil.unpackBasicData<int> ();
	x_acc = unpackageUtil.unpackBasicData<double> ();
	y_acc = unpackageUtil.unpackBasicData<double> ();
	x_vel = unpackageUtil.unpackBasicData<double> ();
	y_vel = unpackageUtil.unpackBasicData<double> ();

	xPos.force(x_pos);
	yPos.force(y_pos);
	xAcc.force(x_acc);
	yAcc.force(y_acc);
	fwdVel.force(x_vel);
	latVel.force(y_vel);

	toRemoved = unpackageUtil.unpackBasicData<bool> ();
	dynamic_seed = unpackageUtil.unpackBasicData<int> ();
}

void sim_mob::Agent::packProxy(PackageUtils& packageUtil)
{
	packageUtil.packBasicData(id);
	//packageUtil.packBasicData(isSubscriptionListBuilt);
	packageUtil.packBasicData(startTime);

	packageUtil.packBasicData(xPos.get());
	packageUtil.packBasicData(yPos.get());
	packageUtil.packBasicData(fwdVel.get());
	packageUtil.packBasicData(latVel.get());
	packageUtil.packBasicData(xAcc.get());
	packageUtil.packBasicData(yAcc.get());

	packageUtil.packBasicData(toRemoved);
	packageUtil.packBasicData(dynamic_seed);
}

void sim_mob::Agent::unpackProxy(UnPackageUtils& unpackageUtil) {
	id = unpackageUtil.unpackBasicData<int> ();
	//isSubscriptionListBuilt = unpackageUtil.unpackBasicData<bool> ();
	startTime = unpackageUtil.unpackBasicData<int> ();

	int x_pos, y_pos;
	double x_acc, y_acc;
	double x_vel, y_vel;

	x_pos = unpackageUtil.unpackBasicData<int> ();
	y_pos = unpackageUtil.unpackBasicData<int> ();
	x_acc = unpackageUtil.unpackBasicData<double> ();
	y_acc = unpackageUtil.unpackBasicData<double> ();
	x_vel = unpackageUtil.unpackBasicData<double> ();
	y_vel = unpackageUtil.unpackBasicData<double> ();

	xPos.force(x_pos);
	yPos.force(y_pos);
	xAcc.force(x_acc);
	yAcc.force(y_acc);
	fwdVel.force(x_vel);
	latVel.force(y_vel);

	toRemoved = unpackageUtil.unpackBasicData<bool> ();
	dynamic_seed = unpackageUtil.unpackBasicData<int> ();
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
