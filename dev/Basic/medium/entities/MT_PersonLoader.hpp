//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include "entities/PersonLoader.hpp"
#include "entities/misc/TripChain.hpp"

namespace sim_mob
{
namespace medium
{
/**
 * sub class of PersonLoader tailored for loading mid-term persons from day activity schedule
 *
 * \author Harish Loganathan
 */
class MT_PersonLoader : public PeriodicPersonLoader
{
public:
	MT_PersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents);
	virtual ~MT_PersonLoader();

	/**
	 * load activity schedules for next interval
	 */
	virtual void loadPersonDemand();

private:
	/**
	 * makes a single sub trip for trip (for now)
	 * @param r row from database table
	 * @param parentTrip parent Trip for the subtrip to be constructed
	 * @param subTripNo the sub trip number
	 */
	static void makeSubTrip(const soci::row& r, Trip* parentTrip, unsigned short subTripNo=1);

	/**
	 * makes an activity
	 * @param r row from database table
	 * @param seqNo tripchain item sequence number
	 * @return the activity constructed from the supplied row
	 */
	static Activity* makeActivity(const soci::row& r, unsigned int seqNo);

	/**
	 * makes a trip
	 * @param r row from database table
	 * @param seqNo tripchain item sequence number
	 * @return the trip constructed from the supplied row
	 */
	static Trip* makeTrip(const soci::row& r, unsigned int seqNo);
};

} // namespace medium
} // namespace sim_mob
