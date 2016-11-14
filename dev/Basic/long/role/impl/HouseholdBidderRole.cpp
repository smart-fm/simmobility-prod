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
		int index =	day / quarter;
		double interestRate = (*interestRateListX)[index]->getInterestRate() / 100 / 12.0; // divide by 12 to get the monthly interest rate.

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

	double price = expectations[0].hedonicPrice;

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

		//Just before we set the bidderRole to inactive, we do the unit ownership switch.
		if( moveInWaitingTimeInDays == 1 )
		{
			TakeUnitOwnership();
		}

		moveInWaitingTimeInDays--;

		return;
	}

	//wait x days after move in to a new unit to reconsider the vehicle ownership option.
	if( vehicleBuyingWaitingTimeInDays > 0 && moveInWaitingTimeInDays == 0)
	{

		if( vehicleBuyingWaitingTimeInDays == 1)
		{
			TimeCheck vehicleOwnershipTiming;

			VehicleOwnershipModel vehOwnershipModel(getParent()->getModel());
			vehOwnershipModel.reconsiderVehicleOwnershipOption(getParent()->getHousehold(),getParent(), day);

			double vehicleOwnershipTime = vehicleOwnershipTiming.getClockTime();

			#ifdef VERBOSE_SUBMODEL_TIMING
				PrintOutV("vehicleOwnership time for agent " << getParent()->getId() << " is " << vehicleOwnershipTime << std::endl );
			#endif
		}
			vehicleBuyingWaitingTimeInDays--;
	}

    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms())
    {
        bidOnCurrentDay = false;
    }

    if (isActive())
    {
    	getParent()->getModel()->incrementNumberOfBidders();

        if (!waitingForResponse && !bidOnCurrentDay && bidUnit(now))
        {
            waitingForResponse = true;
            bidOnCurrentDay = true;
        }
    }

    lastTime = now;
}

void HouseholdBidderRole::TakeUnitOwnership()
{
	#ifdef VERBOSE
	PrintOutV("[day " << day << "] Household " << getParent()->getId() << " is moving into unit " << unitIdToBeOwned << " today." << std::endl);
	#endif
	getParent()->addUnitId( unitIdToBeOwned );

	boost::shared_ptr<Household> houseHold = boost::make_shared<Household>( *getParent()->getHousehold());
	houseHold->setUnitId(unitIdToBeOwned);
	houseHold->setHasMoved(1);
	houseHold->setUnitPending(0);
	houseHold->setMoveInDate(getDateBySimDay(year,day));
	HM_Model* model = getParent()->getModel();
	model->addHouseholdsTo_OPSchema(houseHold);

    biddingEntry.invalidate();
    Statistics::increment(Statistics::N_ACCEPTED_BIDS);
}


void HouseholdBidderRole::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
        case LTMID_BID_RSP:// Bid response received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            switch (msg.getResponse())
            {
                case ACCEPTED:// Bid accepted 
                {
                	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

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


                	vehicleBuyingWaitingTimeInDays = config.ltParams.vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays;
                	int simulationEndDay = config.ltParams.days;
                	year = config.ltParams.year;
                	getParent()->getHousehold()->setLastBidStatus(1);

                	if(simulationEndDay < (moveInWaitingTimeInDays))

                	{
                		boost::shared_ptr<Household> houseHold = boost::make_shared<Household>( *getParent()->getHousehold());
                		houseHold->setUnitId(unitIdToBeOwned);
                		houseHold->setHasMoved(0);
                		houseHold->setUnitPending(1);
                		int awakenDay = getParent()->getAwakeningDay();
                		houseHold->setAwakenedDay(awakenDay);
                		houseHold->setMoveInDate(getDateBySimDay(year,moveInWaitingTimeInDays));
                		HM_Model* model = getParent()->getModel();
                		model->addHouseholdsTo_OPSchema(houseHold);

                		getParent()->setAcceptedBid(true);
                	}

                    break;
                }
                case NOT_ACCEPTED:
                {
                    biddingEntry.incrementTries();
                    getParent()->getHousehold()->setLastBidStatus(2);
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

				Bid newBid(model->getBidId(),household->getUnitId(),entry->getUnitId(), household->getId(), getParent(), biddingEntry.getBestBid(), now.ms(), biddingEntry.getWP(), biddingEntry.getWtp_e(), biddingEntry.getAffordability());
				bid(entry->getOwner(), newBid);

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

				model->incrementBids();
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


bool HouseholdBidderRole::pickEntryToBid()
{
    const Household* household = getParent()->getHousehold();
    HousingMarket* market = getParent()->getMarket();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    HM_Model* model = getParent()->getModel();

    boost::gregorian::date simulationDate(HITS_SURVEY_YEAR, 1, 1);
    boost::gregorian::date_duration dt(day);
    simulationDate = simulationDate + dt;

    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;

    market->getAvailableEntries(entries);

    const HousingMarket::Entry* maxEntry = nullptr;
    double maxSurplus = 0; // holds the wp of the entry with maximum surplus.
    double finalBid = 0;
    double maxWp	= 0;
    double maxWtpe  = 0;
    double maxAffordability = 0;
    bool isBTO = false;

    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
    float housingMarketSearchPercentage = config.ltParams.housingModel.housingMarketSearchPercentage;

    HouseHoldHitsSample *householdHits = model->getHouseHoldHitsById( household->getId() );
    std::string hitsId = householdHits->getHouseholdHitsId();

    std::vector<double>householdScreeningProbabilities;

    //We cannot use those probabilities because they are based on HITS2008 ids
    //model->getScreeningProbabilities(hitsId, householdScreeningProbabilities);
    //getScreeningProbabilities(household->getId(), householdScreeningProbabilities);

    ScreeningSubModel screeningSubmodel;
    screeningSubmodel.getScreeningProbabilities( household->getId(), householdScreeningProbabilities, model, day);

    //PrintOutV(" size: " << householdScreeningProbabilities.size() << std::endl);

    if(householdScreeningProbabilities.size() > 0 )
    	printProbabilityList(household->getId(), householdScreeningProbabilities);

	std::vector<const HousingMarket::Entry*> screenedEntries;

    for(int n = 0; n < entries.size() /** housingMarketSearchPercentage*/ && screenedEntries.size() < config.ltParams.housingModel.bidderUnitsChoiceSet; n++)
    {
        double randomDraw = (double)rand()/RAND_MAX;
        int zoneHousingType = -1;
        double cummulativeProbability = 0.0;
        for( int m = 0; m < householdScreeningProbabilities.size(); m++ )
        {
        	cummulativeProbability +=  householdScreeningProbabilities[m];
        	if( cummulativeProbability > randomDraw )
        	{
        		zoneHousingType = m + 1; //housing type is a one-based index
        		break;
        	}
        }

      	int offset = (float)rand() / RAND_MAX * ( entries.size() - 1 );

    	HousingMarket::ConstEntryList::const_iterator itr = entries.begin() + offset;
    	const HousingMarket::Entry* entry = *itr;

    	//std::multimap<BigSerial, Unit*>  unitByZHT = model->getUnitsByZoneHousingType();

        const Unit* thisUnit = model->getUnitById( entry->getUnitId() );

        if( thisUnit->getZoneHousingType() == zoneHousingType )
        {
			if( thisUnit->getTenureStatus() == 2 && getParent()->getFutureTransitionOwn() == false ) //rented
			{
				std::vector<const HousingMarket::Entry*>::iterator screenedEntriesItr;
				screenedEntriesItr = std::find(screenedEntries.begin(), screenedEntries.end(), entry );

				if( screenedEntriesItr == screenedEntries.end() )
					screenedEntries.push_back(entry);
			}
			else
			if( thisUnit->getTenureStatus() == 1) //owner-occupied
			{
				std::vector<const HousingMarket::Entry*>::iterator screenedEntriesItr;
				screenedEntriesItr = std::find(screenedEntries.begin(), screenedEntries.end(), entry );

				if( screenedEntriesItr == screenedEntries.end() )
					screenedEntries.push_back(entry);
			}
        }
    }

    bool sucessfulScreening = true;
    if( screenedEntries.size() == 0 )
    {
    	sucessfulScreening = false;
    	screenedEntries = entries;
    }
    else
    {
        //Add x BTO units to the screenedUnit vector if the household is eligible for it
        for(int n = 0, m = 0; n < entries.size() && m < config.ltParams.housingModel.bidderBTOUnitsChoiceSet; n++ )
        {
        	int offset = (float)rand() / RAND_MAX * ( entries.size() - 1 );

         	HousingMarket::ConstEntryList::const_iterator itr = entries.begin() + offset;
           	const HousingMarket::Entry* entry = *itr;

        	if( entry->isBTO() == true )
        	{
        		screenedEntries.push_back(entry);
        		m++;
        	}
        }

    	std::string choiceset(" ");
    	for(int n = 0; n < screenedEntries.size(); n++)
    	{
    		choiceset += std::to_string( screenedEntries[n]->getUnitId() )  + ", ";
    	}

    	printChoiceset(household->getId(), choiceset);
    }

   //PrintOutV("Screening  entries is now: " << screenedEntries.size() << std::endl );

    // Choose the unit to bid with max surplus. However, we are not iterating through the whole list of available units.
    // We choose from a subset of units set by the housingMarketSearchPercentage parameter in the long term XML file.
    // This is done to replicate the real life scenario where a household will only visit a certain percentage of vacant units before settling on one.
    for(int n = 0; n < screenedEntries.size(); n++)
    {
    	int offset = (float)rand() / RAND_MAX * ( entries.size() - 1 );

    	//if we have a good choiceset, let's iterate linearly
    	if(sucessfulScreening == true)
    		offset = n;

    	if( n > config.ltParams.housingModel.bidderUnitsChoiceSet)
    		break;

    	HousingMarket::ConstEntryList::const_iterator itr = screenedEntries.begin() + offset;
        const HousingMarket::Entry* entry = *itr;

        if( entry->getAskingPrice() < 0.01 )
        {
        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "[unit %1%] Asking price is suspiciously low at %2%.") % entry->getUnitId() % entry->getAskingPrice() ).str());
        }

        if(entry && entry->getOwner() != getParent() )
        {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

            bool flatEligibility = true;

 			if( unit->isBto() && unit->getUnitType() == 2 && household->getTwoRoomHdbEligibility()  == false )
				flatEligibility = false;

			if( unit->isBto() && unit->getUnitType() == 3 && household->getThreeRoomHdbEligibility() == false )
				flatEligibility = false;

			if( unit->isBto() && unit->getUnitType() == 4 && household->getFourRoomHdbEligibility() == false )
				flatEligibility = false;


            if( stats && flatEligibility )
            {
            	const Unit *hhUnit = model->getUnitById( household->getUnitId() );

            	BigSerial postcodeCurrent = 0;
            	if( hhUnit != NULL )
            		postcodeCurrent = hhUnit->getSlaAddressId();

            	Postcode *oldPC = model->getPostcodeById(postcodeCurrent);
            	Postcode *newPC = model->getPostcodeById(unit->getSlaAddressId());

               //double wp_old = luaModel.calulateWP(*household, *unit, *stats);
            	double wtp_e = 0;

            	//The willingness to pay is in millions of dollars
            	WillingnessToPaySubModel wtp_m;
            	double wp = wtp_m.CalculateWillingnessToPay(unit, household, wtp_e,day, model);

            	wtp_e = wtp_e * entry->getAskingPrice(); //wtp error is a fraction of the asking price.

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
            		computeBidValueLogistic( entry->getAskingPrice(), wp, currentBid, currentSurplus );
            	else
            		PrintOutV("Asking price is zero for unit " << entry->getUnitId() << std::endl );

                printHouseholdBiddingList( day, household->getId(), unit->getId(), oldPCStr, newPCStr, wp, entry->getAskingPrice(), maxAffordability, currentBid, currentSurplus);

            	if( currentSurplus > maxSurplus && maxAffordability > entry->getAskingPrice() )
            	{
            		maxSurplus = currentSurplus;
            		finalBid = currentBid;
            		maxEntry = entry;
            		maxWp = wp;
            		maxWtpe = wtp_e;
            	}
            }
        }
    }

    if( maxEntry && model->getUnitById(maxEntry->getUnitId())->isBto() )
    {
    	//When bidding on BTO units, we cannot bid above the asking price. So it's basically the ceiling we cannot exceed.
    	finalBid = maxEntry->getAskingPrice();
    }

    biddingEntry = CurrentBiddingEntry( (maxEntry) ? maxEntry->getUnitId() : INVALID_ID, finalBid, maxWp, maxSurplus, maxWtpe, maxAffordability );
    return biddingEntry.isValid();
}



void HouseholdBidderRole::computeBidValueLogistic( double price, double wp, double &finalBid, double &finalSurplus )
{
	const double sigma = 1.0;
	const double mu    = 0.0;

	double lowerBound = -5.0;
	double upperBound =  5.0;
	double a = 0.6;
	double b = 1.05;
	double w = wp / price;
	const int MAX_ITERATIONS = 50;


	double increment = (upperBound - lowerBound) / MAX_ITERATIONS;
	double m = lowerBound;

	double  expectedSurplusMax = 0;
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
