//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "DriverUpdateParams.hpp"
#include "Biker.hpp"

/*
 * BikerFacets.hpp
 *
 */

namespace sim_mob {
namespace medium
{
class Biker;

/**
 * Behavior facet of Biker role
 * \author Harish Loganathan
 */
class BikerBehavior: public DriverBehavior {
public:
	explicit BikerBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~BikerBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	sim_mob::medium::Biker* getParentBiker() const {
		return parentBiker;
	}

	void setParentBiker(sim_mob::medium::Biker* parentBiker) {
		if(!parentBiker) { throw std::runtime_error("parentBiker cannot be NULL"); }
		this->parentBiker = parentBiker;
	}

protected:
	sim_mob::medium::Biker* parentBiker;
};

/**
 * Movement facet of Biker role
 * \author Harish Loganathan
 */
class BikerMovement: public DriverMovement {
public:
	explicit BikerMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~BikerMovement();

	//Virtual overrides
	virtual void frame_init();

	sim_mob::medium::Biker* getParentBiker() const {
		return parentBiker;
	}

	void setParentBiker(sim_mob::medium::Biker* parentBiker) {
		if(!parentBiker) {
			throw std::runtime_error("parentBiker cannot be NULL");
		}
		this->parentBiker = parentBiker;
	}

protected:
	/**pointer to parent bus driver*/
	sim_mob::medium::Biker* parentBiker;
};

}
}

