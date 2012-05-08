/*
 * BoundaryProcessor.cpp
 *
 */

#include "BoundaryProcessor.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "mpi.h"
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include <CGAL/Homogeneous.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <limits>

#include "PackageUtils.hpp"
#include "UnPackageUtils.hpp"
#include "BoundaryProcessor.hpp"

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

#include "entities/Signal.hpp"
#include "PartitionManager.hpp"
#include "ParitionDebugOutput.hpp"


typedef CGAL::Homogeneous<double> Rep_class;
typedef CGAL::Polygon_2<Rep_class> Polygon_2;
typedef CGAL::Point_2<Rep_class> Point;

namespace mpi = boost::mpi;

using namespace sim_mob;



//Anonymous namespace for private, helper functions.
namespace {
bool isOneagentInPolygon(int location_x, int location_y, BoundarySegment* boundary_segment)
{
	Point agent_location(location_x, location_y);
	std::vector<Point> points;

	std::vector<Point2D>::iterator itr = boundary_segment->bounary_box.begin();
	for (; itr != boundary_segment->bounary_box.end(); itr++)
	{
		Point point((*itr).getX(), (*itr).getY());
		points.push_back(point);
	}

	switch (CGAL::bounded_side_2(points.begin(), points.end(), agent_location))
	{
	case CGAL::ON_BOUNDED_SIDE:
		return true;
	case CGAL::ON_BOUNDARY:
		return false;
	case CGAL::ON_UNBOUNDED_SIDE:
		return false;
	}

	return false;
}

void outputLineT(Point2D& start_p, Point2D& end_p, std::string color)
{
	static int line_id = 100;
	if (line_id < 105)
		LogOut("(" << "\"CutLine\"," << "0," << line_id++ << "," << "{\"startPointX\":\"" << start_p.getX() << "\","
				<< "\"startPointY\":\"" << start_p.getY() << "\"," << "\"endPointX\":\"" << end_p.getX() << "\","
				<< "\"endPointY\":\"" << end_p.getY() << "\"," << "\"color\":\"" << color << "\"" << "})" << std::endl);
}

const Signal* findOneSignalByNode(const Point2D* point)
{
	for (size_t i = 0; i < Signal::all_signals_.size(); i++)
	{
		if (Signal::all_signals_[i]->getNode().location.getX() == point->getX() && Signal::all_signals_[i]->getNode().location.getY() == point->getY())
		{
			return Signal::all_signals_[i];
		}
	}

	return 0;
}

} //End anonymous namespace



sim_mob::BoundaryProcessor::BoundaryProcessor(): BOUNDARY_PROCOSS_TAG(2)
{
	entity_group = nullptr;
	singal_group = nullptr;
	scenario = nullptr;
	partition_config = nullptr;

	neighbor_ips.clear();
//		downstream_ips.clear();
}


std::string sim_mob::BoundaryProcessor::boundaryProcessing(int time_step)
{
	//step 1, update fake agents (received in previous time step)
	clearFakeAgentFlag();

	//step 2, check the agents that should be send to downstream partitions
	int neighbor_size = neighbor_ips.size();
	BoundaryProcessingPackage sendout_package[neighbor_size];

	int index = -1;
	std::set<int>::iterator itr_neighbor = neighbor_ips.begin();
	for (; itr_neighbor != neighbor_ips.end(); itr_neighbor++)
	{
		index++;
		sendout_package[index].from_id = partition_config->partition_id;
		sendout_package[index].to_id = (*itr_neighbor);

		sendout_package[index].boundary_signals.clear();
		sendout_package[index].cross_persons.clear();
		sendout_package[index].feedback_persons.clear();
	}

	checkBoundaryAgents(sendout_package);

	//Step 3, commmunicate with downstream
	mpi::communicator world;
	mpi::request recvs[neighbor_size];
	mpi::request sends[neighbor_size];
	std::string all_received_package_data[neighbor_size];

	//ready to receive
	std::set<int>::iterator itr_upstream = neighbor_ips.begin();
	index = -1;
	for (; itr_upstream != neighbor_ips.end(); itr_upstream++)
	{
		index++;
		int upstream_id = (*itr_upstream);
		recvs[index] = world.irecv(upstream_id, (time_step) % 99 + 1, all_received_package_data[index]);
	}

	//ready to send
	itr_neighbor = neighbor_ips.begin();
	index = -1;
	for (; itr_neighbor != neighbor_ips.end(); itr_neighbor++)
	{
		index++;
		std::string data = getDataInPackage(sendout_package[index]);
		sends[index] = world.isend(sendout_package[index].to_id, (time_step) % 99 + 1, data);
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
void sim_mob::BoundaryProcessor::clearFakeAgentFlag()
{
	std::vector<Entity*>::iterator itr = Agent::all_agents.begin();
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

std::string sim_mob::BoundaryProcessor::checkBoundaryAgents(BoundaryProcessingPackage* sendout_package)
{
	std::map<std::string, BoundarySegment*>::iterator loopRoadSegment;

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
		std::vector<Agent const*> allAgentsInBoundary;
		allAgentsInBoundary = agentsInSegmentBoundary(segment);

		if (segment->responsible_side == -1)
		{
			std::vector<Agent const*>::iterator agent_pointer;

			for (agent_pointer = allAgentsInBoundary.begin(); agent_pointer != allAgentsInBoundary.end(); agent_pointer++)
			{
				if ((*agent_pointer)->isFake || (*agent_pointer)->toRemoved)
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

				if (isAgentCrossed(segment, *agent_pointer, false))
				{
					const Person *person = dynamic_cast<const Person *> (*agent_pointer);
					if (person)
					{
						sendout_package[downstream_index].cross_persons.push_back(person);

						Person *one_person = const_cast<Person *> (person);
						changeAgentToFake(one_person);
					}
				}

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
			std::vector<Agent const*>::iterator agent_pointer;
			for (agent_pointer = allAgentsInBoundary.begin(); agent_pointer != allAgentsInBoundary.end(); ++agent_pointer)
			{
				if ((*agent_pointer)->isFake || (*agent_pointer)->toRemoved)
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

std::string sim_mob::BoundaryProcessor::getDataInPackage(BoundaryProcessingPackage& package)
{
	PackageUtils packageUtil;

	//package cross agents
	packageUtil.packBasicData(package.cross_persons.size());

	std::vector<Person const*>::iterator itr_cross = package.cross_persons.begin();
	for (; itr_cross != package.cross_persons.end(); itr_cross++)
	{

		BoundaryProcessor::Agent_Type type = getAgentTypeForSerialization(*itr_cross);
		packageUtil.packBasicData((int) (type));

		Person* one_person = const_cast<Person*> (*itr_cross);

		one_person->pack(packageUtil);

		one_person->getRole()->pack(packageUtil);

	}

	//package feedback agents
	packageUtil.packBasicData(package.feedback_persons.size());

	std::vector<Person const*>::iterator itr_feedback = package.feedback_persons.begin();

	for (; itr_feedback != package.feedback_persons.end(); itr_feedback++)
	{
		BoundaryProcessor::Agent_Type type = getAgentTypeForSerialization(*itr_feedback);
		packageUtil.packBasicData((int) (type));

		int agent_id = (*itr_feedback)->getId();
		packageUtil.packBasicData(agent_id);

		Person* one_person = const_cast<Person*> (*itr_feedback);

		one_person->packProxy(packageUtil);
		one_person->getRole()->packProxy(packageUtil);
	}

	//package signal
	packageUtil.packBasicData(package.boundary_signals.size());

	std::vector<Signal const*>::iterator itr_signal = package.boundary_signals.begin();
	for (; itr_signal != package.boundary_signals.end(); itr_signal++)
	{
		packageUtil.packPoint2D((*itr_signal)->getNode().location);

		Signal* one_signal = const_cast<Signal*> (*itr_signal);
		one_signal->packProxy(packageUtil);
	}

	return (packageUtil.getPackageData());
}

void sim_mob::BoundaryProcessor::processPackageData(std::string data)
{
	UnPackageUtils unpackageUtil(data);

	int cross_size = unpackageUtil.unpackBasicData<int> ();

	for (int i = 0; i < cross_size; i++)
	{
		int type = unpackageUtil.unpackBasicData<int> ();

		switch (type)
		{
		case DRIVER_TYPE: {
			Person* one_person = new Person(ConfigParams::GetInstance().mutexStategy);
			Driver* one_driver = new Driver(one_person, ConfigParams::GetInstance().mutexStategy, 0, 0, 0);
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
			Person* one_person = new Person(ConfigParams::GetInstance().mutexStategy);
			Pedestrian* one_pedestrian = new Pedestrian(one_person, one_person->getGenerator());
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

	//feedback agents
	int feedback_size = unpackageUtil.unpackBasicData<int> ();
	for (int i = 0; i < feedback_size; i++)
	{


		int value = unpackageUtil.unpackBasicData<int> ();
		int agent_id = unpackageUtil.unpackBasicData<int> ();
		Person* one_person = getFakePersonById(agent_id);

		if (one_person)
		{
			one_person->unpackProxy(unpackageUtil);
			one_person->getRole()->unpackProxy(unpackageUtil);
			one_person->isFake = true;
			one_person->receiveTheFakeEntityAgain = true;
			one_person->toRemoved = false;
		}
		else
		{
			ConfigParams& config = ConfigParams::GetInstance();

			switch (value)
			{
			case DRIVER_TYPE:
				one_person = new Person(ConfigParams::GetInstance().mutexStategy, -1);
				one_person->changeRole(new Driver(one_person, ConfigParams::GetInstance().mutexStategy, config.reacTime_LeadingVehicle, config.reacTime_SubjectVehicle, config.reacTime_Gap));

				one_person->unpackProxy(unpackageUtil);
				one_person->getRole()->unpackProxy(unpackageUtil);
				if (isAgentInLocalPartition(one_person->getId(), false))
					continue;

				one_person->isFake = true;
				one_person->receiveTheFakeEntityAgain = true;
				one_person->toRemoved = false;
				insertOneFakeAgentToWorkerGroup(one_person);
				break;

			case PEDESTRIAN_TYPE:
				one_person = new Person(ConfigParams::GetInstance().mutexStategy, -1);
				one_person->changeRole(new Pedestrian(one_person, one_person->getGenerator()));

				one_person->unpackProxy(unpackageUtil);
				one_person->getRole()->unpackProxy(unpackageUtil);

				if (isAgentInLocalPartition(one_person->getId(), false))
					continue;

				one_person->isFake = true;
				one_person->receiveTheFakeEntityAgain = true;
				one_person->toRemoved = false;
				insertOneFakeAgentToWorkerGroup(one_person);
				break;
			case 3:
				break;

			default:
				break;
			}
		}
	}

	//feedback signal
	int signal_size = unpackageUtil.unpackBasicData<int> ();

	for (int i = 0; i < signal_size; i++)
	{
		Point2D* location = unpackageUtil.unpackPoint2D();
		Signal* one_signal = const_cast<Signal*> (getSignalBasedOnNode(location));

		one_signal->unpackProxy(unpackageUtil);
	}
}

std::string sim_mob::BoundaryProcessor::processBoundaryPackages(std::string all_packages[], int size)
{
	for (int i = 0; i < size; i++)
	{
		processPackageData(all_packages[i]);
	}

	return "";
}

Person* sim_mob::BoundaryProcessor::getFakePersonById(unsigned int agent_id)
{
	std::vector<Entity*>::iterator itr = Agent::all_agents.begin();

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

bool sim_mob::BoundaryProcessor::isAgentCrossed(BoundarySegment* segment, Agent const* agent, bool is_down_boundary)
{
	if (agent->isFake)
		return false;

	//downside
	if (is_down_boundary)
		return false;

	if (getAgentTypeForSerialization(agent) == DRIVER_TYPE)
	{
		const Person *person = dynamic_cast<const Person *> (agent);
		Person* p = const_cast<Person*> (person);
		const Driver *driver = dynamic_cast<const Driver *> (p->getRole());

		//normal check
		if (driver->getVehicle()->getCurrSegment()->getStart()->location.getX() != segment->boundarySegment->getStart()->location.getX())
		{
			return false;
		}

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

		int x_cut_dis = segment->cut_line_to->getX() - segment->cut_line_start->getX();
		int y_cut_dis = segment->cut_line_to->getY() - segment->cut_line_start->getY();

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

	return false;
}

bool sim_mob::BoundaryProcessor::isAgentInFeedbackorForward(BoundarySegment* segment, Agent const* agent, bool is_down_boundary)
{
	if (agent->isFake)
		return false; //Error, should not happen;

	return true;
}

void sim_mob::BoundaryProcessor::changeAgentToFake(Agent * agent)
{
	agent->isFake = true;
	agent->receiveTheFakeEntityAgain = true;
	agent->toRemoved = false;

	entity_group->removeAgentFromWorker(agent);
}

void sim_mob::BoundaryProcessor::insertOneAgentToWorkerGroup(Agent * agent)
{
	agent->isFake = false;
	agent->receiveTheFakeEntityAgain = false;
	agent->toRemoved = false;

	Agent::all_agents.push_back(agent);
	entity_group->addAgentInWorker(agent);
}

void sim_mob::BoundaryProcessor::insertOneFakeAgentToWorkerGroup(Agent * agent)
{
	agent->isFake = true;
	agent->receiveTheFakeEntityAgain = true;
	agent->toRemoved = false;

	Agent::all_agents.push_back(agent);
}

void sim_mob::BoundaryProcessor::removeOneFakeAgentFromWorkerGroup(Agent * agent)
{
	std::vector<Entity*>::iterator position = std::find(Agent::all_agents.begin(), Agent::all_agents.end(), agent);

	if (position != Agent::all_agents.end())
	{
		Agent::all_agents.erase(position);
	}
}

bool sim_mob::BoundaryProcessor::isAgentInLocalPartition(unsigned int agent_id, bool includeFakeAgent)
{

	std::vector<Entity*>::iterator it = Agent::all_agents.begin();
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

bool sim_mob::BoundaryProcessor::isCrossAgentShouldBeInsert(const Agent* agent)
{
	if (isAgentInLocalPartition(agent->getId(), false))
	{
		return false;
	}

	return true;
}

BoundarySegment* sim_mob::BoundaryProcessor::getBoundarySegmentByID(std::string segmentID)
{
	std::map<std::string, BoundarySegment*>::iterator it;
	it = boundary_segments.find(segmentID);

	if (it != boundary_segments.end())
		return it->second;

	return 0;
}

std::string sim_mob::BoundaryProcessor::outputAllEntities(frame_t time_step)
{

	std::vector<Entity*>::iterator it = Agent::all_agents.begin();
	for (; it != Agent::all_agents.end(); it++)
	{

		Person* one_agent = dynamic_cast<Person*> (*it);

		if ((one_agent) && (one_agent->toRemoved == false)) {
			one_agent->currRole->frame_tick_output_mpi(time_step);
		}
	}

	//signal output every 10 seconds
	if (time_step % ConfigParams::GetInstance().granSignalsTicks != 0)
	{
		return "";
	}

	std::vector<Signal*>::iterator itr_sig = Signal::all_signals_.begin();
	for (; itr_sig != Signal::all_signals_.end(); itr_sig++)
	{
		Signal* one_signal = (*itr_sig);
		one_signal->frame_output(time_step);
	}

	return "";
}

//very simply version
//need to build virtual destructors
void sim_mob::BoundaryProcessor::releaseFakeAgentMemory(Entity* agent)
{
}


//need to be changed to polygon2D
std::vector<Agent const *> sim_mob::BoundaryProcessor::agentsInSegmentBoundary(BoundarySegment* boundary_segment)
{

	//	//get the boundary box
	int box_maximum_x = 0;
	int box_maximum_y = 0;
	int box_minumum_x = std::numeric_limits<int>::max();
	int box_minumum_y = std::numeric_limits<int>::max();

	//get all agents in the box
	std::vector<Point2D>::iterator itr = boundary_segment->bounary_box.begin();
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
	std::vector<Agent const *> all_agents = auraMgr.agentsInRect(lower_left_point, upper_right_point);

	std::vector<Agent const *> agents_after_filter;
	std::vector<Agent const *>::iterator filter_itr = all_agents.begin();
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

void sim_mob::BoundaryProcessor::initBoundaryTrafficItems()
{
	std::map<std::string, BoundarySegment*>::iterator itr;

	for (itr = boundary_segments.begin(); itr != boundary_segments.end(); itr++)
	{
		BoundarySegment* one_segment = itr->second;

		//if the road segment is in the upper side
		if (one_segment->responsible_side == -1)
		{
			const Signal* startNodeSignal = findOneSignalByNode(&one_segment->boundarySegment->getStart()->location);

			if (startNodeSignal)
			{
				int downstream_id = one_segment->connected_partition_id - 1;
				int signal_id = startNodeSignal->getId();
				traffic_items_mapping_to[signal_id] = downstream_id;
				boundaryRealTrafficItems.insert(startNodeSignal);
			}

			const Signal* endNodeSignal = findOneSignalByNode(&one_segment->boundarySegment->getEnd()->location);
			if (endNodeSignal)
			{
				Signal* one_signal = const_cast<Signal*> (endNodeSignal);
				one_signal->isFake = true;
			}
		}
		//down stream
		else if (one_segment->responsible_side == 1)
		{
			const Signal* startNodeSignal = findOneSignalByNode(&one_segment->boundarySegment->getStart()->location);

			if (startNodeSignal)
			{
				if (startNodeSignal->isFake == false)
				{
					Signal* one_signal = const_cast<Signal*> (startNodeSignal);
					one_signal->isFake = true;
				}
			}

			const Signal* endNodeSignal = findOneSignalByNode(&one_segment->boundarySegment->getEnd()->location);
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

std::string sim_mob::BoundaryProcessor::releaseMPIEnvironment()
{
	if (partition_config->partition_size < 1)
		return "";

	MPI_Finalize();

	if (scenario)
	{
		delete scenario;
	}

	if (partition_config)
	{
		delete partition_config;
	}

	std::map<std::string, BoundarySegment*>::iterator it2 = boundary_segments.begin();
	for (; it2 != boundary_segments.end(); it2++)
	{
		if (it2->second)
		{
			delete it2->second;
		}
	}

	return "";
}

void sim_mob::BoundaryProcessor::setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group)
{
	this->entity_group = entity_group;
	this->singal_group = singal_group;
}

void sim_mob::BoundaryProcessor::loadInBoundarySegment(std::string id, BoundarySegment* boundary)
{
	boundary_segments[id] = boundary;

	//update neighbour ids
	neighbor_ips.insert(boundary->connected_partition_id - 1);

	boundary->buildBoundaryBox(partition_config->boundary_length, partition_config->boundary_width);
	boundary->output();
}

void sim_mob::BoundaryProcessor::setConfigure(PartitionConfigure* partition_config, SimulationScenario* scenario)
{
	this ->partition_config = partition_config;
	this ->scenario = scenario;
}

BoundaryProcessor::Agent_Type sim_mob::BoundaryProcessor::getAgentTypeForSerialization(Agent const* agent)
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
