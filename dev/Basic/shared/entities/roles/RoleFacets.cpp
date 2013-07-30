#include "RoleFacets.hpp"

#include "entities/Person.hpp"

sim_mob::NullableOutputStream sim_mob::BehaviorFacet::Log()
{
	return NullableOutputStream(parentAgent->currWorker->getLogFile());
}

sim_mob::NullableOutputStream sim_mob::MovementFacet::Log()
{
	return NullableOutputStream(parentAgent->currWorker->getLogFile());
}

