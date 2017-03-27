//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   PostcodeAmenities.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 3:05 PM
 */

#include "PostcodeAmenities.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

PostcodeAmenities::PostcodeAmenities(): postcode(EMPTY_STR),
										mrtStation(EMPTY_STR), distanceToMRT(0), distanceToBus(0), distanceToExpress(0), distanceToPMS30(0),
										distanceToCBD(0), distanceToMall(0), distanceToJob(0), mrt_200m(false), mrt_400m(false), express_200m(false),
										bus_200m(false), bus_400m(false), pms_1km(false) {}

PostcodeAmenities::~PostcodeAmenities() {
}

bool PostcodeAmenities::hasPms_1km() const {
    return pms_1km;
}

bool PostcodeAmenities::hasBus_400m() const {
    return bus_400m;
}

bool PostcodeAmenities::hasBus_200m() const {
    return bus_200m;
}

bool PostcodeAmenities::hasExpress_200m() const {
    return express_200m;
}

bool PostcodeAmenities::hasMRT_400m() const {
    return mrt_400m;
}

bool PostcodeAmenities::hasMRT_200m() const {
    return mrt_200m;
}

double PostcodeAmenities::getDistanceToJob() const {
    return distanceToJob;
}

double PostcodeAmenities::getDistanceToMall() const {
    return distanceToMall;
}

double PostcodeAmenities::getDistanceToCBD() const {
    return distanceToCBD;
}

double PostcodeAmenities::getDistanceToPMS30() const {
    return distanceToPMS30;
}

double PostcodeAmenities::getDistanceToExpress() const {
    return distanceToExpress;
}

double PostcodeAmenities::getDistanceToBus() const {
    return distanceToBus;
}

double PostcodeAmenities::getDistanceToMRT() const {
    return distanceToMRT;
}

const std::string& PostcodeAmenities::getMrtStation() const {
    return mrtStation;
}

void PostcodeAmenities::setBus200m(bool bus200m)
{
	bus_200m = bus200m;
}

void PostcodeAmenities::setBus400m(bool bus400m)
{
	bus_400m = bus400m;
}

void PostcodeAmenities::setDistanceToBus(double distanceToBus)
{
	this->distanceToBus = distanceToBus;
}

void PostcodeAmenities::setDistanceToCbd(double distanceToCbd)
{
	distanceToCBD = distanceToCbd;
}

void PostcodeAmenities::setDistanceToExpress(double distanceToExpress)
{
	this->distanceToExpress = distanceToExpress;
}

void PostcodeAmenities::setDistanceToJob(double distanceToJob)
{
	this->distanceToJob = distanceToJob;
}

void PostcodeAmenities::setDistanceToMall(double distanceToMall)
{
	this->distanceToMall = distanceToMall;
}

void PostcodeAmenities::setDistanceToMrt(double distanceToMrt)
{
	distanceToMRT = distanceToMrt;
}

void PostcodeAmenities::setDistanceToPms30(double distanceToPms30)
{
	distanceToPMS30 = distanceToPms30;
}

void PostcodeAmenities::setExpress200m(bool express200m)
{
	express_200m = express200m;
}

void PostcodeAmenities::setMrt200m(bool mrt200m)
{
	mrt_200m = mrt200m;
}

void PostcodeAmenities::setMrt400m(bool mrt400m)
{
	mrt_400m = mrt400m;
}

void PostcodeAmenities::setMrtStation(const std::string& mrtStation)
{
	this->mrtStation = mrtStation;
}

void PostcodeAmenities::setPms1km(bool pms1km)
{
	pms_1km = pms1km;
}

void PostcodeAmenities::setPostcode(const std::string& postcode)
{
	this->postcode = postcode;
}

BigSerial PostcodeAmenities::getAddressId() const
{
	return addressId;
}

void PostcodeAmenities::setAddressId(BigSerial addressId)
{
	this->addressId = addressId;
}

const std::string& PostcodeAmenities::getPostcode() const
{
	return this->postcode;
}

BigSerial PostcodeAmenities::getTazId() const
{
	return tazId;
}

void PostcodeAmenities::setTazId(BigSerial tazId)
{
	this->tazId = tazId;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const PostcodeAmenities& data) {
            return strm << "{"
                    << "\"postcode\":\"" << data.postcode << "\","
                    << "\"mrtStation\":\"" << data.mrtStation << "\","
                    << "\"distanceMrt\":\"" << data.distanceToMRT << "\","
                    << "\"distanceBus\":\"" << data.distanceToBus << "\","
                    << "\"distanceExpress\":\"" << data.distanceToExpress << "\","
                    << "\"distancePms30\":\"" << data.distanceToPMS30 << "\","
                    << "\"distanceCBD\":\"" << data.distanceToCBD << "\","
                    << "\"distanceMall\":\"" << data.distanceToMall << "\","
                    << "\"distanceJob\":\"" << data.distanceToJob << "\","
                    << "\"mrt_200m\":\"" << data.mrt_200m << "\","
                    << "\"mrt_400m\":\"" << data.mrt_400m << "\","
                    << "\"express_200m\":\"" << data.express_200m << "\","
                    << "\"bus_200m\":\"" << data.bus_200m << "\","
                    << "\"bus_400m\":\"" << data.bus_400m << "\","
                    << "\"pms_1km\":\"" << data.pms_1km << "\""
                    << "}";
        }
    }
}
