#pragma once

#include <map>
#include <string>
#include <vector>

#include "conf/settings/DisableMPI.h"

#include "entities/Agent.hpp"
#include "entities/roles/Role.hpp"
#include "event/EventPublisher.hpp"
#include "event/GenericEventPublisher.hpp"

namespace sim_mob {

class UpdateParams;
#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

namespace long_term {

/**
 * TestAgent class.
 *
 * \author gandola
 */
class TestAgent: public sim_mob::Agent, public sim_mob::GenericEventPublisher {
public:
	explicit TestAgent(Role* newRole, const MutexStrategy& mtxStrat, int id = -1);
	virtual ~TestAgent();

	/// requested.
	virtual void load(const std::map<std::string, std::string>& configProps);

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);
	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

	void SetRole(Role* newRole);
	//sim_mob::Role* getRole() const;

protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

private:
	Role* currRole;
	UpdateParams* params;
};

}
}
