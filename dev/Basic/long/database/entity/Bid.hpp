//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Bid.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 5, 2013, 5:03 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include "metrics/Frame.hpp"
#include "agent/LT_Agent.hpp"

namespace sim_mob
{
    namespace long_term
    {
        /**
         * Represents a bid to a unit.
         */
        class Bid
        {
        public:
            Bid(BigSerial bidId, int simulationDay, BigSerial bidderId, BigSerial currentUnitId, BigSerial newUnitId,double willingnessToPay,double affordabilityAmount,double hedonicPrice,
            		double askingPrice,double targetPrice, double bidValue, int isAccepted,BigSerial currentPostcode, BigSerial newPostcode,LT_Agent* bidder,std::tm moveInDate, double wtpErrorTerm);

            Bid(BigSerial bidId,BigSerial currentUnitId, BigSerial newUnitId,BigSerial bidderId,LT_Agent* bidder,double bidValue, int simulationDay, double willingnessToPay, double wtp_e);
            Bid(const Bid& source);
            Bid();
            virtual ~Bid();

            /**
             * An operator to allow the bid copy.
             * @param source an Bid to be copied.
             * @return The Bid after modification
             */
            Bid& operator=(const Bid& source);

            BigSerial getBidId() const;
            int getSimulationDay() const;

            /**
             * Gets the Bidder unique identifier.
             * @return value with Bidder identifier.
             */
            BigSerial getBidderId() const;

            /**
             * Gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            BigSerial getCurrentUnitId() const;

            BigSerial getNewUnitId() const;

            /**
             * Gets the value of the willingness to pay of the bidder to this unit.
             * @return the value of the willingness to pay.
             */
            double getWillingnessToPay() const;
            
            double getAffordabilityAmount() const;
            double getHedonicPrice() const;
            double getAskingPrice() const;
            double getTargetPrice() const;

            /**
             * Gets the value of the bid.
             * @return the value of the bid.
             */
            double getBidValue() const;

           int getIsAccepted() const;
           BigSerial getCurrentPostcode() const;
           BigSerial getNewPostcode() const;

           /**
            * Gets the Bidder pointer.
            * @return bidder pointer.
            */
           LT_Agent* getBidder() const;

           const std::tm& getMoveInDate() const;
           /*
            * setters
            */
           void setAffordabilityAmount(double affordabilityAmount);
           void setAskingPrice(double askingPrice);
           void setBidderId(BigSerial bidderId);
           void setBidId(BigSerial bidId);
           void setBidValue(double bidValue);
           void setCurrentPostcode(BigSerial currentPostcode);
           void setHedonicPrice(double hedonicPrice);
           void setIsAccepted(int isAccepted) ;
           void setNewPostcode(BigSerial newPostcode);
           void setSimulationDay(int simulationDay);
           void setTargetPrice(double targetPrice);
           void setCurrentUnitId(BigSerial currUnitId);
           void setNewUnitId(BigSerial newUnitId);
           void setWillingnessToPay(double willingnessToPay);
           void setMoveInDate(const std::tm& moveInDate);

           void setWtpErrorTerm(double error);
           double getWtpErrorTerm() const;

           /**
            * Operator to print the Bid data.
            */
           friend std::ostream& operator<<(std::ostream& strm, const Bid& data);

        private:
           friend class BidDao;
           /**
            * Private constructor for DAO use.
            */
           // Bid();
        private:
           BigSerial bidId;
           int simulationDay;
           BigSerial bidderId;
           BigSerial currentUnitId;
           BigSerial newUnitId;
           double willingnessToPay;
           double wtpErrorTerm;
           double affordabilityAmount;
           double hedonicPrice;
           double askingPrice;
           double targetPrice;
           double bidValue;
           int isAccepted;
           BigSerial currentPostcode;
           BigSerial newPostcode;
           LT_Agent* bidder;
           std::tm moveInDate;
        };
    }
}

