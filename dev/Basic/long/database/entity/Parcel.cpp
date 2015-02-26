//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Parcel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 10, 2014, 5:54 PM
 */

#include "Parcel.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

Parcel::Parcel(BigSerial id,float lot_size, std::string gpr,std::string owner_name,  int	owner_category, std::tm last_transaction_date,
	   float last_transaction_type_total, float psm_per_gps, int lease_type, std::tm lease_start_date, float centroid_x, float centroid_y, std::tm award_date,
       bool	award_status, std::string use_restriction, int	successful_tender_id,
       float successful_tender_price, std::tm tender_closing_date,int lease,float actual_gpr,float allowed_gpr, int land_use_type_id,int development_type_code,int status,
       int developmentAllowed, std::tm nextAvailableDate)
	   : id(id),lot_size(lot_size), owner_name(owner_name), owner_category(owner_category),
	     last_transaction_date(last_transaction_date), last_transaction_type_total(last_transaction_type_total), psm_per_gps(psm_per_gps),
	     lease_type(lease_type), lease_start_date(lease_start_date), centroid_x(centroid_x), centroid_y(centroid_y), award_date(award_date),
	     award_status(award_status), use_restriction(use_restriction),successful_tender_id(successful_tender_id), successful_tender_price(successful_tender_price),
	     tender_closing_date(tender_closing_date), lease(lease),gpr(gpr),land_use_type_id(land_use_type_id),development_type_code(development_type_code),status(status),
	     developmentAllowed(developmentAllowed),nextAvailableDate(nextAvailableDate)
{
}

Parcel::~Parcel() {}

BigSerial Parcel::getId() const
{
    return id;
}

float Parcel::getLotSize() const
{
	return lot_size;
}

std::string Parcel::getGpr() const
{
	return gpr;
}

int Parcel::getLandUseTypeId() const
{
	return land_use_type_id;
}

std::string Parcel::getOwnerName() const
{
		return owner_name;
}

int	Parcel::getOwnerCategory() const
{
	return owner_category;
}

std::tm Parcel::getLastTransactionDate() const
{
	return last_transaction_date;
}

float Parcel::getLastTransationTypeTotal() const
{
	return last_transaction_type_total;
}

float Parcel::getPsmPerGps() const
{
	return psm_per_gps;
}

int	Parcel::getLeaseType() const
{
	return lease_type;
}

std::tm	Parcel::getLeaseStartDate() const
{
	return lease_start_date;
}

float Parcel::getCentroidX() const
{
	return centroid_x;
}

float Parcel::getCentroidY() const
{
	return centroid_y;
}

std::tm	Parcel::getAwardDate() const
{
	return award_date;
}

bool Parcel::getAwardStatus() const
{
	return award_status;
}

std::string	Parcel::getUseRestriction() const
{
			return use_restriction;
}

int	Parcel::getDevelopmentTypeCode() const
{
	return development_type_code;
}

int	Parcel::getSuccessfulTenderId() const
{
	return successful_tender_id;
}

float Parcel::getSuccessfulTenderPrice() const
{
	return successful_tender_price;
}

std::tm	Parcel::getTenderClosingDate() const
{
	return tender_closing_date;
}

int	Parcel::getLease() const
{
	return lease;
}

int	Parcel::getStatus() const
{
	return status;
}

void Parcel::setStatus(int status)
{
	this->status = status;
}

int Parcel::getDevelopmentAllowed() const
{
	return developmentAllowed;
}

void Parcel::setDevelopmentAllowed(int developmentAllowed)
{
	this->developmentAllowed = developmentAllowed;
}

std::tm Parcel::getNextAvailableDate() const
{
	return nextAvailableDate;
}

void Parcel::setNextAvailableDate(std::tm nextAvailableDate)
{
	this->nextAvailableDate = nextAvailableDate;
}


namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Parcel& data) {
            return strm << "{"
						<< "\"id\":\"" << data.id << "\","
						<< "\"lot_size\":\"" << data.lot_size << "\","
						<< "\"actual_gpr\":\"" << data.gpr << "\","
						<< "\"land_use_type_id\":\"" << data.land_use_type_id << "\","
						<< "\"owner_name\":\"" << data.owner_name << "\","
						<< "\"owner_category\":\"" << data.owner_category << "\","
						<< "\"last_transaction_date\":\"" << data.last_transaction_date.tm_year << data.last_transaction_date.tm_mon << data.last_transaction_date.tm_mday << "\","
						<< "\"last_transaction_type_total\":\"" << data.last_transaction_type_total << "\","
						<< "\"psm_per_gps\":\"" << data.psm_per_gps << "\","
						<< "\"lease_type\":\"" << data.lease_type << "\","
						<< "\"lease_start_date\":\"" << data.lease_start_date.tm_year << data.lease_start_date.tm_mon << data.lease_start_date.tm_mday << "\","
						<< "\"centroid_x\":\"" << data.centroid_x << "\","
						<< "\"centroid_y\":\"" << data.centroid_y << "\","
						<< "\"award_date\":\"" << data.award_date.tm_year << data.award_date.tm_year << data.award_date.tm_mon << data.award_date.tm_mday << "\","
						<< "\"award_status\":\"" << data.award_status << "\","
						<< "\"use_restriction\":\"" << data.use_restriction << "\","
						<< "\"development_type_code\":\"" << data.development_type_code << "\","
						<< "\"successful_tender_id\":\"" << data.successful_tender_id << "\","
						<< "\"successful_tender_price\":\"" << data.successful_tender_price << "\","
						<< "\"tender_closing_date\":\"" << data.tender_closing_date.tm_year << data.tender_closing_date.tm_mday << "\","
						<< "\"lease\":\"" << data.lease << "\","
						<< "\"status\":\"" << data.status << "\","
						<< "\"development_allowed\":\"" << data.developmentAllowed << "\","
						<< "\"next_available_date\":\"" << data.nextAvailableDate.tm_year << data.nextAvailableDate.tm_mon << data.nextAvailableDate.tm_mday << "\""
						<< "}";

        }
    }
}
