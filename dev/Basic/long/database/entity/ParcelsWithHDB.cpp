/*
 * ParcelsWithHDB.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: gishara
 */

#include "ParcelsWithHDB.hpp"
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace sim_mob::long_term;

ParcelsWithHDB::ParcelsWithHDB(BigSerial fmParcelId,int unitTypeId):
		fmParcelId(fmParcelId),unitTypeId(unitTypeId) {}

ParcelsWithHDB::~ParcelsWithHDB() {}

ParcelsWithHDB& ParcelsWithHDB::operator=(const ParcelsWithHDB& source)
{
	this->fmParcelId 	= source.fmParcelId;
	this->unitTypeId	= source.unitTypeId;
    return *this;
}

template<class Archive>
void ParcelsWithHDB::serialize(Archive & ar,const unsigned int version)
{
	ar & fmParcelId;
	ar & unitTypeId;

}

void ParcelsWithHDB::saveParcelsWithHDB(std::vector<ParcelsWithHDB*> &s, const char * filename)
{
	// make an archive
	std::ofstream ofs(filename);
	boost::archive::binary_oarchive oa(ofs);
	oa & s;

}

std::vector<ParcelsWithHDB*> ParcelsWithHDB::loadSerializedData()
{
	std::vector<ParcelsWithHDB*> parcelsWithHDB;
	// Restore from saved data and print to verify contents
	std::vector<ParcelsWithHDB*> restored_info;
	{
		// Create and input archive
		std::ifstream ifs( "parcelsWithHDB" );
		boost::archive::binary_iarchive ar( ifs );

		// Load the data
		ar & restored_info;
	}

	for (auto *itr :restored_info)
	{
		ParcelsWithHDB *parcel = itr;
		parcelsWithHDB.push_back(parcel);
	}

	return parcelsWithHDB;
}

BigSerial ParcelsWithHDB::getFmParcelId() const
{
		return fmParcelId;
}

int ParcelsWithHDB::getUnitTypeId() const
{
		return unitTypeId;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const ParcelsWithHDB& data)
        {
            return strm << "{"
						<< "\"fmParcelId \":\"" << data.fmParcelId 	<< "\","
						<< "\"unitTypeId \":\"" 	<< data.unitTypeId 	<< "\","
						<< "}";
        }
    }
}





