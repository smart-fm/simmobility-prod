//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <unordered_map>
#include "entities/Agent.hpp"
#include "geospatial/network/SMSVehicleParking.hpp"
#include "Person_MT.hpp"

namespace sim_mob
{
namespace medium
{

class ParkingAgent : public Agent
{
private:
    /**The SMS vehicle parking object*/
    const SMSVehicleParking *smsVehicleParking;

    /**Static map of all parking agents*/
    static std::unordered_map<const SMSVehicleParking *, ParkingAgent *> mapOfParkingAgents;

    /**List of persons parked at the parking and being managed by this agent*/
    std::list<Person_MT *> parkedPersons;

    /**The conflux that manages this parking agent*/
    Conflux *parentConflux;

    /**
     * Sets the parent conflux based on the node the parking is located at
     */
    void setParentConflux();

protected:
    //Virtual overrides
    virtual Entity::UpdateStatus frame_init(timeslice now);

    virtual Entity::UpdateStatus frame_tick(timeslice now);

    virtual void frame_output(timeslice now)
    {
    }

    virtual bool isNonspatial()
    {
        return false;
    }

public:
    ParkingAgent(const MutexStrategy& mtxStrat, int id, const SMSVehicleParking *parking);
    virtual ~ParkingAgent();

    /**
     * Registers the parking into a static map along with the parking agent for lookup
     * @param pkAgent
     */
    static void registerParkingAgent(ParkingAgent *pkAgent);

    /**
     * Finds the parking agent corresponding to the vehicle parking object
     * @param parking the vehicle parking object
     * @return the parking agent managing the given vehicle parking object
     */
    static ParkingAgent* getParkingAgent(const SMSVehicleParking *parking);

    void addParkedPerson(Person_MT *person);

    const SMSVehicleParking* getSMSParking()
    {
        return smsVehicleParking;
    }

    Conflux* getParentConflux()
    {
        return parentConflux;
    }
};

}
}