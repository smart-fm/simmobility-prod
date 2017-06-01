/*
 * VehicleOwnershipCoefficients.cpp
 *
 *  Created on: Feb 24, 2015
 *      Author: gishara
 */
#include "VehicleOwnershipCoefficients.hpp"

using namespace sim_mob::long_term;

VehicleOwnershipCoefficients::VehicleOwnershipCoefficients(BigSerial vehicleOwnershipOptionId, double incomeAdj, double malay, double indian, double otherRaces, double aboveSixty,  double privateProperty, double whiteCollar, double worker,
		 	 	 	 	 	 	 	 	 	 	 	 	   double HHChild1, double HHChild2Plus, double taxi, double mrt500m , double mrt1000m, double logsum, double constant):
														   vehicleOwnershipOptionId(vehicleOwnershipOptionId),incomeAdj(incomeAdj), malay(malay),indian(indian), otherRaces(otherRaces),aboveSixty(aboveSixty),privateProperty(privateProperty),whiteCollar(whiteCollar),worker(worker),
														   HHChild1(HHChild1),HHChild2Plus(HHChild2Plus), taxi(taxi), mrt500m(mrt500m), mrt1000m(mrt1000m), logsum(logsum), constant(constant){}

VehicleOwnershipCoefficients::~VehicleOwnershipCoefficients() {}

VehicleOwnershipCoefficients& VehicleOwnershipCoefficients::operator=(const VehicleOwnershipCoefficients& source)
{
	this->vehicleOwnershipOptionId 		= source.vehicleOwnershipOptionId;
	this->incomeAdj					    = source.incomeAdj;
	this->malay 						= source.malay;
	this->indian 						= source.indian;
	this->otherRaces 					= source.otherRaces;
	this->aboveSixty					= source.aboveSixty;
	this->privateProperty 				= source.privateProperty;
	this->whiteCollar 					= source.whiteCollar;
	this->worker 						= source.worker;
	this->HHChild1 						= source.HHChild1;
	this->HHChild2Plus 					= source.HHChild2Plus;
	this->taxi 							= source.taxi;
	this->mrt500m 						= source.mrt500m;
	this->mrt1000m 						= source.mrt1000m;
	this->logsum 						= source.logsum;
	this->constant 						= source.constant;

    return *this;
}

double VehicleOwnershipCoefficients::getConstant() const
{
	return constant;
}

void VehicleOwnershipCoefficients::setConstant(double constant)
{
	this->constant = constant;
}

double VehicleOwnershipCoefficients::getHhChild1() const
{
	return HHChild1;
}

void VehicleOwnershipCoefficients::setHhChild1(double hhChild1)
{
		HHChild1 = hhChild1;
}

double VehicleOwnershipCoefficients::getHhChild2Plus() const
{
	return HHChild2Plus;
}

void VehicleOwnershipCoefficients::setHhChild2Plus(double hhChild2Plus)
{
	HHChild2Plus = hhChild2Plus;
}

double VehicleOwnershipCoefficients::getIndian() const
{
	return indian;
}

void VehicleOwnershipCoefficients::setIndian(double indian)
{
	this->indian = indian;
}

double VehicleOwnershipCoefficients::getLogsum() const
{
	return logsum;
}

void VehicleOwnershipCoefficients::setLogsum(double logsum)
{
	this->logsum = logsum;
}

double VehicleOwnershipCoefficients::getMalay() const
{
	return malay;
}

void VehicleOwnershipCoefficients::setMalay(double malay)
{
	this->malay = malay;
}

double VehicleOwnershipCoefficients::getMrt1000m() const
{
	return mrt1000m;
}

void VehicleOwnershipCoefficients::setMrt1000m(double mrt1000m)
{
	this->mrt1000m = mrt1000m;
}

double VehicleOwnershipCoefficients::getMrt500m() const
{
	return mrt500m;
}

void VehicleOwnershipCoefficients::setMrt500m(double mrt500m)
{
	this->mrt500m = mrt500m;
}

double VehicleOwnershipCoefficients::getOtherRaces() const
{
	return otherRaces;
}

void VehicleOwnershipCoefficients::setOtherRaces(double other_races)
{
	otherRaces = other_races;
}

double VehicleOwnershipCoefficients::getPrivateProperty() const
{
	return privateProperty;
}

void VehicleOwnershipCoefficients::setPrivateProperty(double privateProperty)
{
	this->privateProperty = privateProperty;
}

double VehicleOwnershipCoefficients::getTaxi() const
{
	return taxi;
}

void VehicleOwnershipCoefficients::setTaxi(double taxi)
{
	this->taxi = taxi;
}

BigSerial VehicleOwnershipCoefficients::getVehicleOwnershipOptionId() const
{
	return vehicleOwnershipOptionId;
}

void VehicleOwnershipCoefficients::setVehicleOwnershipOptionId(BigSerial vehicleOwnershipOptionId)
{
	this->vehicleOwnershipOptionId = vehicleOwnershipOptionId;
}

double VehicleOwnershipCoefficients::getWhiteCollar() const
{
	return whiteCollar;
}

void VehicleOwnershipCoefficients::setWhiteCollar(double white_collar)
{
	whiteCollar = white_collar;
}

double VehicleOwnershipCoefficients::getWorker() const
{
	return worker;
}

void VehicleOwnershipCoefficients::setWorker(double worker)
{
	this->worker = worker;
}

double VehicleOwnershipCoefficients::getIncomeAdj() const
{
	return incomeAdj;
}

void VehicleOwnershipCoefficients::setIncomeAdj(double incomeAdj)
{
	this->incomeAdj = incomeAdj;
}

double VehicleOwnershipCoefficients::getAboveSixty() const
{
	return aboveSixty;
}

void VehicleOwnershipCoefficients::setAboveSixty(double aboveSixty)
{
	this->aboveSixty = aboveSixty;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const VehicleOwnershipCoefficients& data)
        {
            return strm << "{"
						<< "\"vehicleOwnershipOptionId \":\"" << data.vehicleOwnershipOptionId 	<< "\","
						<< "\"incomeAdj \":\"" 	<< data.incomeAdj 	<< "\","
						<< "\"malay \":\"" 	<< data.malay 	<< "\","
						<< "\"indian \":\"" 	<< data.indian 	<< "\","
						<< "\"white_collar \":\"" 	<< data.whiteCollar	<< "\","
						<< "\"worker \":\"" 	<< data.worker 	<< "\","
						<< "\"HH_child1 \":\"" 	<< data.HHChild1 	<< "\","
						<< "\"HH_child_2plus \":\"" 	<< data.HHChild2Plus 	<< "\","
						<< "\"aboveSixty \":\"" 	<< data.aboveSixty 	<< "\","
						<< "\"taxi \":\"" 	<< data.taxi 	<< "\","
						<< "\"mrt_500m \":\"" 	<< data.mrt500m 	<< "\","
						<< "\"mrt_1000m \":\"" 	<< data.mrt1000m 	<< "\","
						<< "\"private_property \":\"" 	<< data.privateProperty 	<< "\","
						<< "\"logsum \":\"" 	<< data.logsum 	<< "\","
						<< "\"constant \":\"" 	<< data.constant 	<< "\","
						<< "}";
        }
    }
}



