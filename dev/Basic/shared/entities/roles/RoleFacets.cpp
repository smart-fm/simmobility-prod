#include "RoleFacets.hpp"

#include "entities/Person.hpp"

sim_mob::NullableOutputStream sim_mob::BehaviorFacet::Log()
{
	return NullableOutputStream(parentAgent->currWorkerProvider->getLogFile());
}

sim_mob::NullableOutputStream sim_mob::MovementFacet::Log()
{
	return NullableOutputStream(parentAgent->currWorkerProvider->getLogFile());
}

