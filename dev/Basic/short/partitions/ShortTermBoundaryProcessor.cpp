/*
 * ShortTermBoundaryProcessor.cpp
 *
 */

#include "conf/settings/DisableMPI.h"

#ifndef SIMMOB_DISABLE_MPI
#include "mpi.h"
#include <boost/mpi.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <limits>

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "ShortTermBoundaryProcessor.hpp"

#include "util/GeomHelpers.hpp"
#include "entities/AuraManager.hpp"
#include "util/OutputUtil.hpp"
#include "conf/simpleconf.hpp"
#include "workers/WorkGroup.hpp"

#include "geospatial/Node.hpp"
#include "geospatial/RoadSegment.hpp"

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"

#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/Role.hpp"

#include "partitions/ParitionDebugOutput.hpp"
#include "entities/signal/Signal.hpp"
#include "partitions/PartitionManager.hpp"

#define BOUNDARY_BOX_SIZE 4

namespace mpi = boost::mpi;

using std::string;
using std::vector;
using namespace sim_mob;

//Anonymous namespace for private, helper functions.
namespace {
bool isOneagentInPolygon(int location_x, int location_y, BoundarySegment* boundary_segment)
{
	int node_size = boundary_segment->bounary_box.size();
	if (node_size != BOUNDARY_BOX_SIZE)
	{
		std::cerr << "Boundary Segment's boundary should have 4 nodes, but not." << std::endl;
	}

	Point2D pointlist[BOUNDARY_BOX_SIZE + 1];
	Point2D agent_location(location_x, location_y);

	int index = 0;
	vector<Point2D>::iterator itr = boundary_segment->bounary_box.begin();
	for (; itr != boundary_segment->bounary_box.end(); itr++)
	{
		Point2D point((*itr).getX(), (*itr).getY());
		pointlist[index] = point;
		index++;
	}

	Point2D last_point(pointlist[0].getX(), pointlist[0].getY());
	pointlist[index] = last_point;

	return sim_mob::PointInsidePolygon(pointlist, BOUNDARY_BOX_SIZE + 1, agent_location);
}

void outputLineT(Point2D& start_p, Point2D& end_p, string color)
{
	static int line_id = 100;
	if (line_id < 105) {
		LogOut("(" << "\"CutLine\"," << "0," << line_id++ << "," << "{\"startPointX\":\"" << start_p.getX() << "\","
				<< "\"startPointY\":\"" << start_p.getY() << "\"," << "\"endPointX\":\"" << end_p.getX() << "\","
				<< "\"endPointY\":\"" << end_p.getY() << "\"," << "\"color\":\"" << color << "\"" << "})" << std::endl);
	}
}

const Signal* findOneSignalByNode(const Point2D& point)
{
	for (size_t i=0; i<Signal::all_signals_.size(); i++) {
		if (Signal::all_signals_.at(i)->getNode().location == point) {
			return Signal::all_signals_.at(i);
		}
	}

	return nullptr;
}

} //End anonymous namespace



sim_mob::ShortTermBoundaryProcessor::ShortTermBoundaryProcessor()
{
	entity_group = nullptr;
	singal_group = nullptr;
	scenario = nullptr;
	partition_config = nullptr;

	neighbor_ips.clear();
//		downstream_ips.clear();
}


string sim_mob::ShortTermBoundaryProcessor::boundaryProcessing(int time_step)
{
	//step 1, update fake agents (received in previous time step)
	ParitionDebugOutput debug;
	clearFakeAgentFlag();

	//step 2, check the agents that should be send to downstream partitions
	int neighbor_size = neighbor_ips.size();
	BoundaryProcessingPackage sendout_package[neighbor_size];


	size_t index = 0;
	std::set<int>::iterator itr_neighbor = neighbor_ips.begin();
	for (; itr_neighbor != neighbor_ips.end(); itr_neighbor++)
	{
		BoundaryProcessingPackage& currPackage = sendout_package[index++];
		currPackage.from_id = partition_config->partition_id;
		currPackage.to_id = (*itr_neighbor);

		currPackage.boundary_signals.clear();
		currPackage.cross_persons.clear();
		currPackage.feedback_persons.clear();
	}

	checkBoundaryAgents(sendout_package);

	//Step 3, commmunicate with downstream
	mpi::communicator world;
	mpi::request recvs[neighbor_size];
	mpi::request sends[neighbor_size];
	string all_received_package_data[neighbor_size];

	//ready to receive
	std::set<int>::iterator itr_upstream = neighbor_ips.begin();
	index = 0;
	for (; itr_upstream != neighbor_ips.end(); itr_upstream++)
	{
		int upstream_id = (*itr_upstream);
		recvs[index] = world.irecv(upstream_id, (time_step) % 99 + 1, all_received_package_data[index]);
		index++;
	}

	//ready to send
	itr_neighbor = neighbor_ips.begin();
	index = 0;
	for (; itr_neighbor != neighbor_ips.end(); itr_neighbor++)
	{
		string data = getDataInPackage(sendout_package[index]);
		sends[index] = world.isend(sendout_package[index].to_id, (time_step) % 99 + 1, data);
		index = 0;
	}

	//waiting for the end of sending and recving
	mpi::wait_all(recvs, recvs + neighbor_size);
	mpi::wait_all(sends, sends + neighbor_size);

	//Step 4
	processBoundaryPackages(all_received_package_data, neighbor_size);
	return "";
}

/**
 * xuyan:
 * The rule is that:
 * if one fake agent is not received for the next time step,
 * then, it means the fake agent has moved outside of the boundary area,
 * So, we need to remove the fake agent.
 */
void sim_mob::ShortTermBoundaryProcessor::clearFakeAgentFlag()
{
	vector<Entity*>::iterator itr = Agent::all_agents.begin();
	while (itr != Agent::all_agents.end()) {
		if (((*itr)->isFake) && ((*itr)->receiveTheFakeEntityAgain == false)) {
			itr = Agent::all_agents.erase(itr);
			releaseFakeAgentMemory(*itr);
		} else {
			itr++;
		}
	}

	itr = Agent::all_agents.begin();
	for (; itr != Agent::all_agents.end(); itr++)
	{
		if ((*itr)->isFake)
		{
			(*itr)->receiveTheFakeEntityAgain = false;
		}
	}

	itr = Agent::all_agents.begin();
	for (; itr != Agent::all_agents.end(); itr++)
	{
		if ((*itr)->isFake && ((*itr)->receiveTheFakeEntityAgain == true))
		{
			std::cout << "Error" << std::endl;
			return;
		}
	}
}

string sim_mob::ShortTermBoundaryProcessor::checkBoundaryAgents(BoundaryProcessingPackage* sendout_package)
{
	ParitionDebugOutput debug;

	std::map<string, BoundarySegment*>::iterator loopRoadSegment;

//	debug.outputToConsole("Start to check checkBoundaryAgents");
//	debug.outputToConsole(boundary_segments.size());

	for (loopRoadSegment = boundary_segments.begin(); loopRoadSegment != boundary_segments.end(); ++loopRoadSegment)
	{
		BoundarySegment* segment = loopRoadSegment->second;

		int downstream_id = segment->connected_partition_id - 1;
		int downstream_index = -1;
		for (int i = 0; i < neighbor_ips.size(); i++)
		{
			if (sendout_package[i].to_id == downstream_id)
			{
				downstream_index = i;
				break;
			}
		}

		//get all agents in box
		vector<Agent const*> allAgentsInBoundary;
		allAgentsInBoundary = agentsInSegmentBoundary(segment);

		//nothing to process
		if(allAgentsInBoundary.size() <= 0)
			continue;

		if (segment->responsible_side == -1)
		{
			vector<Agent const*>::iterator agent_pointer;

			for (agent_pointer = allAgentsInBoundary.begin(); agent_pointer != allAgentsInBoundary.end(); agent_pointer++)
			{
				if ((*agent_pointer)->isFake || (*agent_pointer)->isToBeRemoved())
				{
					continue;
				}

				//TODO: What do we do if the vehicle is already done with its path?
				//      Currently, this will lead to undefined behavior if the BoundarySegment is
				//      DIRECTLY at the end of the Agent's route, and the Agent lands on
				//      that line EXACTLY. So, this is probably ok, but I'd rather not rely
				//      on this behvaior in the future. ~Seth
				{//Temporary bugfix.
					const Person* p = dynamic_cast<const Person*> (*agent_pointer);
					if (p)
					{
						const Driver* d = dynamic_cast<const Driver*> (p->getRole());
						if (d && d->getVehicle()->isDone())
						{
							continue; //Avoid crashing on a call to getCurrSegment()
						}
					}
				}

				if ((getAgentTypeForSerialization(*agent_pointer) != DRIVER_TYPE) && (getAgentTypeForSerialization(*agent_pointer) != PEDESTRIAN_TYPE))
				{
					continue;
				}

//				debug.outputToConsole("Start to check whether one vehicle cross the line");

				if (isAgentCrossed(segment, *agent_pointer, false))
				{
//					debug.outputToConsole("Succeed to cross");

					const Person *person = dynamic_cast<const Person *> (*agent_pointer);
					if (person)
					{
//						debug.outputToConsole("Add the cross vehicle");
						sendout_package[downstream_index].cross_persons.push_back(person);

						Person *one_person = const_cast<Person *> (person);
						changeAgentToFake(one_person);
					}
				}
//				else
//				{
////					debug.outputToConsole("Did not cross");
//				}

				if (isAgentInFeedbackorForward(segment, *agent_pointer, false))
				{
					if ((*agent_pointer)->isFake == false)
					{
						const Person *person = dynamic_cast<const Person *> (*agent_pointer);
						if (person)
							sendout_package[downstream_index].feedback_persons.push_back(person);
					}
				}
			}

		}
		else
		{
			vector<Agent const*>::iterator agent_pointer;
			for (agent_pointer = allAgentsInBoundary.begin(); agent_pointer != allAgentsInBoundary.end(); ++agent_pointer)
			{
				if ((*agent_pointer)->isFake || (*agent_pointer)->isToBeRemoved())
					continue;

				if ((getAgentTypeForSerialization(*agent_pointer) != DRIVER_TYPE) && (getAgentTypeForSerialization(*agent_pointer) != PEDESTRIAN_TYPE))
				{
					continue;
				}

				if (isAgentInFeedbackorForward(segment, *agent_pointer, true))
				{
					const Person *person = dynamic_cast<const Person *> (*agent_pointer);
					if (person)
						sendout_package[downstream_index].feedback_persons.push_back(person);
				}
			}
		}
	}

	std::set<const Entity*>::iterator itr = boundaryRealTrafficItems.begin();

	for (; itr != boundaryRealTrafficItems.end(); itr++)
	{
		const Entity* one_entity = (*itr);
		unsigned int entity_id = one_entity->getId();

		const Signal *one_const_signal = dynamic_cast<const Signal *> (one_entity);
		if (one_const_signal)
		{
			Signal* one_signal = const_cast<Signal*> (one_const_signal);
			std::map<int, int>::iterator itr = traffic_items_mapping_to.find(entity_id);

			if (itr != traffic_items_mapping_to.end())
			{
				int downstream_id = (*itr).second;
				int downstream_index = -1;

				for (int i = 0; i < neighbor_ips.size(); i++)
				{
					if (sendout_package[i].to_id == downstream_id)
					{
						downstream_index = i;
						break;
					}
				}

				if(downstream_index > -1)
				{
					sendout_package[downstream_index].boundary_signals.push_back(one_signal);
				}

			}
		}
	}
	return "";
}

string sim_mob::ShortTermBoundaryProcessor::getDataInPackage(BoundaryProcessingPackage& package)
{
	ParitionDebugOutput debug;
	PackageUtils packageUtil;
	int cross_size = package.cross_persons.size();

	//package cross agents
	packageUtil << (cross_size);
//	debug.outputToConsole("cross_size");
//	debug.outputToConsole(cross_size);

	vector<Person const*>::iterator itr_cross = package.cross_persons.begin();
	for (; itr_cross != package.cross_persons.end(); itr_cross++)
	{

		ShortTermBoundaryProcessor::Agent_Type type = getAgentTypeForSerialization(*itr_cross);
		packageUtil << ((int) (type));

		Person* one_person = const_cast<Person*> (*itr_cross);
		one_person->pack(packageUtil);
		one_person->getRole()->pack(packageUtil);
	}

//	double test_1 = 100.80;
//	packageUtil<<(test_1);

	//package feedback agents
//	std::cout << "package.feedback_persons.size():" << package.feedback_persons.size() << std::endl;
	int feedback_size = package.feedback_persons.size();
	packageUtil<<(feedback_size);
//	debug.outputToConsole("feedback_persons size");
//	debug.outputToConsole(package.feedback_persons.size());

	vector<Person const*>::iterator itr_feedback = package.feedback_persons.begin();

	for (; itr_feedback != package.feedback_persons.end(); itr_feedback++)
	{
		ShortTermBoundaryProcessor::Agent_Type type = getAgentTypeForSerialization(*itr_feedback);
		packageUtil<<((int) (type));

		int agent_id = (*itr_feedback)->getId();
		packageUtil<<(agent_id);

		Person* one_person = const_cast<Person*> (*itr_feedback);

		one_person->packProxy(packageUtil);
//		debug.outputToConsole("packProxy");

//		double test_3 = 13.33;
//		packageUtil<<(test_3);

		one_person->getRole()->packProxy(packageUtil);
//		debug.outputToConsole("pack Role");

//		double test_2 = 13.80;
//		packageUtil<<(test_2);
	}

//	double test = 12.80;
//	packageUtil<<(test);

	//package signal
	int signal_size = package.boundary_signals.size();
	packageUtil<<(signal_size);
//	debug.outputToConsole("package.boundary_signals.size():");
//	debug.outputToConsole(package.boundary_signals.size());

	vector<Signal const*>::iterator itr_signal = package.boundary_signals.begin();
	for (; itr_signal != package.boundary_signals.end(); itr_signal++)
	{
		packageUtil << ((*itr_signal)->getNode().location);

		Signal* one_signal = const_cast<Signal*> (*itr_signal);
		one_signal->packProxy(packageUtil);
	}

	return (packageUtil.getPackageData());
}

void sim_mob::ShortTermBoundaryProcessor::processPackageData(string data)
{
	ParitionDebugOutput debug;
	UnPackageUtils unpackageUtil(data);

	int cross_size = 0;
	unpackageUtil >> cross_size;

//	std::cout << "DDDDDDDDD:" << cross_size << std::endl;
//	debug.outputToConsole("receive 11");

	for (int i = 0; i < cross_size; i++)
	{
		int type = 0;
		unpackageUtil >> type;

		switch (type)
		{
		case DRIVER_TYPE: {
			Person* one_person = new Person("XML_Def", ConfigParams::GetInstance().mutexStategy);
			Driver* one_driver = new Driver(one_person, ConfigParams::GetInstance().mutexStategy);
			one_person->changeRole(one_driver);

			one_person->unpack(unpackageUtil);
			one_driver->unpack(unpackageUtil);

			if (isCrossAgentShouldBeInsert(one_person))
			{
				insertOneAgentToWorkerGroup(one_person);
			}
		}

			break;
		case PEDESTRIAN_TYPE: {
			Person* one_person = new Person("XML_Def", ConfigParams::GetInstance().mutexStategy);
			Pedestrian* one_pedestrian = new Pedestrian(one_person);
			one_person->changeRole(one_pedestrian);

			one_person->unpack(unpackageUtil);
			one_pedestrian->unpack(unpackageUtil);

			if (isCrossAgentShouldBeInsert(one_person))
			{
				insertOneAgentToWorkerGroup(one_person);
			}
		}
			break;
		case 3:
			break;
		}
	}

//	debug.outputToConsole("receive 22");
//	double test_1 = 1;
//	unpackageUtil >> (test_1);

//	debug.outputToConsole("test_1");
//	debug.outputToConsole(test_1);

	//feedback agents
	int feedback_size = 0;
	unpackageUtil >> feedback_size;

//	debug.outputToConsole("feedback_size");
//	debug.outputToConsole(feedback_size);
//	std::cout << "receive feedback_size:" << feedback_size << std::endl;

	for (int i = 0; i < feedback_size; i++)
	{
		int value = 0;
		unpackageUtil >> value;
//		debug.outputToConsole("value");
//		debug.outputToConsole(value);

		int agent_id = 0;
		unpackageUtil >> agent_id;

		Person* one_person = getFakePersonById(agent_id);

		if (one_person)
		{
//			debug.outputToConsole("receive 23");
			one_person->unpackProxy(unpackageUtil);
			one_person->getRole()->unpackProxy(unpackageUtil);

			one_person->isFake = true;
			one_person->receiveTheFakeEntityAgain = true;
//			one_person->toRemoved = false;
		}
		else
		{
//			debug.outputToConsole("receive 26");
			ConfigParams& config = ConfigParams::GetInstance();

			switch (value)
			{
			case DRIVER_TYPE:

//				debug.outputToConsole("receive 27");
				one_person = new Person("XML_Def", ConfigParams::GetInstance().mutexStategy, -1);
				one_person->changeRole(new Driver(one_person, ConfigParams::GetInstance().mutexStategy));

//				debug.outputToConsole("receive 28");
				one_person->unpackProxy(unpackageUtil);

//				debug.outputToConsole("receive 29");
				one_person->getRole()->unpackProxy(unpackageUtil);

//				debug.outputToConsole("receive 30");

				if (isAgentInLocalPartition(one_person->getId(), false))
					continue;
//				debug.outputToConsole("receive 31");

				one_person->isFake = true;
				one_person->receiveTheFakeEntityAgain = true;
//				one_person->toRemoved = false;
				insertOneFakeAgentToWorkerGroup(one_person);
//				debug.outputToConsole("receive 32");
				break;

			case PEDESTRIAN_TYPE:
//				debug.outputToConsole("receive 311");
				one_person = new Person("XML_Def", ConfigParams::GetInstance().mutexStategy, -1);
				one_person->changeRole(new Pedestrian(one_person));

				one_person->unpackProxy(unpackageUtil);
				one_person->getRole()->unpackProxy(unpackageUtil);

				if (isAgentInLocalPartition(one_person->getId(), false))
					continue;

				one_person->isFake = true;
				one_person->receiveTheFakeEntityAgain = true;
//				one_person->toRemoved = false;
				insertOneFakeAgentToWorkerGroup(one_person);
				break;
			case 3:
//				debug.outputToConsole("receive 312");
				break;

			default:
//				debug.outputToConsole("receive 313");
				break;
			}
		}

//		double test_2 = 1;
//		unpackageUtil >> (test_2);
//
//		debug.outputToConsole("test_2");
//		debug.outputToConsole(test_2);
	}

//	double test = 1;
//	unpackageUtil >> (test);
//
//	debug.outputToConsole("receive 33");
//	debug.outputToConsole(test);

//	//feedback signal
	int signal_size = 0;
	unpackageUtil >> signal_size;

	for (int i = 0; i < signal_size; i++)
	{
		Point2D location;
		unpackageUtil >> location;

		Signal* one_signal = const_cast<Signal*> (getSignalBasedOnNode(&location));
		one_signal->unpackProxy(unpackageUtil);
	}
//	debug.outputToConsole("receive 44");
}

string sim_mob::ShortTermBoundaryProcessor::processBoundaryPackages(string all_packages[], int size)
{
	for (int i = 0; i < size; i++)
	{
		processPackageData(all_packages[i]);
	}

	return "";
}

Person* sim_mob::ShortTermBoundaryProcessor::getFakePersonById(unsigned int agent_id)
{
	vector<Entity*>::iterator itr = Agent::all_agents.begin();

	for (; itr != Agent::all_agents.end(); itr++)
	{
		if ((*itr)->isFake && (*itr)->getId() == agent_id)
		{
			Person* person = dynamic_cast<Person *> (*itr);
			if (person)
				return person;
		}
	}

	return nullptr;
}

bool sim_mob::ShortTermBoundaryProcessor::isAgentCrossed(BoundarySegment* segment, Agent const* agent, bool is_down_boundary)
{
	ParitionDebugOutput debug;

	if (agent->isFake)
		return false;

//	debug.outputToConsole("Check 1");

	//downside
	if (is_down_boundary)
		return false;

//	debug.outputToConsole("Check 2");

	if (getAgentTypeForSerialization(agent) == DRIVER_TYPE)
	{
//		debug.outputToConsole("Check 3");

		const Person *person = dynamic_cast<const Person *> (agent);
		Person* p = const_cast<Person*> (person);
		const Driver *driver = dynamic_cast<const Driver *> (p->getRole());

		//normal check
//		debug.outputToConsole("*********************************************************");
//		debug.outputToConsole(driver->getVehicle()->getCurrSegment()->getStart()->location.getX());
//		debug.outputToConsole(driver->getVehicle()->getCurrSegment()->getStart()->location.getY());
//		debug.outputToConsole(driver->getVehicle()->getCurrSegment()->getEnd()->location.getX());
//		debug.outputToConsole(driver->getVehicle()->getCurrSegment()->getEnd()->location.getY());
//		debug.outputToConsole("---------------------------------------------------------");
//		debug.outputToConsole(segment->boundarySegment->getStart()->location.getX());
//		debug.outputToConsole(segment->boundarySegment->getStart()->location.getY());
//		debug.outputToConsole(segment->boundarySegment->getEnd()->location.getX());
//		debug.outputToConsole(segment->boundarySegment->getEnd()->location.getY());

		if (driver->getVehicle()->getCurrSegment()->getStart()->location.getX() != segment->boundarySegment->getStart()->location.getX())
		{
			return false;
		}

//		debug.outputToConsole("Check 4");

		if (driver->getVehicle()->getCurrSegment()->getStart()->location.getY() != segment->boundarySegment->getStart()->location.getY())
		{
			return false;
		}
		if (driver->getVehicle()->getCurrSegment()->getEnd()->location.getX() != segment->boundarySegment->getEnd()->location.getX())
		{
			return false;
		}
		if (driver->getVehicle()->getCurrSegment()->getEnd()->location.getY() != segment->boundarySegment->getEnd()->location.getY())
		{
			return false;
		}

//		debug.outputToConsole("Check 5");

		int x_cut_dis = segment->cut_line_to->getX() - segment->cut_line_start->getX();
		int y_cut_dis = segment->cut_line_to->getY() - segment->cut_line_start->getY();

		int x_pos = person->xPos.get();
		int y_pos = person->yPos.get();



		int pos_line = (x_pos - segment->cut_line_start->getX()) * y_cut_dis - (y_pos - segment->cut_line_start->getY()) * x_cut_dis;
		if (pos_line == 0)
			return false;
//
//		debug.outputToConsole("Check 6");

		int upper_x_pos = segment->boundarySegment->getStart()->location.getX();
		int upper_y_pos = segment->boundarySegment->getStart()->location.getY();

		int upper_pos_line = (upper_x_pos - segment->cut_line_start->getX()) * y_cut_dis - (upper_y_pos - segment->cut_line_start->getY()) * x_cut_dis;

//		debug.outputToConsole("Check 7");

		if (upper_pos_line > 0 && pos_line > 0)
			return false;

//		debug.outputToConsole("Check 8");

		if (upper_pos_line < 0 && pos_line < 0)
			return false;

//		debug.outputToConsole("Check 9");

		return true;

	}
	else if (getAgentTypeForSerialization(agent) == PEDESTRIAN_TYPE)
	{
		const Person *person = dynamic_cast<const Person *> (agent);

		//calcuate the relationship between point and line
		int y_cut_dis = segment->cut_line_to->getY() - segment->cut_line_start->getY();
		int x_cut_dis = segment->cut_line_to->getX() - segment->cut_line_start->getX();

		int x_pos = person->xPos.get();
		int y_pos = person->yPos.get();

		int pos_line = (x_pos - segment->cut_line_start->getX()) * y_cut_dis - (y_pos - segment->cut_line_start->getY()) * x_cut_dis;
		if (pos_line == 0)
			return false;

		int upper_x_pos = segment->boundarySegment->getStart()->location.getX();
		int upper_y_pos = segment->boundarySegment->getStart()->location.getY();

		int upper_pos_line = (upper_x_pos - segment->cut_line_start->getX()) * y_cut_dis - (upper_y_pos - segment->cut_line_start->getY()) * x_cut_dis;

		if (upper_pos_line > 0 && pos_line > 0)
			return false;

		if (upper_pos_line < 0 && pos_line < 0)
			return false;

		return true;
	}

//	debug.outputToConsole("Check 10");

	return false;
}

bool sim_mob::ShortTermBoundaryProcessor::isAgentInFeedbackorForward(BoundarySegment* segment, Agent const* agent, bool is_down_boundary)
{
	if (agent->isFake)
		return false; //Error, should not happen;

	return true;
}

void sim_mob::ShortTermBoundaryProcessor::changeAgentToFake(Agent * agent)
{
	agent->isFake = true;
	agent->receiveTheFakeEntityAgain = true;
//	agent->toRemoved = false;

	entity_group->removeAgentFromWorker(agent);
}

void sim_mob::ShortTermBoundaryProcessor::insertOneAgentToWorkerGroup(Agent * agent)
{
	agent->isFake = false;
	agent->receiveTheFakeEntityAgain = false;
//	agent->toRemoved = false;

	Agent::all_agents.push_back(agent);
	entity_group->addAgentInWorker(agent);
}

void sim_mob::ShortTermBoundaryProcessor::insertOneFakeAgentToWorkerGroup(Agent * agent)
{
	agent->isFake = true;
	agent->receiveTheFakeEntityAgain = true;
//	agent->toRemoved = false;

	Agent::all_agents.push_back(agent);
}

void sim_mob::ShortTermBoundaryProcessor::removeOneFakeAgentFromWorkerGroup(Agent * agent)
{
	vector<Entity*>::iterator position = std::find(Agent::all_agents.begin(), Agent::all_agents.end(), agent);

	if (position != Agent::all_agents.end())
	{
		Agent::all_agents.erase(position);
	}
}

bool sim_mob::ShortTermBoundaryProcessor::isAgentInLocalPartition(unsigned int agent_id, bool includeFakeAgent)
{

	vector<Entity*>::iterator it = Agent::all_agents.begin();
	for (; it != Agent::all_agents.end(); it++)
	{
		if (includeFakeAgent)
		{
			if ((*it)->getId() == agent_id)
				return true;
		}
		else
		{
			if ((*it)->getId() == agent_id && (*it)->isFake == false)
				return true;
		}
	}

	return false;
}

bool sim_mob::ShortTermBoundaryProcessor::isCrossAgentShouldBeInsert(const Agent* agent)
{
	if (isAgentInLocalPartition(agent->getId(), false))
	{
		return false;
	}

	return true;
}

BoundarySegment* sim_mob::ShortTermBoundaryProcessor::getBoundarySegmentByID(string segmentID)
{
	std::map<string, BoundarySegment*>::iterator it;
	it = boundary_segments.find(segmentID);

	if (it != boundary_segments.end())
		return it->second;

	return 0;
}

//string sim_mob::ShortTermBoundaryProcessor::outputAllEntities(timeslice now)
//{
//
//	vector<Entity*>::iterator it = Agent::all_agents.begin();
//	for (; it != Agent::all_agents.end(); it++)
//	{
//
//		Person* one_agent = dynamic_cast<Person*> (*it);
//
//		if ((one_agent) && (one_agent->toRemoved == false)) {
//			one_agent->currRole->frame_tick_output_mpi(now.frame());
//		}
//	}
//
//	//signal output every 10 seconds
//	if (now.frame() % ConfigParams::GetInstance().granSignalsTicks != 0)
//	{
//		return "";
//	}
//
//	All_Signals::iterator itr_sig = Signal::all_signals_.begin();
//	for (; itr_sig != Signal::all_signals_.end(); itr_sig++)
//	{
//		Signal* one_signal = (*itr_sig);
//		one_signal->outputTrafficLights(now.frame(),"");
//	}
//
//	return "";
//}

//very simply version
//need to build virtual destructors
void sim_mob::ShortTermBoundaryProcessor::releaseFakeAgentMemory(Entity* agent)
{
}


//need to be changed to polygon2D
vector<Agent const *> sim_mob::ShortTermBoundaryProcessor::agentsInSegmentBoundary(BoundarySegment* boundary_segment)
{

	//	//get the boundary box
	int box_maximum_x = 0;
	int box_maximum_y = 0;
	int box_minumum_x = std::numeric_limits<int>::max();
	int box_minumum_y = std::numeric_limits<int>::max();

	//get all agents in the box
	vector<Point2D>::iterator itr = boundary_segment->bounary_box.begin();
	for (; itr != boundary_segment->bounary_box.end(); itr++)
	{
		if ((*itr).getX() > box_maximum_x)
			box_maximum_x = (*itr).getX();

		if ((*itr).getY() > box_maximum_y)
			box_maximum_y = (*itr).getY();

		if ((*itr).getX() < box_minumum_x)
			box_minumum_x = (*itr).getX();

		if ((*itr).getY() < box_minumum_y)
			box_minumum_y = (*itr).getY();
	}

	Point2D lower_left_point(box_minumum_x, box_minumum_y);
	Point2D upper_right_point(box_maximum_x, box_maximum_y);

	AuraManager& auraMgr = AuraManager::instance();
	vector<Agent const *> all_agents = auraMgr.agentsInRect(lower_left_point, upper_right_point);

	vector<Agent const *> agents_after_filter;
	vector<Agent const *>::iterator filter_itr = all_agents.begin();
	for (; filter_itr != all_agents.end(); filter_itr++)
	{
		if ((*filter_itr)->isFake)
			continue;

		if (isOneagentInPolygon((*filter_itr)->xPos.get(), (*filter_itr)->yPos.get(), boundary_segment))
		{
			agents_after_filter.push_back(*filter_itr);
		}
	}

	return agents_after_filter;
}

void sim_mob::ShortTermBoundaryProcessor::initBoundaryTrafficItems()
{
	std::map<string, BoundarySegment*>::iterator itr;

	for (itr = boundary_segments.begin(); itr != boundary_segments.end(); itr++)
	{
		BoundarySegment* one_segment = itr->second;

		//if the road segment is in the upper side
		if (one_segment->responsible_side == -1)
		{
			const Signal* startNodeSignal = findOneSignalByNode(one_segment->boundarySegment->getStart()->location);

			if (startNodeSignal)
			{
				int downstream_id = one_segment->connected_partition_id - 1;
				int signal_id = startNodeSignal->getId();
				traffic_items_mapping_to[signal_id] = downstream_id;
				boundaryRealTrafficItems.insert(startNodeSignal);
			}

			const Signal* endNodeSignal = findOneSignalByNode(one_segment->boundarySegment->getEnd()->location);
			if (endNodeSignal)
			{
				Signal* one_signal = const_cast<Signal*> (endNodeSignal);
				one_signal->isFake = true;
			}
		}
		//down stream
		else if (one_segment->responsible_side == 1)
		{
			const Signal* startNodeSignal = findOneSignalByNode(one_segment->boundarySegment->getStart()->location);

			if (startNodeSignal)
			{
				if (startNodeSignal->isFake == false)
				{
					Signal* one_signal = const_cast<Signal*> (startNodeSignal);
					one_signal->isFake = true;
				}
			}

			const Signal* endNodeSignal = findOneSignalByNode(one_segment->boundarySegment->getEnd()->location);
			if (endNodeSignal)
			{
				int downstream_id = one_segment->connected_partition_id - 1;
				int signal_id = endNodeSignal->getId();
				traffic_items_mapping_to[signal_id] = downstream_id;

				boundaryRealTrafficItems.insert(endNodeSignal);
			}
		}
	}
}

string sim_mob::ShortTermBoundaryProcessor::releaseResources()
{
	if (partition_config->partition_size < 1)
		return "";

	MPI_Finalize();

	safe_delete_item(scenario);
	/*if (scenario)
	{
		delete scenario;
	}*/

	safe_delete_item(partition_config);
	/*if (partition_config)
	{
		delete partition_config;
	}*/

	std::map<string, BoundarySegment*>::iterator it2 = boundary_segments.begin();
	for (; it2 != boundary_segments.end(); it2++)
	{
		safe_delete_item(it2->second);
		/*if (it2->second)
		{
			delete it2->second;
		}*/
	}

	return "";
}

void sim_mob::ShortTermBoundaryProcessor::setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group)
{
	this->entity_group = entity_group;
	this->singal_group = singal_group;
}

void sim_mob::ShortTermBoundaryProcessor::loadInBoundarySegment(string id, BoundarySegment* boundary)
{
	boundary_segments[id] = boundary;

	//update neighbour ids
	neighbor_ips.insert(boundary->connected_partition_id - 1);

	boundary->buildBoundaryBox(partition_config->boundary_length, partition_config->boundary_width);
	boundary->output();
}

void sim_mob::ShortTermBoundaryProcessor::setConfigure(PartitionConfigure* partition_config, SimulationScenario* scenario)
{
	this ->partition_config = partition_config;
	this ->scenario = scenario;
}

ShortTermBoundaryProcessor::Agent_Type sim_mob::ShortTermBoundaryProcessor::getAgentTypeForSerialization(Agent const* agent)
{
	const Person *person = dynamic_cast<const Person *> (agent);
	if (!person)
	{
		const Signal *one_signal = dynamic_cast<const Signal *> (agent);
		if (!one_signal)
		{
			return NO_TYPE;
		}
		else
		{
			return SIGNAL_TYPE;
		}
	}
	else
	{
		Person* p = const_cast<Person*> (person);

		const Driver *driver = dynamic_cast<const Driver *> (p->getRole());
		if (driver)
		{
			return DRIVER_TYPE;
		}

		const Pedestrian *pedestrian = dynamic_cast<const Pedestrian *> (p->getRole());
		if (pedestrian)
		{
			return PEDESTRIAN_TYPE;
		}

		const Passenger *passenger = dynamic_cast<const Passenger *> (p->getRole());
		if (passenger)
		{
			return PASSENGER_TYPE;
		}

		return NO_TYPE;
	}
}


#endif
