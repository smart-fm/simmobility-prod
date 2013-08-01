#include "geo10-pimpl.hpp"
#include "geospatial/streetdir/WayPoint.hpp"
#include "entities/misc/TripChain.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Trip_t_pimpl::pre ()
{
	model = sim_mob::Trip();
}

sim_mob::TripChainItem* sim_mob::xml::Trip_t_pimpl::post_Trip_t ()
{
	//NOTE: I'm not sure if the model would ever be null. It should have been created in pre(), not in tripID(). ~Seth
	//if(!model) { return 0; }
	sim_mob::Trip* res = new sim_mob::Trip(model);//here, model only helps as a factory object

	//Copy over temporary properties.
	sim_mob::TripChainItem* temp = TripChainItem_t_pimpl::post_TripChainItem_t ();
	res->setPersonID(temp->getPersonID());
	res->itemType = temp->itemType;
	res->sequenceNumber = temp->sequenceNumber;
	res->requestTime = temp->requestTime;
	res->startTime = temp->startTime;
	res->endTime = temp->endTime;
	delete temp;

	return res;
}


void sim_mob::xml::Trip_t_pimpl::tripID (long long value)
{
	model.tripID = value;
}

void sim_mob::xml::Trip_t_pimpl::fromLocation (unsigned int value)
{
	model.fromLocation = sim_mob::WayPoint( book.getNode(value) );
}

void sim_mob::xml::Trip_t_pimpl::fromLocationType (std::string value)
{
	model.fromLocationType = sim_mob::TripChainItem::GetLocationTypeXML(value);
}

void sim_mob::xml::Trip_t_pimpl::toLocation (unsigned int value)
{
	model.toLocation = sim_mob::WayPoint(book.getNode(value));
}

void sim_mob::xml::Trip_t_pimpl::toLocationType (std::string value)
{
	model.toLocationType = sim_mob::TripChainItem::GetLocationTypeXML(value);
}

void sim_mob::xml::Trip_t_pimpl::subTrips (std::vector<sim_mob::SubTrip> value)
{
	model.setSubTrips(value);
}

