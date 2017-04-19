/*
 * GreedyTaxiController_MT.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#include "GreedyTaxiController_MT.hpp"

#include "entities/roles/driver/TaxiDriver.hpp"

namespace sim_mob
{
bool GreedyTaxiController_MT::isCruising(Person* p)
{
	medium::Person_MT* person_MT = dynamic_cast<medium::Person_MT*>(p);

	if (person_MT)
	{
		if (person_MT->getRole())
		{
			medium::TaxiDriver* currDriver = dynamic_cast<medium::TaxiDriver*>(person_MT->getRole());
			if (currDriver)
			{
				if (currDriver->getDriverMode() == medium::CRUISE)
				{
					return true;
				}
			}
		}
	}

	return false;
}

const Node* GreedyTaxiController_MT::getCurrentNode(Person* p)
{
	medium::Person_MT* person_MT = dynamic_cast<medium::Person_MT*>(p);

	if (person_MT)
	{
		if (person_MT->getRole())
		{
			medium::TaxiDriver* currDriver = dynamic_cast<medium::TaxiDriver*>(person_MT->getRole());
			if (currDriver)
			{
				return currDriver->getMovementFacet()->getCurrentNode();
			}
		}
	}

	return nullptr;
}
}



