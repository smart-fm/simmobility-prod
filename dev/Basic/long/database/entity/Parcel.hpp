//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Parcel.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 10, 2014, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class Parcel
        {
        public:
            Parcel(BigSerial id = INVALID_ID,BigSerial tazId = INVALID_ID, float lot_size = .0, std::string gpr = EMPTY_STR, int land_use_type_id = 0, std::string owner_name = EMPTY_STR,  int	owner_category = 0, std::tm last_transaction_date = std::tm(), float last_transation_type_total = .0,
                   float	psm_per_gps = .0, int lease_type = 0, std::tm lease_start_date = std::tm(), float centroid_x = .0, float centroid_y = .0, std::tm award_date = std::tm(),bool	award_status = false, int	use_restriction = 0, int	development_type_code = 0,
                   int	successful_tender_id = 0, float	successful_tender_price = .0,std::tm	tender_closing_date = std::tm(), int lease = 0, int status = 0,
                   int developmentAllowed = 0, std::tm nextAvailableDate = std::tm(), std::tm lastChangedDate = std::tm());

            virtual ~Parcel();

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            BigSerial getTazId() const;
            float	getLotSize() const;
            std::string	getGpr() const;
            int getLandUseTypeId() const;
            std::string getOwnerName() const;
            int	getOwnerCategory() const;
            std::tm getLastTransactionDate() const;
            float	getLastTransationTypeTotal() const;
            float	getPsmPerGps() const;
            int		getLeaseType() const;
            std::tm	getLeaseStartDate() const;
            float	getCentroidX() const;
            float	getCentroidY() const;
            std::tm	getAwardDate() const;
            bool	getAwardStatus() const;
            int	getUseRestriction() const;
            int 	getDevelopmentTypeCode() const;
            int		getSuccessfulTenderId() const;
            float	getSuccessfulTenderPrice() const;
            std::tm	getTenderClosingDate() const;
            int		getLease() const;
            int getStatus()const;
            void setStatus(int status);
            int getDevelopmentAllowed() const;
            void setDevelopmentAllowed(int developmentAllowed);
            std::tm getNextAvailableDate() const;
            void setNextAvailableDate(std::tm date);
            std::tm getLastChangedDate() const;
            void setLastChangedDate(std::tm date);


            /**
             * Operator to print the Parcel data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Parcel& data);
        private:
            friend class ParcelDao;

        private:
            BigSerial id;
            BigSerial tazId;
            float	lot_size;
            std::string	gpr;
            int land_use_type_id;
            std::string owner_name;
            int	owner_category;
            std::tm last_transaction_date;
            float	last_transaction_type_total;
            float	psm_per_gps;
            int		lease_type;
            std::tm	lease_start_date;
            float	centroid_x;
            float	centroid_y;
            std::tm	award_date;
            bool	award_status;
            int	use_restriction;
            int	development_type_code;
            int		successful_tender_id;
            float	successful_tender_price;
            std::tm	tender_closing_date;
            int		lease;
            int status;
            int developmentAllowed;
            std::tm nextAvailableDate;
            std::tm lastChangedDate;
         };
    }
}
