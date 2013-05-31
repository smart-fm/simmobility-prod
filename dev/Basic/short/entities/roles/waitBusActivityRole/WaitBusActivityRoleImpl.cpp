#include "WaitBusActivityRoleImpl.hpp"

#include "entities/Person.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/roles/passenger/Passenger.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::WaitBusActivityRoleImpl::WaitBusActivityRoleImpl(Agent* parent) :
		WaitBusActivityRole(parent)
{
}

sim_mob::WaitBusActivityRoleImpl::~WaitBusActivityRoleImpl() {

}

Role* sim_mob::WaitBusActivityRoleImpl::clone(Person* parent) const
{
	return new WaitBusActivityRoleImpl(parent);
}

void sim_mob::WaitBusActivityRoleImpl::frame_init(UpdateParams& p) {
	if(parent->destNode.type_== WayPoint::BUS_STOP) { // to here waiting(busstop)
		busStopAgent = parent->destNode.busStop_->generatedBusStopAgent;
	} else {
		sim_mob::BusStop* busStop_dest = setBusStopXY(parent->destNode.node_);// to here waiting(node)
		busStopAgent = busStop_dest->generatedBusStopAgent;// assign the BusStopAgent to WaitBusActivityRole
		parent->xPos.set(busStop_dest->xPos);// set xPos to WaitBusActivityRole
		parent->yPos.set(busStop_dest->yPos);// set yPos to WaitBusActivityRole
	}
	TimeOfReachingBusStop = p.now.ms();
	buslineid = "7_2";// set Busline information(hardcoded now, later from Public Transit Route Choice to choose the busline)
}

void sim_mob::WaitBusActivityRoleImpl::frame_tick(UpdateParams& p) {
	if(0!=boarding_MS) {// if boarding_Frame is already set
		if(boarding_MS == p.now.ms()) {// if currFrame is equal to the boarding_Frame
			parent->setToBeRemoved();
			boarding_MS = 0;
			Person* person = dynamic_cast<Person*> (parent);
			if(person) {
				if(person->getNextRole()) {
					Passenger* passenger = dynamic_cast<Passenger*> (person->getNextRole());// check whether nextRole is passenger Role or not
					if(passenger) {
						//BusDriver* driver = dynamic_cast<BusDriver*>(busDriver);
						passenger->busdriver.set(busDriver);// assign this busdriver to Passenger
						passenger->BoardedBus.set(true);
						passenger->AlightedBus.set(false);
					}
				}
			}
		}
	}
}

void sim_mob::WaitBusActivityRoleImpl::frame_tick_output(const UpdateParams& p) {
	if (ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	//Reset our offset if it's set to zero
	if (DisplayOffset.getX()==0 && DisplayOffset.getY()==0) {
	   boost::mt19937 gen(static_cast<unsigned int>(parent->getId()*parent->getId()));
	   boost::uniform_int<> distX(0, 249);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varX(gen, distX);
	   boost::uniform_int<> distY(0, 99);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varY(gen, distY);
	   unsigned int value = (unsigned int)varX();
	   DisplayOffset.setX(value+1);
	   value= (unsigned int)varY();
	   DisplayOffset.setY(value+1);
	}
	//LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(parent->xPos.get()+DisplayOffset.getX()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(parent->yPos.get()+DisplayOffset.getY()+DisplayOffset.getY())<<"\",})"<<std::endl);
	LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(parent->xPos.get()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(parent->yPos.get()+DisplayOffset.getY())<<"\",})"<<std::endl);
}

void sim_mob::WaitBusActivityRoleImpl::frame_tick_output_mpi(timeslice now) {

}

UpdateParams& sim_mob::WaitBusActivityRoleImpl::make_frame_tick_params(timeslice now) {
	params.reset(now);
	return params;
}
