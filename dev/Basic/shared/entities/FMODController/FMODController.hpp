/*
 * FMODController.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef FMODCONTROLLER_HPP_
#define FMODCONTROLLER_HPP_
#include "TCPSession.hpp"
#include "ParkingCoordinator.h"
#include "entities/roles/Role.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

namespace FMOD
{

class FMODController : public sim_mob::Agent {
public:
	explicit FMODController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id), connectPoint(new TCPSession(io_service)), frameTicks(0){}

	virtual ~FMODController();

	bool InsertFMODItems(const std::string& personID, TripChainItem* item);
	void Settings(std::string ipadress, int port, int updateTiming, std::string mapFile) { this->ipAddress=ipadress; this->port=port; this->updateTiming=updateTiming; this->mapFile=mapFile;}

	static void RegisterController(int id, const MutexStrategy& mtxStrat);
	static FMODController* Instance();
	bool StartClientService();
	void StopClientService();
	void Initialize();

private:
	std::map<Link*, double> linkTravelTimes;
	std::map<std::string, TripChainItem*> all_items;
	std::map<Request*, TripChainItem*> all_requests;
	std::vector<Agent*> all_persons;
	std::vector<Agent*> all_drivers;
	// keep all children agents to communicate with it
	std::vector<Agent*> all_children;

protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

	//May implement later
	virtual void load(const std::map<std::string, std::string>& configProps){}
	//Signals are non-spatial in nature.
	virtual bool isNonspatial() { return true; }
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList){}
	virtual void unregisteredChild(Entity* child);

private:
	void ProcessMessages(timeslice now);
	MessageList GenerateRequest(timeslice now);
	MessageList HandleOfferMessage(std::string msg);
	MessageList HandleConfirmMessage(std::string msg);
	void HandleScheduleMessage(std::string msg);
	void HandleVehicleInit(std::string msg);

	void UpdateMessages(timeslice now);
	MessageList CollectVehStops();
	MessageList CollectVehPos();
	MessageList CollectLinkTravelTime();

	void DispatchActivityAgents(timeslice now);

private:
	void CollectPerson();
	void CollectRequest();

private:
	TCPSessionPtr connectPoint;
	std::string ipAddress;
	std::string mapFile;
	int port;
	int updateTiming;
	int frameTicks;

private:
	struct travelTimes
	{
	public:
		unsigned int linkTravelTime_;
		unsigned int agentCount_;

		travelTimes(unsigned int linkTravelTime, unsigned int agentCount)
		: linkTravelTime_(linkTravelTime), agentCount_(agentCount) {}
	};

	std::map<const Link*, travelTimes> LinkTravelTimesMap;

	//when vehicle initialize and pack, it will store to this structure
	ParkingCoordinator parkingCoord;

private:
	static FMODController* pInstance;
	static boost::asio::io_service io_service;

};

}

} /* namespace sim_mob */
#endif /* FMODCONTROLLER_HPP_ */
