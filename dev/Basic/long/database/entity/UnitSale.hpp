/*
 * UnitSale.hpp
 *
 *  Created on: Dec 15, 2015
 *      Author: gishara
 */

/*
 * UnitPriceSum.hpp
 *
 *  Created on: Aug 20, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class UnitSale {
        public:
        	UnitSale( BigSerial unitId = INVALID_ID, BigSerial buyerId = INVALID_ID, BigSerial sellerId = INVALID_ID,double unitPrice = 0.0, std::tm transactionDate = std::tm());
        	UnitSale(const UnitSale& source);
        	UnitSale& operator=(const UnitSale& source);
            virtual ~UnitSale();

            /**
             * Getters and Setters
             */
            BigSerial getBuyerId() const;
            BigSerial getSellerId() const;
            const std::tm& getTransactionDate() const;
            BigSerial getUnitId() const;
            double getUnitPrice() const;

            void setBuyerId(BigSerial buyerId);
            void setSellerId(BigSerial sellerId);
            void setTransactionDate(const std::tm& transactionDate);
            void setUnitId(BigSerial unitId);
            void setUnitPrice(double unitPrice);

        	private:
            friend class UnitSaleDao;

        	private:
            BigSerial unitId;
            BigSerial buyerId;
            BigSerial sellerId;
            double unitPrice;
            std::tm transactionDate;
        };
    }
}


