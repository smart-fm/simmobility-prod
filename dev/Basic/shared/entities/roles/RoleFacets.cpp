//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoleFacets.hpp"

#include "entities/Person.hpp"
#include "workers/Worker.hpp"

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
