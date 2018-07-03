//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverVariantFacets.hpp"
#include "entities/Person_MT.hpp"
#include "Driver.hpp"

namespace sim_mob
{
namespace medium
{

class BikerBehavior;
class BikerMovement;

/**
 * Mid-term motor-cyclist
 * \author Harish Loganathan
 */
class Biker: public medium::Driver
{
public:
	Biker(Person_MT* parent, medium::BikerBehavior* behavior = nullptr, medium::BikerMovement* movement = nullptr, std::string roleName =
			std::string(), Role<Person_MT>::Type roleType = RL_BIKER);
	virtual ~Biker();

	//Virtual overrides
	virtual Role<Person_MT>* clone(Person_MT* parent) const;
};

class TruckerBehavior;
class TruckerMovement;

/**
 * Mid-term freight driver
 * \author Harish Loganathan
 */
class TruckerLGV: public medium::Driver
{
public:
	TruckerLGV(Person_MT* parent, medium::TruckerBehavior* behavior = nullptr, medium::TruckerMovement* movement = nullptr, std::string roleName =
			std::string(), Role<Person_MT>::Type roleType = RL_TRUCKER_LGV);
	virtual ~TruckerLGV();

	//Virtual overrides
	virtual Role<Person_MT>* clone(Person_MT* parent) const;
};

/**
 * Mid-term freight driver
 * \author Harish Loganathan
 */
class TruckerHGV: public medium::Driver
{
public:
	TruckerHGV(Person_MT* parent, medium::TruckerBehavior* behavior = nullptr, medium::TruckerMovement* movement = nullptr, std::string roleName =
			std::string(), Role<Person_MT>::Type roleType = RL_TRUCKER_HGV);
	virtual ~TruckerHGV();

	//Virtual overrides
	virtual Role<Person_MT>* clone(Person_MT* parent) const;
};
} //end namespace medium
} //end namespace sim_mob

