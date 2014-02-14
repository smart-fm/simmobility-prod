//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PopulationMongoDao.hpp
 *
 *  Created on: Nov 28, 2013
 *      Author: Harish Loganathan
 */

#pragma once

#include "behavioral/params/PersonParams.hpp"
#include "database/dao/MongoDao.hpp"
#include "database/DB_Config.hpp"
#include "logging/Log.hpp"

namespace sim_mob {
namespace medium {
class PopulationMongoDao : public db::MongoDao {
public:
	PopulationMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection);
	virtual ~PopulationMongoDao();

    /**
     * Gets all values from the source and put them on the given list.
     * @param outList to put the retrieved values.
     * @return true if some values were returned, false otherwise.
     */
    bool getAll(std::vector<PersonParams*>& outList) {
    	outList.reserve(connection.getSession<mongo::DBClientConnection>().count(collectionName, mongo::BSONObj()));
    	std::auto_ptr<mongo::DBClientCursor> cursor = connection.getSession<mongo::DBClientConnection>().query(collectionName, mongo::BSONObj());
    	while(cursor->more()) {
    		PersonParams* personParams = new PersonParams();
    		fromRow(cursor->next(), *personParams);
    		outList.push_back(personParams);
    	}
    	Print() << "Persons loaded from MongoDB: " << outList.size() << std::endl;
    	return true;
    }

    /**
     * Converts a given row into a PersonParams type.
     * @param result result row.
     * @param outParam (Out parameter) to receive data from row.
     */
    void fromRow(mongo::BSONObj document, PersonParams& outParam);
};
}
}
