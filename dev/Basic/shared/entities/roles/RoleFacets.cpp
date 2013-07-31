#include "RoleFacets.hpp"

#include "entities/Person.hpp"

sim_mob::NullableOutputStream sim_mob::Facet::Log()
{
	return NullableOutputStream(parent->currWorkerProvider->getLogFile());
}

sim_mob::Person* sim_mob::Facet::getParent()
{
	return parent;
}

void sim_mob::Facet::setParent(sim_mob::Person* parent)
{
	this->parent = parent;
}
