/*
 * BoundaryProcessor.cpp
 *
 */

#include "GenConfig.h"
#ifndef SIMMOB_DISABLE_MPI

#include "BoundaryProcessor.hpp"

#include <limits>
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "AgentTypeManager.hpp"
#include "PackageUtils.hpp"
#include "UnPackageUtils.hpp"
#include "BoundaryProcessor.hpp"

#include "util/GeomHelpers.hpp"
#include "entities/AuraManager.hpp"
#include "util/OutputUtil.hpp"
#include "conf/simpleconf.hpp"
#include "workers/WorkGroup.hpp"

#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/Role.hpp"

#include "entities/Signal.hpp"
#include "partitions/PartitionManager.hpp"

#include <CGAL/Homogeneous.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>

typedef CGAL::Homogeneous<double> Rep_class;
typedef CGAL::Polygon_2<Rep_class> Polygon_2;
typedef CGAL::Point_2<Rep_class> Point;

namespace mpi = boost::mpi;

namespace sim_mob {

const sim_mob::Signal* findOneSignalByNode(sim_mob::Point2D* point) {
	//std::cout << "sim_mob::Signal::all_signals_.size():" << sim_mob::Signal::all_signals_.size() << std::endl;

	for (size_t i = 0; i < sim_mob::Signal::all_signals_.size(); i++) {
		if (sim_mob::Signal::all_signals_[i]->getNode().location->getX() == point->getX()
				&& sim_mob::Signal::all_signals_[i]->getNode().location->getY() == point->getY()) {
			return sim_mob::Signal::all_signals_[i];
		}
	}

	return 0;
}

std::string BoundaryProcessor::boundaryProcessing(int time_step) {
	//step 1
	clearFakeAgentFlag();
	//std::cout << "Step 1" << std::endl;

	//step 2
	BoundaryProcessingPackage own_package;
	checkBoundaryAgents(own_package);
	//std::cout << "Step 2" << std::endl;

	std::string data = getDataInPackage(own_package);
	//std::cout << "Step 3" << std::endl;

	//Step 3
	mpi::communicator world;
	std::vector<std::string> all_package_data;
	all_gather(world, data, all_package_data);
	//std::cout << "Step 4" << std::endl;

	//Step 4
	processBoundaryPackages(all_package_data);
	//std::cout << "Step 5" << std::endl;

	return "";
}

void BoundaryProcessor::clearFakeAgentFlag() {
	std::vector<Entity*>::iterator itr = Agent::all_agents.begin();
	for (; itr != Agent::all_agents.end();) {
		if (((*itr)->isFake) && ((*itr)->receiveTheFakeEntityAgain == false)) {
			Agent::all_agents.erase(itr);
			releaseFakeAgentMemory(*itr);
		} else {
			itr++;
		}
	}

	itr = Agent::all_agents.begin();
	for (; itr != Agent::all_agents.end(); itr++) {
		if ((*itr)->isFake) {
			(*itr)->receiveTheFakeEntityAgain = false;
		}
	}

	itr = Agent::all_agents.begin();
	for (; itr != Agent::all_agents.end(); itr++) {
		if ((*itr)->isFake && ((*itr)->receiveTheFakeEntityAgain == true)) {
			std::cout << "Error" << std::endl;
			return;
		}
	}
}

std::string BoundaryProcessor::checkBoundaryAgents(BoundaryProcessingPackage& package) {
	std::map<std::string, BoundarySegment*>::iterator loopRoadSegment;
	AgentTypeManager& packageImpl = AgentTypeManager::instance();

	for (loopRoadSegment = boundary_segments.begin(); loopRoadSegment != boundary_segments.end(); ++loopRoadSegment) {
		//std::cout << "1.1.1.1" << std::endl;
		sim_mob::BoundarySegment* segment = loopRoadSegment->second;

		//std::cout << "1.1.1.2.1" << std::endl;
		//get all agents in box
		std::vector<Agent const*> allAgentsInBoundary;
		allAgentsInBoundary = agentsInSegmentBoundary(segment);

		if (segment->responsible_side == -1) {
			std::vector<Agent const*>::iterator agent_pointer;

			for (agent_pointer = allAgentsInBoundary.begin(); agent_pointer != allAgentsInBoundary.end(); agent_pointer++) {
				if ((*agent_pointer)->isFake || (*agent_pointer)->toRemoved) {
					continue;
				}

				if ((packageImpl.getAgentRoleType(*agent_pointer) != role_modes(Driver_Role))
						&& (packageImpl.getAgentRoleType(*agent_pointer) != role_modes(Pedestrian_Role))) {
					continue;
				}

				if (isAgentCrossed(segment, *agent_pointer, false)) {
					const Person *person = dynamic_cast<const Person *> (*agent_pointer);
					if (person) {
						package.cross_persons.push_back(person);

						Person *one_person = const_cast<Person *> (person);
						changeAgentToFake(one_person);
					}
				}

				if (isAgentInFeedbackorForward(segment, *agent_pointer, false)) {
					if ((*agent_pointer)->isFake == false) {
						const Person *person = dynamic_cast<const Person *> (*agent_pointer);
						if (person)
							package.feedback_persons.push_back(person);
					}
				}
			}
		} else {
			std::vector<Agent const*>::iterator agent_pointer;
			for (agent_pointer = allAgentsInBoundary.begin(); agent_pointer != allAgentsInBoundary.end(); ++agent_pointer) {
				if ((*agent_pointer)->isFake || (*agent_pointer)->toRemoved)
					continue;

				if ((packageImpl.getAgentRoleType(*agent_pointer) != role_modes(Driver_Role))
						&& (packageImpl.getAgentRoleType(*agent_pointer) != role_modes(Pedestrian_Role))) {
					continue;
				}

				if (isAgentInFeedbackorForward(segment, *agent_pointer, true)) {
					const Person *person = dynamic_cast<const Person *> (*agent_pointer);
					if (person)
						package.feedback_persons.push_back(person);
				}
			}
		}
	}

	std::set<const Entity*>::iterator itr = boundaryRealTrafficItems.begin();

	for (; itr != boundaryRealTrafficItems.end(); itr++) {
		const Entity* one_entity = (*itr);
		const Signal *one_const_signal = dynamic_cast<const Signal *> (one_entity);
		if (one_const_signal) {
			Signal* one_signal = const_cast<Signal*> (one_const_signal);
			package.boundary_signals.push_back(one_signal);
		}
	}

	return "";
}

std::string BoundaryProcessor::getDataInPackage(BoundaryProcessingPackage& package) {
	sim_mob::PackageUtils packageUtil;
	packageUtil.initializePackage();

//	std::cout << "Step 2.1" << PartitionManager::instance().partition_config->partition_id << std::endl;

	//package cross agents
	packageUtil.packageBasicData(package.cross_persons.size());
//	std::cout << "Step 2.2" << PartitionManager::instance().partition_config->partition_id << std::endl;

	//std::cout << "Out cross_persons:" << package.cross_persons.size() << std::endl;
	AgentTypeManager& packageImpl = AgentTypeManager::instance();

	std::vector<Person const*>::iterator itr_cross = package.cross_persons.begin();
	for (; itr_cross != package.cross_persons.end(); itr_cross++) {

		role_modes type = packageImpl.getAgentRoleType(*itr_cross);
		packageUtil.packageBasicData((int) (type));

		Person* one_person = const_cast<Person*> (*itr_cross);

//		std::cout << "Step 2.4:" << one_person->getId() << PartitionManager::instance().partition_config->partition_id << std::endl;

		one_person->package(packageUtil);
//		std::cout << "Step 2.4.1:" << type << " ," << one_person->getId() << PartitionManager::instance().partition_config->partition_id << std::endl;

		one_person->getRole()->package(packageUtil);
//		std::cout << "Step 2.4.2:" <<  type << " ," << one_person->getId() << PartitionManager::instance().partition_config->partition_id << std::endl;

	}

//	std::cout << "Step 2.5" << PartitionManager::instance().partition_config->partition_id << std::endl;

	//package feedback agents
	packageUtil.packageBasicData(package.feedback_persons.size());

//	std::cout << "Step 2.6" << PartitionManager::instance().partition_config->partition_id << std::endl;

	std::vector<Person const*>::iterator itr_feedback = package.feedback_persons.begin();

	for (; itr_feedback != package.feedback_persons.end(); itr_feedback++) {
		role_modes type = packageImpl.getAgentRoleType(*itr_feedback);
		packageUtil.packageBasicData((int) (type));

		int agent_id = (*itr_feedback)->getId();
		packageUtil.packageBasicData(agent_id);

		Person* one_person = const_cast<Person*> (*itr_feedback);

		one_person->packageProxy(packageUtil);
		one_person->getRole()->packageProxy(packageUtil);
	}

//	std::cout << "Step 2.7" << PartitionManager::instance().partition_config->partition_id << std::endl;

	//package signal
	packageUtil.packageBasicData(package.boundary_signals.size());

	std::vector<Signal const*>::iterator itr_signal = package.boundary_signals.begin();
	for (; itr_signal != package.boundary_signals.end(); itr_signal++) {
		sim_mob::Point2D* location = (*itr_signal)->getNode().location;
		packageUtil.packagePoint2D(location);

		Signal* one_signal = const_cast<Signal*> (*itr_signal);
		one_signal->packageProxy(packageUtil);
	}

//	std::cout << "Step 2.8:" << PartitionManager::instance().partition_config->partition_id << std::endl;
	return (packageUtil.getPackageData());
}

void BoundaryProcessor::processPackageData(std::string data) {
	UnPackageUtils unpackageUtil;
	unpackageUtil.initializePackage(data);

	//std::cout << "Step 4.1" << std::endl;

	//cross agents
	int cross_size = unpackageUtil.unpackageBasicData<int> ();
	//std::cout << "In cross_size:" << cross_size << std::endl;
	for (int i = 0; i < cross_size; i++) {
		int type = unpackageUtil.unpackageBasicData<int> ();

		switch (type) {
		case Driver_Role: {
			Person* one_person = new Person();
			Driver* one_driver = new Driver(one_person, 0, 0, 0);
			one_person->changeRole(one_driver);

			one_person->unpackage(unpackageUtil);
			one_driver->unpackage(unpackageUtil);

			if (isCrossAgentShouldBeInsert(one_person)) {
				one_person->buildSubscriptionList();
				insertOneAgentToWorkerGroup(one_person);
			}
		}

			break;
		case Pedestrian_Role: {
			Person* one_person = new Person();
			Pedestrian* one_pedestrian = new Pedestrian(one_person);
			one_person->changeRole(one_pedestrian);

			one_person->unpackage(unpackageUtil);
			one_pedestrian->unpackage(unpackageUtil);

			if (isCrossAgentShouldBeInsert(one_person)) {
				one_person->buildSubscriptionList();
				insertOneAgentToWorkerGroup(one_person);
			}
		}
			break;
		case 3:
			break;
		}
	}

	//std::cout << "Step 4.2" << std::endl;

	//feedback agents
	int feedback_size = unpackageUtil.unpackageBasicData<int> ();
	//std::cout << "In feedback_size:" << feedback_size << std::endl;

	//std::cout << "Step 4.2.1:" << feedback_size << std::endl;

	for (int i = 0; i < feedback_size; i++) {

		//std::cout << "-------------------------" << std::endl;

		int value = unpackageUtil.unpackageBasicData<int> ();
		//std::cout << "Step 4.2.3:" << std::endl;

		int agent_id = unpackageUtil.unpackageBasicData<int> ();
		//std::cout << "Step 4.2.4:" << std::endl;

		Person* one_person = getFakePersonById(agent_id);

		if (one_person) {
			one_person->unpackageProxy(unpackageUtil);
			//std::cout << "Step 4.2.5:" << std::endl;

			one_person->getRole()->unpackageProxy(unpackageUtil);

			one_person->isFake = true;
			one_person->receiveTheFakeEntityAgain = true;
			one_person->toRemoved = false;

			//std::cout << "Step 4.2.6:" << std::endl;

		} else {
			ConfigParams& config = ConfigParams::GetInstance();

			switch (value) {
			case Driver_Role:
				one_person = new Person(-1);
				one_person->changeRole(new Driver(one_person, config.reacTime_LeadingVehicle,
						config.reacTime_SubjectVehicle, config.reacTime_Gap));

				one_person->unpackageProxy(unpackageUtil);
				//std::cout << "Step 4.2.7:" << std::endl;

				one_person->getRole()->unpackageProxy(unpackageUtil);
				//std::cout << "Step 4.2.8:" << std::endl;

				if (isAgentInLocalPartition(one_person->getId(), false))
					continue;

				one_person->isFake = true;
				one_person->receiveTheFakeEntityAgain = true;
				one_person->toRemoved = false;
				insertOneFakeAgentToWorkerGroup(one_person);
				break;

			case Pedestrian_Role:
				one_person = new Person(-1);
				one_person->changeRole(new Pedestrian(one_person));

				one_person->unpackageProxy(unpackageUtil);
				//std::cout << "Step 4.2.9:" << std::endl;

				one_person->getRole()->unpackageProxy(unpackageUtil);
				//std::cout << "Step 4.2.10:" << std::endl;

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

	//std::cout << "Step 4.3" << std::endl;

	//feedback signal
	int signal_size = unpackageUtil.unpackageBasicData<int> ();

	for (int i = 0; i < signal_size; i++) {
		sim_mob::Point2D* location = unpackageUtil.unpackagePoint2D();
		sim_mob::Signal* one_signal = const_cast<Signal*> (sim_mob::getSignalBasedOnNode(location));

		one_signal->unpackageProxy(unpackageUtil);
	}

	//std::cout << "Step 4.4" << std::endl;
}

std::string BoundaryProcessor::processBoundaryPackages(std::vector<std::string>& all_packages) {
	int index = -1;
	std::vector<std::string>::iterator it = all_packages.begin();

	for (; it != all_packages.end(); it++) {
		index++;
		if (index == partition_config->partition_id) {
		} else {
			std::string one_package_data = (*it);
			processPackageData(one_package_data);
		}
	}

	return "";
}

Person* BoundaryProcessor::getFakePersonById(unsigned int agent_id) {
	std::vector<Entity*>::iterator itr = Agent::all_agents.begin();

	for (; itr != Agent::all_agents.end(); itr++) {
		if ((*itr)->isFake && (*itr)->getId() == agent_id) {
			Person* person = dynamic_cast<Person *> (*itr);
			if (person)
				return person;
		}
	}

	return NULL;
}

bool BoundaryProcessor::isAgentCrossed(BoundarySegment* segment, Agent const* agent, bool is_down_boundary) {
	if (agent->isFake)
		return false;

	//downside
	if (is_down_boundary)
		return false;

	sim_mob::AgentTypeManager& packageImpl = sim_mob::AgentTypeManager::instance();

	if (packageImpl.getAgentRoleType(agent) == role_modes(Driver_Role)) {
		const Person *person = dynamic_cast<const Person *> (agent);
		Person* p = const_cast<Person*> (person);
		const Driver *driver = dynamic_cast<const Driver *> (p->getRole());

		if (driver->getVehicle()->getCurrSegment()->getStart()->location->getX()
				!= segment->boundarySegment->getStart()->location->getX()) {
			return false;
		}

		if (driver->getVehicle()->getCurrSegment()->getStart()->location->getY()
				!= segment->boundarySegment->getStart()->location->getY()) {
			return false;
		}
		if (driver->getVehicle()->getCurrSegment()->getEnd()->location->getX()
				!= segment->boundarySegment->getEnd()->location->getX()) {
			return false;
		}
		if (driver->getVehicle()->getCurrSegment()->getEnd()->location->getY()
				!= segment->boundarySegment->getEnd()->location->getY()) {
			return false;
		}

		int x_cut_dis = segment->cut_line_to->getX() - segment->cut_line_start->getX();
		int y_cut_dis = segment->cut_line_to->getY() - segment->cut_line_start->getY();

		int x_pos = person->xPos.get();
		int y_pos = person->yPos.get();

		int pos_line = (x_pos - segment->cut_line_start->getX()) * y_cut_dis
				- (y_pos - segment->cut_line_start->getY()) * x_cut_dis;
		if (pos_line == 0)
			return false;

		int upper_x_pos = segment->boundarySegment->getStart()->location->getX();
		int upper_y_pos = segment->boundarySegment->getStart()->location->getY();

		int upper_pos_line = (upper_x_pos - segment->cut_line_start->getX()) * y_cut_dis - (upper_y_pos
				- segment->cut_line_start->getY()) * x_cut_dis;

		if (upper_pos_line > 0 && pos_line > 0)
			return false;

		if (upper_pos_line < 0 && pos_line < 0)
			return false;

		return true;

	} else if (packageImpl.getAgentRoleType(agent) == role_modes(Pedestrian_Role)) {
		const Person *person = dynamic_cast<const Person *> (agent);

		//calcuate the relationship between point and line
		int y_cut_dis = segment->cut_line_to->getY() - segment->cut_line_start->getY();
		int x_cut_dis = segment->cut_line_to->getX() - segment->cut_line_start->getX();

		int x_pos = person->xPos.get();
		int y_pos = person->yPos.get();

		int pos_line = (x_pos - segment->cut_line_start->getX()) * y_cut_dis
				- (y_pos - segment->cut_line_start->getY()) * x_cut_dis;
		if (pos_line == 0)
			return false;

		int upper_x_pos = segment->boundarySegment->getStart()->location->getX();
		int upper_y_pos = segment->boundarySegment->getStart()->location->getY();

		int upper_pos_line = (upper_x_pos - segment->cut_line_start->getX()) * y_cut_dis - (upper_y_pos
				- segment->cut_line_start->getY()) * x_cut_dis;

		if (upper_pos_line > 0 && pos_line > 0)
			return false;

		if (upper_pos_line < 0 && pos_line < 0)
			return false;

		return true;
	}

	return false;
}

bool BoundaryProcessor::isAgentInFeedbackorForward(BoundarySegment* segment, Agent const* agent, bool is_down_boundary) {
	if (agent->isFake)
		return false; //Error, should not happen;

	//	//downside
	//	if (is_down_boundary)
	//		return true;

	return true;
}

void BoundaryProcessor::changeAgentToFake(Agent * agent) {
	agent->isFake = true;
	agent->receiveTheFakeEntityAgain = true;
	agent->toRemoved = false;

	entity_group->removeAgentFromWorker(agent);
}

void BoundaryProcessor::insertOneAgentToWorkerGroup(Agent * agent) {
	agent->isFake = false;
	agent->receiveTheFakeEntityAgain = false;
	agent->toRemoved = false;

	Agent::all_agents.push_back(agent);
	entity_group->addAgentInWorker(agent);
}

void BoundaryProcessor::insertOneFakeAgentToWorkerGroup(Agent * agent) {
	agent->isFake = true;
	agent->receiveTheFakeEntityAgain = true;
	agent->toRemoved = false;

	Agent::all_agents.push_back(agent);
}

void BoundaryProcessor::removeOneFakeAgentFromWorkerGroup(Agent * agent) {
	std::vector<Entity*>::iterator position = std::find(Agent::all_agents.begin(), Agent::all_agents.end(), agent);

	if (position != Agent::all_agents.end()) {
		Agent::all_agents.erase(position);
	}
}

bool BoundaryProcessor::isAgentInLocalPartition(unsigned int agent_id, bool includeFakeAgent) {

	std::vector<Entity*>::iterator it = Agent::all_agents.begin();
	for (; it != Agent::all_agents.end(); it++) {
		if (includeFakeAgent) {
			if ((*it)->getId() == agent_id)
				return true;
		} else {
			if ((*it)->getId() == agent_id && (*it)->isFake == false)
				return true;
		}
	}

	return false;
}

bool BoundaryProcessor::isCrossAgentShouldBeInsert(const sim_mob::Agent* agent) {
	if (isAgentInLocalPartition(agent->getId(), false)) {
		return false;
	}

	return true;
}

sim_mob::BoundarySegment* BoundaryProcessor::getBoundarySegmentByID(std::string segmentID) {
	std::map<std::string, BoundarySegment*>::iterator it;
	it = boundary_segments.find(segmentID);

	if (it != boundary_segments.end())
		return it->second;

	return 0;
}

std::string BoundaryProcessor::outputAllEntities(frame_t time_step) {

	std::vector<Entity*>::iterator it = Agent::all_agents.begin();
	for (; it != Agent::all_agents.end(); it++) {

		Agent* one_agent = dynamic_cast<Agent*> (*it);
		if (one_agent->isToBeRemoved() == false)
			one_agent ->output(time_step);
	}

	//signal output every 10 seconds
	if (time_step % ConfigParams::GetInstance().granSignalsTicks != 0) {
		return "";
	}

	std::vector<Signal*>::iterator itr_sig = Signal::all_signals_.begin();
	for (; itr_sig != Signal::all_signals_.end(); itr_sig++) {
		Signal* one_signal = (*itr_sig);
		one_signal->output(time_step);
	}

	return "";
}

//very simply version
//need to build virtual destructors
void BoundaryProcessor::releaseFakeAgentMemory(sim_mob::Entity* agent) {
	//	if (agent != NULL) {
	//		delete agent;
	//	}
}

bool isOneagentInPolygon(int location_x, int location_y, sim_mob::BoundarySegment* boundary_segment) {
	Point agent_location(location_x, location_y);
	std::vector<Point> points;

	std::vector<Point2D>::iterator itr = boundary_segment->bounary_box.begin();
	for (; itr != boundary_segment->bounary_box.end(); itr++) {
		Point point((*itr).getX(), (*itr).getY());
		points.push_back(point);
	}

	switch (CGAL::bounded_side_2(points.begin(), points.end(), agent_location)) {
	case CGAL::ON_BOUNDED_SIDE:
		//std::cout << "location_x:" << location_x << ", location_y:" << location_y << "Inside" << std::endl;
		return true;
	case CGAL::ON_BOUNDARY:
		return false;
	case CGAL::ON_UNBOUNDED_SIDE:
		//std::cout << "location_x:" << location_x << ", location_y:" << location_y << "Outside" << std::endl;
		return false;
	}

	return false;
}

void outputLineT(Point2D& start_p, Point2D& end_p, std::string color) {
	static int line_id = 100;
	if (line_id < 105)
		LogOut("(" << "\"CutLine\"," << "0," << line_id++ << "," << "{\"startPointX\":\"" << start_p.getX() << "\","
				<< "\"startPointY\":\"" << start_p.getY() << "\"," << "\"endPointX\":\"" << end_p.getX() << "\","
				<< "\"endPointY\":\"" << end_p.getY() << "\"," << "\"color\":\"" << color << "\"" << "})" << std::endl);
}

//need to be changed to polygon2D
std::vector<Agent const *> BoundaryProcessor::agentsInSegmentBoundary(sim_mob::BoundarySegment* boundary_segment) {

	//	//get the boundary box
	int box_maximum_x = 0;
	int box_maximum_y = 0;
	int box_minumum_x = std::numeric_limits<int>::max();
	int box_minumum_y = std::numeric_limits<int>::max();

	//get all agents in the box
	std::vector<Point2D>::iterator itr = boundary_segment->bounary_box.begin();
	for (; itr != boundary_segment->bounary_box.end(); itr++) {
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
	for (; filter_itr != all_agents.end(); filter_itr++) {
		if ((*filter_itr)->isFake)
			continue;

		if (isOneagentInPolygon((*filter_itr)->xPos.get(), (*filter_itr)->yPos.get(), boundary_segment)) {
			agents_after_filter.push_back(*filter_itr);
		}
	}

	return agents_after_filter;
}

void BoundaryProcessor::initBoundaryTrafficItems() {
	std::map<std::string, BoundarySegment*>::iterator itr;

	for (itr = boundary_segments.begin(); itr != boundary_segments.end(); itr++) {
		BoundarySegment* one_segment = itr->second;

		if (one_segment->responsible_side == -1) {
			const sim_mob::Signal* startNodeSignal = findOneSignalByNode(
					one_segment->boundarySegment->getStart()->location);

			if (startNodeSignal) {
				boundaryRealTrafficItems.insert(startNodeSignal);
			}

			const sim_mob::Signal* endNodeSignal =
					findOneSignalByNode(one_segment->boundarySegment->getEnd()->location);
			if (endNodeSignal) {
				sim_mob::Signal* one_signal = const_cast<sim_mob::Signal*> (endNodeSignal);
				one_signal->isFake = true;
			}
		} else if (one_segment->responsible_side == 1) //down stream

		{
			const sim_mob::Signal* startNodeSignal = findOneSignalByNode(
					one_segment->boundarySegment->getStart()->location);

			if (startNodeSignal) {
				if (startNodeSignal->isFake == false) {
					sim_mob::Signal* one_signal = const_cast<sim_mob::Signal*> (startNodeSignal);
					one_signal->isFake = true;
				}
			}

			const sim_mob::Signal* endNodeSignal =
					findOneSignalByNode(one_segment->boundarySegment->getEnd()->location);
			if (endNodeSignal) {
				boundaryRealTrafficItems.insert(endNodeSignal);
			}
		}
	}
}

std::string BoundaryProcessor::releaseMPIEnvironment() {
	if (partition_config->partition_size < 1)
		return "";

	MPI_Finalize();

	if (scenario) {
		delete scenario;
	}

	if (partition_config) {
		delete partition_config;
	}

	std::map<std::string, BoundarySegment*>::iterator it2 = boundary_segments.begin();
	for (; it2 != boundary_segments.end(); it2++) {
		if (it2->second) {
			delete it2->second;
		}
	}

	return "";
}

void BoundaryProcessor::setEntityWorkGroup(WorkGroup* entity_group, WorkGroup* singal_group) {
	this->entity_group = entity_group;
	this->singal_group = singal_group;
}

void BoundaryProcessor::loadInBoundarySegment(std::string id, BoundarySegment* boundary) {
	boundary_segments[id] = boundary;

	boundary->buildBoundaryBox(partition_config->boundary_length, partition_config->boundary_width);
	boundary->output();
}

void BoundaryProcessor::setConfigure(PartitionConfigure* partition_config, SimulationScenario* scenario) {
	this ->partition_config = partition_config;
	this ->scenario = scenario;
}

}

#endif
