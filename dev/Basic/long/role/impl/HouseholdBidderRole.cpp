//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */

#include <cmath>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include "HouseholdBidderRole.hpp"
#include "message/LT_Message.hpp"
#include "event/EventPublisher.hpp"
#include "event/EventManager.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "util/Statistics.hpp"
#include "util/SharedFunctions.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "model/HM_Model.hpp"
#include "model/ScreeningSubModel.hpp"
#include "database/entity/VehicleOwnershipChanges.hpp"
#include "core/AgentsLookup.hpp"
#include "core/DataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "behavioral/PredayLT_Logsum.hpp"
#include "model/HedonicPriceSubModel.hpp"
#include "model/WillingnessToPaySubModel.hpp"
#include "util/PrintLog.hpp"
#include "model/VehicleOwnershipModel.hpp"


using std::list;
using std::endl;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using boost::format;

namespace
{
    /**
     * Send given bid to given owner.
     * @param owner of the unit
     * @param bid to send.
     */
    inline void bid(MessageHandler* owner, const Bid& bid)
    {
        MessageBus::PostMessage(owner, LTMID_BID, MessageBus::MessagePtr(new BidMessage(bid)));
    }
}

HouseholdBidderRole::CurrentBiddingEntry::CurrentBiddingEntry( const BigSerial unitId, double bestBid, const double wp, double lastSurplus, double wtp_e, double affordability )
															 : unitId(unitId), bestBid(bestBid), wp(wp), tries(0), lastSurplus(lastSurplus), wtp_e(wtp_e), affordability(affordability){}

HouseholdBidderRole::CurrentBiddingEntry::~CurrentBiddingEntry()
{
    invalidate();
}

BigSerial HouseholdBidderRole::CurrentBiddingEntry::getUnitId() const
{
    return unitId;
}


double HouseholdBidderRole::CurrentBiddingEntry::getAffordability() const
{
	return affordability;
}

void HouseholdBidderRole::CurrentBiddingEntry::setAffordability(double value)
{
	affordability = value;
}


double HouseholdBidderRole::CurrentBiddingEntry::getWP() const
{
    return wp;
}

double HouseholdBidderRole::CurrentBiddingEntry::getBestBid() const
{
	return bestBid;
}

void HouseholdBidderRole::CurrentBiddingEntry::setBestBid(double val)
{
	bestBid = val;
}


long int HouseholdBidderRole::CurrentBiddingEntry::getTries() const
{
    return tries;
}

void HouseholdBidderRole::CurrentBiddingEntry::incrementTries(int quantity)
{
    tries += quantity;
}

bool HouseholdBidderRole::CurrentBiddingEntry::isValid() const
{
    return (unitId != INVALID_ID);
}

void HouseholdBidderRole::CurrentBiddingEntry::invalidate()
{
    unitId = INVALID_ID;
    tries = 0;
    wp = 0;
}

double HouseholdBidderRole::CurrentBiddingEntry::getLastSurplus() const
{
	return lastSurplus;
}

void HouseholdBidderRole::CurrentBiddingEntry::setLastSurplus(double value)
{
	lastSurplus = value;
}

double HouseholdBidderRole::CurrentBiddingEntry::getWtp_e()
{
	return wtp_e;
}

void HouseholdBidderRole::CurrentBiddingEntry::setWtp_e(double value)
{
	wtp_e = value;
}


HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent): parent(parent), waitingForResponse(false), lastTime(0, 0), bidOnCurrentDay(false), active(false), unitIdToBeOwned(0),
																  moveInWaitingTimeInDays(-1),vehicleBuyingWaitingTimeInDays(0), day(day), initBidderRole(true),year(0),bidComplete(true){}

HouseholdBidderRole::~HouseholdBidderRole(){}

HouseholdAgent* HouseholdBidderRole::getParent()
{
	return parent;
}

bool HouseholdBidderRole::isActive() const
{
    return active;
}

void HouseholdBidderRole::setActive(bool activeArg)
{
    active = activeArg;
    if( getParent()->getHousehold() != nullptr)
    {
    	getParent()->getHousehold()->setIsBidder(activeArg);
    }
}

void HouseholdBidderRole::computeHouseholdAffordability()
{
	double householdAffordabilityAmount = 0;
	//This is the inflation-adjusted income of individuals through the years starting from age 20 (first element) based on the 2012 HITS survey.
	//This model was done by Jingsi Shaw [xujs@mit.edu]
	int incomeProjection[] = { 	13, 15, 16, 18, 21, 23, 26, 28, 31, 34, 37, 41, 44, 48, 51, 55, 59, 63, 66, 70, 74, 77, 81, 84, 87, 90, 92, 94, 96, 98, 99,
								100, 100, 100, 100, 99, 98, 96, 95, 92, 90, 87, 84, 81, 78, 74, 71, 67, 63, 59, 56, 52, 48, 45, 41, 38, 35, 32, 29, 26, 23 };

	Household *bidderHousehold = const_cast<Household*>(getParent()->getHousehold()); 

	std::vector<BigSerial> individuals = bidderHousehold->getIndividuals();

	int householdSize = individuals.size();

	const double DTIR_Single = 0.33; //Debt to income ratio of a single person.
	const double DTIR_Couple = 0.30; //Debt to income ratio of a child-less couple.
	const double DTIR_Family = 0.27; //Debt to income ratio of a family.
	const int retirementAge  = 65;
	const int maturityAge 	 = 18;

	double debtToIncomeRatio = DTIR_Single;

	int children = 0;
	int householdHeadAge = 0;
	if( householdSize > 1 )
	{
		children = 0;
		for( int n = 0; n < householdSize; n++ )
		{
			Individual * householdIndividual = getParent()->getModel()->getIndividualById( individuals[n] );
			std::tm dob = householdIndividual->getDateOfBirth();

			int age = HITS_SURVEY_YEAR  - 1900 - dob.tm_year;

			if( householdIndividual->getHouseholdHead() )
			{
				householdHeadAge = age;
			}

			if( age < maturityAge )
			{
				children++;
			}
		}

		debtToIncomeRatio = DTIR_Couple;

		if(children > 0 )
		{
			debtToIncomeRatio = DTIR_Family;
		}
	}

	//
	const Household *household = getParent()->getHousehold();
	int maxMortgage = 0;

	for( int n = 0; n < household->getSize(); n++ )
	{
		Individual * householdIndividual = getParent()->getModel()->getIndividualById( individuals[n] );
		std::tm dob = householdIndividual->getDateOfBirth();

		double income = debtToIncomeRatio * householdIndividual->getIncome();
		double loanTenure = ( retirementAge - ( HITS_SURVEY_YEAR - ( 1900 + dob.tm_year ) ) ) * 12.0; //times 12 to get the tenure in months, not years.

		loanTenure = std::min( 360.0, loanTenure ); //tenure has a max for 30 years.

		HM_Model::HousingInterestRateList *interestRateListX = getParent()->getModel()->getHousingInterestRateList();

		const double quarter = 365.0 / 4.0; // a yearly quarter
		const int FirstQuarter2012 = 45;  // index 45 is 1st quarter 2012 in table housing_interest_rate
		int index =	FirstQuarter2012 + (day / quarter);

		index = index % (*interestRateListX).size();

		double interestRate = (*interestRateListX)[index]->getRate_real() / 100.0 / 12.0; // divide by 12 to get the monthly interest rate.

		//Household affordability formula based on excel PV function:
		//https://support.office.com/en-ca/article/PV-function-3d25f140-634f-4974-b13b-5249ff823415
		double mortgage = income / interestRate *  ( 1.0 - pow( 1 + interestRate, -loanTenure ) );

		mortgage = std::max(0.0, mortgage);
		//PrintOutV("mortage " << householdIndividual->getId() << " " << mortgage << std::endl);
		maxMortgage += mortgage;
	}

	//
	// We use the current income as a baseline and estimate the income to the current age
	// starting from 20 years old. We can then estimate how much this individual has saved.
	double alpha = 0.03;

	if( householdSize > 1 )
		alpha = 0.05;

	double savedIncome = 0.0;
	double individualIncome = 0.0;

	for( int m = 0; m < household->getSize(); m++ )
	{
		Individual * householdIndividual = getParent()->getModel()->getIndividualById( individuals[m] );
		std::tm dob = householdIndividual->getDateOfBirth();

		int age = HITS_SURVEY_YEAR - ( 1900 + dob.tm_year );

		for( int n = 0; n < ( age - 20 ); n++ )
		{
			double normIncome = householdIndividual->getIncome() * 12 * (0.3 + alpha);
			double increment = ( ( normIncome / incomeProjection[ ( age - 20 ) ] ) * incomeProjection[n] );

			individualIncome = individualIncome + increment;
		}

		savedIncome = savedIncome + individualIncome;
		individualIncome = 0;
	}

	double maxDownpayment = savedIncome + ( bidderHousehold->getIncome() * 12 * (0.3 + alpha) );

	maxDownpayment = std::max(0.0, maxDownpayment);

	householdAffordabilityAmount = ( maxMortgage + maxDownpayment ) / 1000000.0; //Roberto's model calculates housing prices in units of million.
	householdAffordabilityAmount = std::max(householdAffordabilityAmount, 0.0);

	bidderHousehold->setAffordabilityAmount( householdAffordabilityAmount );

	HM_Model *model = getParent()->getModel();

	Unit *unit = const_cast<Unit*>(model->getUnitById( household->getUnitId() ));

	HedonicPrice_SubModel hpSubmodel(day, model, unit);

	std::vector<ExpectationEntry> expectations;
	hpSubmodel.ComputeExpectation(1, expectations);

	double price = 0;

	if(expectations.size() > 0 )
		price = expectations[0].hedonicPrice;

	bidderHousehold->setCurrentUnitPrice( price );
}

void HouseholdBidderRole::init()
{
	TimeCheck affordabilityTimeCheck;

	computeHouseholdAffordability();

	double affordabilityTime = affordabilityTimeCheck.getClockTime();

	#ifdef VERBOSE_SUBMODEL_TIMING
		PrintOutV(" affordabilityTime for agent " << getParent()->getId() << " is " << affordabilityTime << std::endl );
	#endif

	initBidderRole = false;
}

void HouseholdBidderRole::update(timeslice now)
{
	day = now.ms();

	if(initBidderRole)
	{
		init();
	}

	//reconsiderVehicleOwnershipOption();
	//This bidder has a successful bid already.
	//It's now waiting to move in its new unit.
	//The bidder role will do nothing else during this period (hence the return at the end of the if function).
	if( moveInWaitingTimeInDays > 0 )
	{
		getParent()->getModel()->incrementWaitingToMove();
		Statistics::increment(Statistics::N_WAITING_TO_MOVE);

		//Just before we set the bidderRole to inactive, we do the unit ownership switch.
		if( moveInWaitingTimeInDays == 1 )
		{
			TakeUnitOwnership();
		}

		moveInWaitingTimeInDays--;

		//return;
	}

	//wait x days after move in to a new unit to reconsider the vehicle ownership option.
//	if( vehicleBuyingWaitingTimeInDays > 0 && moveInWaitingTimeInDays == 0)
//	{
//
//		if( vehicleBuyingWaitingTimeInDays == 1)
//		{
//			TimeCheck vehicleOwnershipTiming;
//
//			VehicleOwnershipModel vehOwnershipModel(getParent()->getModel());
//			vehOwnershipModel.reconsiderVehicleOwnershipOption2(*getParent()->getHousehold(),getParent(), day,true,false);
//
//			double vehicleOwnershipTime = vehicleOwnershipTiming.getClockTime();
//
//			#ifdef VERBOSE_SUBMODEL_TIMING
//				PrintOutV("vehicleOwnership time for agent " << getParent()->getId() << " is " << vehicleOwnershipTime << std::endl );
//			#endif
//		}
//			vehicleBuyingWaitingTimeInDays--;
//
//		//return;
//	}

    //based on the last bid status a household can't bid again until it passes awakeningOffMarketSuccessfulBid/awakeningOffMarketUnsuccessfulBid
	if(getParent()->getHousehold()->getTimeOffMarket() <= 0)
    {
    	bidUnit(now);
    	getParent()->getModel()->incrementNumberOfBidders();
    	Statistics::increment(Statistics::N_BIDDERS);
    }

    lastTime = now;
}

void HouseholdBidderRole::TakeUnitOwnership()
{
	#ifdef VERBOSE
	PrintOutV("[day " << day << "] Household " << getParent()->getId() << " is moving into unit " << unitIdToBeOwned << " today." << std::endl);
	#endif
	getParent()->addUnitId( unitIdToBeOwned );

	getParent()->getHousehold()->setUnitId(unitIdToBeOwned);
	getParent()->getHousehold()->setHasMoved(1);
	getParent()->getHousehold()->setUnitPending(0);
	getParent()->getHousehold()->setTenureStatus(1);
	Unit *unit = getParent()->getModel()->getUnitById(unitIdToBeOwned);
	//update the unit tenure status to "owner occupied" when a household moved to a new unit.
	unit->setTenureStatus(1);
	unit->setbiddingMarketEntryDay(day + unit->getTimeOffMarket());


    biddingEntry.invalidate();
   // Statistics::increment(Statistics::N_ACCEPTED_BIDS);
}


void HouseholdBidderRole::HandleMessage(Message::MessageType type, const Message& message)
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
    switch (type)
    {
        case LTMID_BID_RSP:// Bid response received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            switch (msg.getResponse())
            {
                case ACCEPTED:// Bid accepted 
                {
                	unitIdToBeOwned = msg.getBid().getNewUnitId();
                	const Unit *newUnit = getParent()->getModel()->getUnitById(unitIdToBeOwned);
                	boost::gregorian::date moveInDate = boost::gregorian::date_from_tm(newUnit->getOccupancyFromDate());
                	boost::gregorian::date simulationDate(HITS_SURVEY_YEAR, 1, 1);
                	boost::gregorian::date_duration dt(day);
                	simulationDate = simulationDate + dt;

                	if( simulationDate <  moveInDate )
                		moveInWaitingTimeInDays = ( moveInDate - simulationDate ).days();
                	else
                		moveInWaitingTimeInDays = config.ltParams.housingModel.housingMoveInDaysInterval;

#ifdef VERBOSE					
					PrintOutV("moveInDays " << moveInWaitingTimeInDays
											<< " occupancy "
											<< newUnit->getOccupancyFromDate().tm_year
											<< newUnit->getOccupancyFromDate().tm_mon
											<< newUnit->getOccupancyFromDate().tm_mday
											<< " moveInDate "
											<< moveInDate
											<< " simDay "
											<< simulationDate
											<< "  "
											<< endl);
#endif

                	vehicleBuyingWaitingTimeInDays = config.ltParams.vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays;
                	int simulationEndDay = config.ltParams.days;
                	year = config.ltParams.year;
                	getParent()->getHousehold()->setLastBidStatus(1);

                	getParent()->getHousehold()->setTimeOffMarket(moveInWaitingTimeInDays + config.ltParams.housingModel.awakeningModel.awakeningOffMarketSuccessfulBid);
            		getParent()->setAcceptedBid(true);
            		getParent()->setBuySellInterval(config.ltParams.housingModel.offsetBetweenUnitBuyingAndSelling);

                	if(simulationEndDay < (moveInWaitingTimeInDays + day))
                	{
                		getParent()->getHousehold()->setUnitId(unitIdToBeOwned);
                		getParent()->getHousehold()->setHasMoved(0);
                		getParent()->getHousehold()->setUnitPending(1);
                		moveInWaitingTimeInDays = (moveInWaitingTimeInDays + day) - simulationEndDay;
                		getParent()->getHousehold()->setPendingFromDate(getDateBySimDay(year,moveInWaitingTimeInDays));
                	}

                    break;
                }
                case NOT_ACCEPTED:
                {
                    biddingEntry.incrementTries();
                    getParent()->getHousehold()->setLastBidStatus(2);
                    if(getParent()->getHouseholdBiddingWindow() == 0)
                    {
                    	getParent()->getHousehold()->setTimeOffMarket(config.ltParams.housingModel.awakeningModel.awakeningOffMarketUnsuccessfulBid);
                    }
                    break;
                }
                case BETTER_OFFER:
                {
                    break;
                }
                case NOT_AVAILABLE:
                {
                    biddingEntry.invalidate();
                    break;
                }
                default:break;
            }
            waitingForResponse = false;
            Statistics::increment(Statistics::N_BID_RESPONSES);
            break;
        }
        default:break;
    }
}

bool HouseholdBidderRole::bidUnit(timeslice now)
{
    HousingMarket* market = getParent()->getMarket();
    const Household* household = getParent()->getHousehold();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    HM_Model* model = getParent()->getModel();
    
    // Following the new assumptions of the model each household will stick on the 
    // unit where he is bidding until he gets rejected for seller by NOT_AVAILABLE/BETTER_OFFER 
    // or the the speculation for the given unit is 0. This last means that the household
    // does not have more margin of negotiation then is better look for another unit.
    const HousingMarket::Entry* entry = market->getEntryById(biddingEntry.getUnitId());

    if (!entry || !biddingEntry.isValid())
    {
        //if unit is not available or entry is not valid then
        //just pick another unit to bid.

    	TimeCheck pickUnitTiming;

        if(pickEntryToBid())
        {
            entry = market->getEntryById(biddingEntry.getUnitId());
        }   

        double pickUnitTime = pickUnitTiming.getClockTime();

		#ifdef  VERBOSE_SUBMODEL_TIMING
        PrintOutV("pickUnit for household " << getParent()->getId() << " is " << pickUnitTime << std::endl );
		#endif
    }
    else
    {
    	//we are now rebidding on the same unit as yesterday.
    	//We will now increase the bid by 20 % of the difference of the bid and the AP
    	biddingEntry.setBestBid( biddingEntry.getBestBid() + fabs( entry->getAskingPrice() - biddingEntry.getBestBid() ) * 0.2 );
    }
    
    if (entry && biddingEntry.isValid())
    {
		const Unit* unit = model->getUnitById(entry->getUnitId());
		const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

		if(unit && stats)
		{
			if (entry->getOwner() && biddingEntry.getBestBid() > 0.0f)
			{
				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Household " << std::dec << household->getId() << " submitted a bid of $" << biddingEntry.getBestBid() << "[wp:$" << biddingEntry.getWP() << ",bids:"  <<   biddingEntry.getTries() << ",ap:$" << entry->getAskingPrice() << "] on unit " << biddingEntry.getUnitId() << " to seller " <<  entry->getOwner()->getId() << "." << std::endl );
				#endif

				Bid newBid(model->getBidId(),household->getUnitId(),entry->getUnitId(), household->getId(), getParent(), biddingEntry.getBestBid(), now.ms()-1, biddingEntry.getWP(), biddingEntry.getWtp_e(), biddingEntry.getAffordability());
				bid(entry->getOwner(), newBid);
				Statistics::increment(Statistics::N_BIDS);
				model->incrementBids();
				writeNewBidsToFile(model->getBidId(),household->getUnitId(),entry->getUnitId(), household->getId(), biddingEntry.getBestBid(), now.ms());
				ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
				//add the bids active on last day to op schema
				if(now.ms() == (config.ltParams.days-1))
				{
					boost::shared_ptr<Bid> newBidPtr = boost::make_shared<Bid>(newBid);
					newBidPtr->setMoveInDate(getDateBySimDay(1900,0)); // set the move in date to a default of 1900-01-01, since it is not decided at this stage.
					newBidPtr->setAskingPrice(entry->getAskingPrice());
					newBidPtr->setHedonicPrice(entry->getHedonicPrice());
					newBidPtr->setSellerId(entry->getOwner()->getId());
					model->addNewBids(newBidPtr);
				}
				return true;
			}
		}
    }
    return false;
}



int HouseholdBidderRole::getMoveInWaitingTimeInDays()
{
	return moveInWaitingTimeInDays;
}

void HouseholdBidderRole::calculateMaxSurplusEntry(const HousingMarket::Entry* entry,double &maxSurplus, double &finalBid, double &maxWp,double &maxAffordability,double &maxWtpe,BigSerial &maxEntryUnitId)
{
	HM_Model* model = getParent()->getModel();
	const Household* household = getParent()->getHousehold();
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	const Unit* unit = model->getUnitById(entry->getUnitId());
	const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

	bool flatEligibility = true;

	if( unit->getTenureStatus() == 0 && unit->getUnitType() == 2 && household->getTwoRoomHdbEligibility()  == false )
		flatEligibility = false;

	if( unit->getTenureStatus() == 0 && unit->getUnitType() == 3 && household->getThreeRoomHdbEligibility() == false )
		flatEligibility = false;

	if( unit->getTenureStatus() == 0 && unit->getUnitType() == 4 && household->getFourRoomHdbEligibility() == false )
		flatEligibility = false;

	if( stats && flatEligibility )
	{
		const Unit *hhUnit = model->getUnitById( household->getUnitId() );

		BigSerial postcodeCurrent = 0;
		if( hhUnit != NULL )
			postcodeCurrent = model->getUnitSlaAddressId( hhUnit->getId() );

		Postcode *oldPC = model->getPostcodeById(postcodeCurrent);
		Postcode *newPC = model->getPostcodeById( model->getUnitSlaAddressId( unit->getId() ) );
		double wtp_e = 0;

		//The willingness to pay is in millions of dollars
		WillingnessToPaySubModel wtp_m;
		double wp = wtp_m.calculateResidentialWillingnessToPay(unit, household, wtp_e,day, model);
		{
			int unit_type = unit->getUnitType();

			UnitType *unitType = model->getUnitTypeById( unit_type );

			//(1-avg(wtp/hedonic)) * hedonic
			//We need to adjust the willingness to pay
			//wtpOffset is enabled by default. If you want to have wtpOffset as 0, set this value to false in the xml config file.
			bool wtpOffsetEnabled = config.ltParams.housingModel.wtpOffsetEnabled;
			if(wtpOffsetEnabled)
			{
				wp += wp * unitType->getWtpOffset();
			}
		}

		//wtp_e = wtp_e * entry->getAskingPrice(); //wtp error is a fraction of the asking price.

		wp += wtp_e; // adjusted willingness to pay in millions of dollars

		std::string oldPCStr = "empty";
		std::string newPCStr = "empty";

		if( oldPC )
			oldPCStr = oldPC->getSlaPostcode();

		if( newPC )
			newPCStr = newPC->getSlaPostcode();


		if( household->getAffordabilityAmount() > household->getCurrentUnitPrice() )
			maxAffordability = household->getAffordabilityAmount();
		else
			maxAffordability = household->getCurrentUnitPrice();

		wp = std::max(0.0, wp );

		double currentBid = 0;
		double currentSurplus = 0;

		if( entry->getAskingPrice() != 0 )
		{
			//tenure_status = 0 mean it is a BTO unit
			if( unit->isBto() )
			{
				currentBid = entry->getAskingPrice();
				currentSurplus = wp - entry->getAskingPrice();
			}
			else
			{
				computeBidValueLogistic( entry->getAskingPrice(), wp, currentBid, currentSurplus );
			}
		}
		else
			PrintOutV("Asking price is zero for unit " << entry->getUnitId() << std::endl );

		printHouseholdBiddingList( day, household->getId(), unit->getId(), oldPCStr, newPCStr, wp, entry->getAskingPrice(), maxAffordability, currentBid, currentSurplus);

		if( currentSurplus > maxSurplus && maxAffordability > currentBid  && currentSurplus > 0)
		{
			maxSurplus = currentSurplus;
			finalBid = currentBid;
			maxEntryUnitId = entry->getUnitId();
			maxWp = wp;
			maxWtpe = wtp_e;
		}
	}
	else
	{
		printError( (boost::format("[day %1%]Could not compute bid value for unit %2%. Eligibility: %3% Stats: %4%") % day % unit->getId() % flatEligibility % stats ).str() );
	}


}

bool HouseholdBidderRole::pickEntryToBid()
{
    const Household* household = getParent()->getHousehold();
	HousingMarket* market = getParent()->getMarket();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    HM_Model* model = getParent()->getModel();

    boost::gregorian::date simulationDate(HITS_SURVEY_YEAR, 1, 1);
    boost::gregorian::date_duration dt(day);
    simulationDate = simulationDate + dt;

    const double minUnitsInZoneHousingType = 2;

    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;

    market->getAvailableEntries(entries);

    BigSerial maxEntryUnitId = INVALID_ID;
    double maxSurplus = INT_MIN; // holds the wp of the entry with maximum surplus.
    double finalBid = 0;
    double maxWp	= 0;
    double maxWtpe  = 0;
    double maxAffordability = 0;
    bool isBTO = false;

    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
    float housingMarketSearchPercentage = config.ltParams.housingModel.housingMarketSearchPercentage;

    HouseHoldHitsSample *householdHits = model->getHouseHoldHitsById( household->getId() );

    std::vector<double>householdScreeningProbabilities;

    //We cannot use those probabilities because they are based on HITS2008 ids
    //model->getScreeningProbabilities(hitsId, householdScreeningProbabilities);
    //getScreeningProbabilities(household->getId(), householdScreeningProbabilities);

    ScreeningSubModel screeningSubmodel;
    screeningSubmodel.getScreeningProbabilities( household->getId(), householdScreeningProbabilities, model, day);

    if(householdScreeningProbabilities.size() > 0 )
    	printProbabilityList(household->getId(), householdScreeningProbabilities);

	std::set<const HousingMarket::Entry*> screenedEntries;
	std::vector<const HousingMarket::Entry*> screenedEntriesVec; //This vector's only purpose is to print the choiceset


	if(config.ltParams.housingModel.bidderUnitChoiceset.randomChoiceset == true)
	{
		while (screenedEntries.size() < config.ltParams.housingModel.bidderUnitChoiceset.bidderChoicesetSize)
		{
			double randomDraw = (double) rand() / RAND_MAX * entries.size();
			screenedEntries.insert(entries[randomDraw]);
		}
	}
	else
	if(config.ltParams.housingModel.bidderUnitChoiceset.shanLopezChoiceset == true)
	{
		for (int n = 0; n < entries.size() && screenedEntries.size() < config.ltParams.housingModel.bidderUnitChoiceset.bidderChoicesetSize; n++)
		{
			double randomDraw = (double) rand() / RAND_MAX;
			int zoneHousingType = -1;
			double cummulativeProbability = 0.0;
			for (int m = 0; m < householdScreeningProbabilities.size(); m++)
			{
				cummulativeProbability += householdScreeningProbabilities[m];
				if (cummulativeProbability > randomDraw)
				{
					zoneHousingType = m + 1; //housing type is a one-based index
					break;
				}
			}


			auto range = market->getunitsByZoneHousingType().equal_range(zoneHousingType);
			int numUnits = distance(range.first, range.second); //find the number of units in the above zoneHousingType

			if (numUnits < minUnitsInZoneHousingType)
				continue;


			if (numUnits == 0)
				continue;

			int offset = (float) rand() / RAND_MAX * (numUnits - 1);
			advance(range.first, offset); // change a random unit in that zoneHousingType

			const BigSerial unitId = (range.first)->second;


			const HousingMarket::Entry *entry = market->getEntryById(unitId);

			if (entry == nullptr || entry->isBuySellIntervalCompleted() == false)
				continue;


			const Unit *thisUnit = model->getUnitById(entry->getUnitId());


			if (thisUnit->getZoneHousingType() == zoneHousingType)
			{

				if (thisUnit->getTenureStatus() == 2 && getParent()->getFutureTransitionOwn() == false) //rented
				{
					std::set<const HousingMarket::Entry *>::iterator screenedEntriesItr;
					screenedEntriesItr = std::find(screenedEntries.begin(), screenedEntries.end(), entry);

					if (screenedEntriesItr == screenedEntries.end())
						screenedEntries.insert(entry);
				}
				else if (thisUnit->getTenureStatus() == 1) //owner-occupied
				{
					std::set<const HousingMarket::Entry *>::iterator screenedEntriesItr;
					screenedEntriesItr = std::find(screenedEntries.begin(), screenedEntries.end(), entry);

					if (screenedEntriesItr == screenedEntries.end())
						screenedEntries.insert(entry);
				}
			}
		}
	}

    {

    	for(auto itr = screenedEntries.begin(); itr != screenedEntries.end(); itr++)
    		screenedEntriesVec.push_back(*itr);


    	//btoEntries will contain pointers all the units in our 'units' vector that are marked as BTOs.
    	set<BigSerial> btoEntries = market->getBTOEntries();

        //Add x number of BTO units to the screenedUnit vector if the household is eligible for it
        for(int n = 0; n < config.ltParams.housingModel.bidderUnitChoiceset.bidderBTOChoicesetSize && btoEntries.size() != 0; n++)
        {
        	int offset = (float)rand() / RAND_MAX * ( btoEntries.size() - 1 );

        	auto itr =  btoEntries.begin();
         	std::advance( itr, offset);

         	const HousingMarket::Entry* entry = market->getEntryById(*itr);

        	screenedEntries.insert(entry);
        	screenedEntriesVec.push_back(entry);

        	btoEntries.erase(*itr);
        }

    	std::string choiceset(" ");
    	for(int n = 0; n < screenedEntriesVec.size(); n++)
    	{
    		printChoiceset2(day-1, household->getId(),screenedEntriesVec[n]->getUnitId());
    		choiceset += std::to_string( screenedEntriesVec[n]->getUnitId() )  + ", ";
    	}

    	printChoiceset(day-1, household->getId(), choiceset);
    }

    //PrintOutV("Screening  entries is now: " << screenedEntries.size() << std::endl );

    // Choose the unit to bid with max surplus. However, we are not iterating through the whole list of available units.
    // We choose from a subset of units set by the housingMarketSearchPercentage parameter in the long term XML file.
    // This is done to replicate the real life scenario where a household will only visit a certain percentage of vacant units before settling on one.
    for(int n = 0; n < screenedEntries.size(); n++)
    {
    	auto itr = screenedEntries.begin();
    	advance(itr, n);

        const HousingMarket::Entry* entry = *itr;

        if( entry->getAskingPrice() < 0.01 )
        {
        	printError( (boost::format( "[unit %1%] Asking price is suspiciously low at %2%.") % entry->getUnitId() % entry->getAskingPrice() ).str());
        }

        calculateMaxSurplusEntry(entry,maxSurplus,finalBid,maxWp,maxAffordability,maxWtpe,maxEntryUnitId);

    }



    //calculate surplus of your own unit and compare with the screened entries.
       {
       	BigSerial uid = household->getUnitId();
       	const HousingMarket::Entry *curEntry = market->getEntryById( uid );

       	if(curEntry != nullptr)
       		calculateMaxSurplusEntry(curEntry,maxSurplus,finalBid,maxWp,maxAffordability,maxWtpe,maxEntryUnitId);
       }

    biddingEntry = CurrentBiddingEntry(maxEntryUnitId, finalBid, maxWp, maxSurplus, maxWtpe, maxAffordability );
    return biddingEntry.isValid();
}



void HouseholdBidderRole::computeBidValueLogistic( double price, double wp, double &finalBid, double &finalSurplus )
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	const double sigma = 1.0;
	const double mu    = 0.0;

	double lowerBound = -5.0;
	double upperBound =  5.0;
	double a = config.ltParams.housingModel.hedonicPriceModel.a;
	double b = config.ltParams.housingModel.hedonicPriceModel.b;
	double w = wp / price;
	const int MAX_ITERATIONS = 50;


	double increment = (upperBound - lowerBound) / MAX_ITERATIONS;
	double m = lowerBound;

	double  expectedSurplusMax = INT_MIN;
	double incrementScaledMax  = 0;

	for (int n = 0; n <= MAX_ITERATIONS; n++ )
	{
		double incrementScaled = ( m - lowerBound ) * ( b - a ) / (upperBound - lowerBound ) + a;

		double Fx   = 1.0 / (1.0 + exp(-( m - mu ) / sigma ) );

		double expectedSurplus =  Fx * ( w - incrementScaled );

		if( expectedSurplus > expectedSurplusMax )
		{
			expectedSurplusMax = expectedSurplus;
			incrementScaledMax = incrementScaled;
		}

		m += increment;


	}

	if( incrementScaledMax < 0.8 || incrementScaledMax > 1.15)
		cout << " incremenScaled " << incrementScaledMax << endl;

	finalBid     = price * incrementScaledMax;
	finalSurplus = ( w - incrementScaledMax ) * price;
}

void HouseholdBidderRole::setMoveInWaitingTimeInDays(int days)
{
	this->moveInWaitingTimeInDays = days;
}

void HouseholdBidderRole::setUnitIdToBeOwned(BigSerial unitId)
{
	this->unitIdToBeOwned = unitId;
}
