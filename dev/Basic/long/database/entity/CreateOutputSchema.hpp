/*
 * CreateOutputSchema.hpp
 *
 *  Created on: Nov 23, 2015
 *      Author: gishara
 */

#pragma once

#include <ctime>
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class CreateOutputSchema
        {
        public:
        	CreateOutputSchema(BigSerial id = INVALID_ID, const std::string& tableName = EMPTY_STR, const std::string& query = EMPTY_STR);

            virtual ~CreateOutputSchema();

            struct OrderById
            {
            	bool operator ()( const CreateOutputSchema *a, const CreateOutputSchema *b ) const
            	{
            		return a->id< b->id;
            	}
            };

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            const std::string& getTableName() const;
            const std::string& getQuery() const;

            void setTableName(const std::string& tableName);
            void setQuery(const std::string& query);

            /**
             * Operator to print the Status if the world data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const CreateOutputSchema& data);

        private:
            friend class CreateOutputSchemaDao;

        private:
            BigSerial id;
            std::string tableName;
            std::string query;
         };
    }
}
