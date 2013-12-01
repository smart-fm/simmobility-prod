//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ZoneMongoDao.hpp
 *
 *  Created on: Nov 30, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>
#include "behavioral/params/ZoneCostParams.hpp"
#include "database/dao/MongoDao.hpp"
#include "database/DB_Config.hpp"
#include "logging/Log.hpp"

namespace sim_mob {
namespace medium {
class ZoneMongoDao : db::MongoDao {
public:
	ZoneMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~ZoneMongoDao();

    /**
     * Gets all values from the source and put them on the given list.
     * @param outList to put the retrieved values.
     * @return true if some values were returned, false otherwise.
     */
    bool getAllZones(boost::unordered_map<int, ZoneParams>& outList) {
    	unsigned int i = 0;
    	std::auto_ptr<mongo::DBClientCursor> cursor = connection.getSession<mongo::DBClientConnection>().query(collectionName, mongo::Query());
    	while(cursor->more()) {
    		ZoneParams zoneParams;
    		fromRow(cursor->next(), zoneParams);
    		outList[zoneParams.getZoneId()] = zoneParams;
    		i++;
    	}
    	Print() << "Zones loaded from MongoDB: " << i << std::endl;
    	return true;
    }

    /**
     * Converts a given row into a PersonParams type.
     * @param result result row.
     * @param outParam (Out parameter) to receive data from row.
     */
    void fromRow(mongo::BSONObj document, ZoneParams& outParam);
};

class CostMongoDao : db::MongoDao {
public:
	CostMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~CostMongoDao();

    /**
     * Gets multiple values from the source and put them on the given list.
     * @param outList to put the retrieved values.
     * @return true if some values were returned, false otherwise.
     */
    bool getMultiple(mongo::Query query, boost::unordered_map<const std::string, CostParams*>& outList) {
    	unsigned int i = 0;
    	std::auto_ptr<mongo::DBClientCursor> cursor = connection.getSession<mongo::DBClientConnection>().query(collectionName, query);
    	while(cursor->more()) {
    		CostParams* costParams = new CostParams();
    		fromRow(cursor->next(), *costParams);
    		outList[costParams->getOrgDest()] = costParams;
    		i++;
    	}
    	return true;
    }

    /**
     * Converts a given row into a PersonParams type.
     * @param result result row.
     * @param outParam (Out parameter) to receive data from row.
     */
    void fromRow(mongo::BSONObj document, CostParams& outParam);
};
}
}
