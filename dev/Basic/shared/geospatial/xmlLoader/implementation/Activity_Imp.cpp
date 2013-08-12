#include "geo10-pimpl.hpp"
#include "entities/misc/TripChain.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Activity_t_pimpl::pre ()
{
	model = sim_mob::Activity();
}

sim_mob::TripChainItem* sim_mob::xml::Activity_t_pimpl::post_Activity_t ()
{
	sim_mob::Activity* res = new sim_mob::Activity(model);//here, model only helps as a factory object

	//Retrieve a temporary item, copy over.
	sim_mob::TripChainItem* temp = TripChainItem_t_pimpl::post_TripChainItem_t ();
	res->setPersonID(temp->getPersonID());
	res->itemType = temp->itemType;
	res->sequenceNumber = temp->sequenceNumber;
	res->startTime = temp->startTime;
	res->endTime = temp->endTime;
	delete temp;

	return res;
}

void sim_mob::xml::Activity_t_pimpl::description (const ::std::string& value)
{
	model.description = value;
}

void sim_mob::xml::Activity_t_pimpl::location (unsigned int value)
{
	model.location = book.getNode(value);
}

void sim_mob::xml::Activity_t_pimpl::locationType (std::string value)
{
	model.locationType = sim_mob::TripChainItem::GetLocationTypeXML(value);
}

void sim_mob::xml::Activity_t_pimpl::isPrimary (bool value)
{
	model.isPrimary = value;
}

void sim_mob::xml::Activity_t_pimpl::isFlexible (bool value)
{
	model.isFlexible = value;
}

void sim_mob::xml::Activity_t_pimpl::isMandatory (bool value)
{
	model.isMandatory = value;
}


