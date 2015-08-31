/*
 * LogsumForDevModel.cpp
 *
 *  Created on: May 13, 2015
 *      Author: gishara
 */

#include "LogsumForDevModel.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

LogsumForDevModel::LogsumForDevModel(BigSerial taz2012Id, BigSerial taz2008Id, double accessibility): taz2012Id(taz2012Id), taz2008Id(taz2008Id), accessibility(accessibility){}

LogsumForDevModel::~LogsumForDevModel() {}

BigSerial LogsumForDevModel::gettAZ2012Id() const
{
	return taz2012Id;
}

BigSerial LogsumForDevModel::getTaz2008Id() const
{
    return taz2008Id;
}

double LogsumForDevModel::getAccessibility() const
{
    return accessibility;
}

//namespace sim_mob {
//    namespace long_term {
//
//        std::ostream& operator<<(std::ostream& strm, const LogsumForDevModel& data) {
//            return strm << "{"
//            		<< "\"fm_parcel_id\":\"" << data.fmParcelId << "\","
//                    << "\"taz_id\":\"" << data.tazId << "\","
//                    << "\"accessibility\":\"" << data.accessibility << "\","
//                    << "}";
//        }
//    }
//}



