#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::SubTrip_t_pimpl::pre ()
{
	model = sim_mob::SubTrip();
}

sim_mob::SubTrip sim_mob::xml::SubTrip_t_pimpl::post_SubTrip_t ()
{
	//Copy over temporary properties
	sim_mob::TripChainItem* tempParent = Trip_t_pimpl::post_Trip_t ();
	sim_mob::Trip* temp = dynamic_cast<sim_mob::Trip*>(tempParent);
	if (!temp) {
		throw std::runtime_error("Unexpected non-trip type.");
	}

	model.setPersonID(temp->getPersonID());

	//TODO:the following three items( which are inherited from trip)
	//may need to be initialized differently
	//for now, I keet the same value as their parent(trip)
	//and we will modify them as and when needed-vahid
	model.startTime = temp->startTime;
	model.endTime = temp->endTime;
	model.sequenceNumber = temp->sequenceNumber;
	model.requestTime = temp->requestTime;
	////////////////////////////////////////////////

	model.tripID = temp->tripID;
	model.fromLocation = temp->fromLocation;
	model.toLocation = temp->toLocation;
	model.fromLocationType = temp->fromLocationType;
	model.toLocationType = temp->toLocationType;
	delete tempParent;

	return model;
}


void sim_mob::xml::SubTrip_t_pimpl::mode (const ::std::string& value)
{
	model.mode = value;
}

void sim_mob::xml::SubTrip_t_pimpl::isPrimaryMode (bool value)
{
	model.isPrimaryMode = value;
}

void sim_mob::xml::SubTrip_t_pimpl::ptLineId (const ::std::string& value)
{
	model.ptLineId = value;
}

