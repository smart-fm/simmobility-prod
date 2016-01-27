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

    inline void writeVehicleOwnershipToFile(BigSerial hhId,int VehiclOwnershiOptionId)
    {
    	boost::format fmtr = boost::format("%1%, %2%") % hhId % VehiclOwnershiOptionId;
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_VEHICLE_OWNERSIP,fmtr.str());

    }

    inline void printHouseholdGroupLogsum( int homeTaz,  int group, BigSerial hhId, double logsum )
    {
    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%") % homeTaz % group % hhId % logsum;
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDGROUPLOGSUM,fmtr.str());
    }

    inline void printHouseholdBiddingList( int day, BigSerial householdId, BigSerial unitId, std::string postcodeCurrent, std::string postcodeNew, float wp  )
    {
    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%")% day % householdId % unitId % postcodeCurrent % postcodeNew % wp;
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDBIDLIST,fmtr.str());
    }

    inline void printProbabilityList( BigSerial householdId, std::vector<double>probabilities )
    {
    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25%, %26%, %27%, %28%, %29%, %30%, %31%, %32%, %33%, %34%, %35%, %36%, %37%, %38%, %39%, %40%, %41%, %42%, "
											"%43%, %44%, %45%, %46%, %47%, %48%, %49%, %50%, %51%, %52%, %53%, %54%, %55%, %56%, %57%, %58%, %59%, %60%, %61%, %62%, %63%, %64%, %65%, %66%, %67%, %68%, %69%, %70%, %71%, %72%, %73%, %74%, %75%, %76%, %77%, %78%, %79%, %80%, %81%, %82%, "
											"%83%, %84%, %85%, %86%, %87%, %88%, %89%, %90%, %91%, %92%, %93%, %94%, %95%, %96%, %97%, %98%, %99%, %100%, %101%, %102%, %103%, %104%, %105%, %106%, %107%, %108%, %109%, %110%, %111%, %112%, %113%, %114%, %115%, %116%, %117%, %118%, "
											"%119%, %120%, %121%, %122%, %123%, %124%, %125%, %126%, %127%, %128%, %129%, %130%, %131%, %132%, %133%, %134%, %135%, %136%, %137%, %138%, %139%, %140%, %141%, %142%, %143%, %144%, %145%, %146%, %147%, %148%, %149%, %150%, %151%, "
											"%152%, %153%, %154%, %155%, %156%, %157%, %158%, %159%, %160%, %161%, %162%, %163%, %164%, %165%, %166%, %167%, %168%, %169%, %170%, %171%, %172%, %173%, %174%, %175%, %176%, %177%, %178%, %179%, %180%, %181%, %182%, %183%, %184%, "
											"%185%, %186%, %187%, %188%, %189%, %190%, %191%, %192%, %193%, %194%, %195%, %196%, %197%, %198%, %199%, %200%, %201%"
											)% householdId % probabilities[0]  % probabilities[1]  % probabilities[2]  % probabilities[3]  % probabilities[4]  % probabilities[5]  % probabilities[6]  % probabilities[7]  % probabilities[8]  % probabilities[9]  % probabilities[10]  % probabilities[11]  % probabilities[12]  % probabilities[13]
											% probabilities[14]  % probabilities[15]  % probabilities[16]  % probabilities[17]  % probabilities[18]  % probabilities[19]  % probabilities[20]  % probabilities[21]  % probabilities[22]  % probabilities[23]  % probabilities[24]  % probabilities[25]  % probabilities[26]  % probabilities[27]
											% probabilities[28]  % probabilities[29]  % probabilities[30]  % probabilities[31]  % probabilities[32]  % probabilities[33]  % probabilities[34]  % probabilities[35]  % probabilities[36]  % probabilities[37]  % probabilities[38]  % probabilities[39]  % probabilities[40]  % probabilities[41]
											% probabilities[42]  % probabilities[43]  % probabilities[44]  % probabilities[45]  % probabilities[46]  % probabilities[47]  % probabilities[48]  % probabilities[49]  % probabilities[50]  % probabilities[51]  % probabilities[52]  % probabilities[53]  % probabilities[54]  % probabilities[55]
											% probabilities[56]  % probabilities[57]  % probabilities[58]  % probabilities[59]  % probabilities[60]  % probabilities[61]  % probabilities[62]  % probabilities[63]  % probabilities[64]  % probabilities[65]  % probabilities[66]  % probabilities[67]  % probabilities[68]  % probabilities[69]
											% probabilities[70]  % probabilities[71]  % probabilities[72]  % probabilities[73]  % probabilities[74]  % probabilities[75]  % probabilities[76]  % probabilities[77]  % probabilities[78]  % probabilities[79]  % probabilities[80]  % probabilities[81]  % probabilities[82]  % probabilities[83]
											% probabilities[84]  % probabilities[85]  % probabilities[86]  % probabilities[87]  % probabilities[88]  % probabilities[89]  % probabilities[90]  % probabilities[91]  % probabilities[92]  % probabilities[93]  % probabilities[94]  % probabilities[95]  % probabilities[96]  % probabilities[97]
											% probabilities[98]  % probabilities[99]  % probabilities[100]  % probabilities[101]  % probabilities[102]  % probabilities[103]  % probabilities[104]  % probabilities[105]  % probabilities[106]  % probabilities[107]  % probabilities[108]  % probabilities[109]  % probabilities[110]
											% probabilities[111]  % probabilities[112]  % probabilities[113]  % probabilities[114]  % probabilities[115]  % probabilities[116]  % probabilities[117]  % probabilities[118]  % probabilities[119]  % probabilities[120]  % probabilities[121]  % probabilities[122]  % probabilities[123]
											% probabilities[124]  % probabilities[125]  % probabilities[126]  % probabilities[127]  % probabilities[128]  % probabilities[129]  % probabilities[130]  % probabilities[131]  % probabilities[132]  % probabilities[133]  % probabilities[134]  % probabilities[135]  % probabilities[136]
											% probabilities[137]  % probabilities[138]  % probabilities[139]  % probabilities[140]  % probabilities[141]  % probabilities[142]  % probabilities[143]  % probabilities[144]  % probabilities[145]  % probabilities[146]  % probabilities[147]  % probabilities[148]  % probabilities[149]
											% probabilities[150]  % probabilities[151]  % probabilities[152]  % probabilities[153]  % probabilities[154]  % probabilities[155]  % probabilities[156]  % probabilities[157]  % probabilities[158]  % probabilities[159]  % probabilities[160]  % probabilities[161]  % probabilities[162]
											% probabilities[163]  % probabilities[164]  % probabilities[165]  % probabilities[166]  % probabilities[167]  % probabilities[168]  % probabilities[169]  % probabilities[170]  % probabilities[171]  % probabilities[172]  % probabilities[173]  % probabilities[174]  % probabilities[175]
											% probabilities[176]  % probabilities[177]  % probabilities[178]  % probabilities[179]  % probabilities[180]  % probabilities[181]  % probabilities[182]  % probabilities[183]  % probabilities[184]  % probabilities[185]  % probabilities[186]  % probabilities[187]  % probabilities[188]
											% probabilities[189]  % probabilities[190]  % probabilities[191]  % probabilities[192]  % probabilities[193]  % probabilities[194]  % probabilities[195]  % probabilities[196]  % probabilities[197]  % probabilities[198]  % probabilities[199];

    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCREENINGPROBABILITIES,fmtr.str());
    }

    inline void printChoiceset( BigSerial householdId, std::string choiceset)
       {
       	boost::format fmtr = boost::format("%1%, %2% ")% householdId % choiceset;

       	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HHCHOICESET,fmtr.str());
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
																  moveInWaitingTimeInDays(0),vehicleBuyingWaitingTimeInDays(0), day(day), initBidderRole(true),year(0),bidComplete(true){}

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
		double loanTenure = ( retirementAge - ( HITS_SURVEY_YEAR - ( 1900 + dob.tm_year ) ) ) * 12.0; //times 12 to get he tenure in months, not years.

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

	//
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
		if(bidComplete)
		{
			bidComplete = false;
			getParent()->getModel()->decrementBidders();
		}

		//Just before we set the bidderRole to inactive, we do the unit ownership switch.
		if( moveInWaitingTimeInDays == 1 )
		{
			TakeUnitOwnership();
		}

		moveInWaitingTimeInDays--;

		return;
	}

	//wait x days after move in to a new unit to reconsider the vehicle ownership option.
//	if( vehicleBuyingWaitingTimeInDays > 0 && moveInWaitingTimeInDays == 0)
//	{
//
//		if( vehicleBuyingWaitingTimeInDays == 1)
//		{
//			TimeCheck vehicleOwnershipTiming;
//
//			reconsiderVehicleOwnershipOption();
//
//			double vehicleOwnershipTime = vehicleOwnershipTiming.getClockTime();
//
//			#ifdef VERBOSE_SUBMODEL_TIMING
//				PrintOutV("vehicleOwnership time for agent " << getParent()->getId() << " is " << vehicleOwnershipTime << std::endl );
//			#endif
//		}
//			vehicleBuyingWaitingTimeInDays--;
//	}

    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms())
    {
        bidOnCurrentDay = false;
    }

    if (isActive())
    {
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
                	moveInWaitingTimeInDays = config.ltParams.housingModel.housingMoveInDaysInterval;
                	unitIdToBeOwned = msg.getBid().getNewUnitId();
                	vehicleBuyingWaitingTimeInDays = config.ltParams.vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays;
                	int simulationEndDay = config.ltParams.days;
                	year = config.ltParams.year;
                	if(simulationEndDay < (vehicleBuyingWaitingTimeInDays+day))
                	{
                		boost::shared_ptr<Household> houseHold = boost::make_shared<Household>( *getParent()->getHousehold());
                		houseHold->setUnitId(unitIdToBeOwned);
                		houseHold->setHasMoved(0);
                		houseHold->setMoveInDate(getDateBySimDay(year,(day+moveInWaitingTimeInDays)));
                		HM_Model* model = getParent()->getModel();
                		model->addHouseholdsTo_OPSchema(houseHold);
                	}

                    break;
                }
                case NOT_ACCEPTED:
                {
                    biddingEntry.incrementTries();
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

				bid(entry->getOwner(), Bid(model->getBidId(),household->getUnitId(),entry->getUnitId(), household->getId(), getParent(), biddingEntry.getBestBid(), now.ms(), biddingEntry.getWP(), biddingEntry.getWtp_e(), biddingEntry.getAffordability()));
				model->incrementBids();
				return true;
			}
		}
    }
    return false;
}

double HouseholdBidderRole::calculateWillingnessToPay(const Unit* unit, const Household* household, double& wtp_e)
{
	double V;

	//
	//These constants are extracted from Roberto Ponce's bidding model
	//
	/* willingness to pay in million of dollars*/
	double sde		=  0.1;
	double barea	=  0.6922591951;
	double blogsum	=  0.0184661069;
	double bchin	= -0.0727597459;
	double bmalay	= -0.3067308978;
	double bHighInc =  0.0558738418;
	const double bHIncChildApart  	=  0.085270578;
	const double bHIncChildCondo  	= -0.0496929496;
	const double bapartment 		= -3.1147976249;
	const double bcondo 			= -2.9582377947;
	const double bdetachedAndSemiDetached = -2.6753868759;
	const double terrace 			= -2.9801756451;
	const double bageOfUnit25		= -0.0432841653;
	const double bageOfUnit25Squared= -0.0164360119;
	const double bageGreaterT25LessT50 =  0.1883170202;
	const double bageGreaterT50 	=  0.3565907423;
	const double bmissingAge 		= -0.1679748285;
	const double bfreeholdAppartm 	=  0.599136353;
	const double bfreeholdCondo 	=  0.4300148333;
	const double fbreeholdTerrace 	=  0.3999045196;

	const double midIncChildHDB3 = -0.0044485643;
	const double midIncChildHDB4 = -0.0068614137;
	const double midIncChildHDB5 = -0.0090473027;

	const double bhdb12	=	-3.7770973415;
	const double bhdb3  =  	-3.4905971667;
	const double bhdb4 	=	-3.4851295051;
	const double bhdb5	=	-3.5070548459;
	const double bageOfUnit30 = -0.7012864149;
	const double bageOfUnit30Squared = 0.1939266362;
	const double bageOfUnitGreater30 = 0.0521622428;

	const PostcodeAmenities* pcAmenities = DataManagerSingleton::getInstance().getAmenitiesById( unit->getSlaAddressId() );

	double Apartment	= 0;
	double Condo		= 0;
	double DetachedAndSemidetaced	= 0;
	double Terrace		= 0;
	double HDB12 		= 0;
	double HDB3			= 0;
	double HDB4			= 0;
	double HDB5			= 0;
	double HH_size1		= 0;
	double HH_size2		= 0;
	double HH_size3m	= 0;
	double DD_area		= 0;
	double ZZ_logsumhh	=-1;
	double ZZ_hhchinese = 0;
	double ZZ_hhmalay	= 0;
	double ZZ_hhindian	= 0;
	double ZZ_hhinc		= 0;
	double ZZ_hhsize	= 0;

	int unitType = unit->getUnitType();

	if( unitType == ID_HDB1 || unitType == ID_HDB2 )
		HDB12 = 1;
	else
	if( unitType == ID_HDB3 )
		HDB3 = 1;
	else
	if( unitType == ID_HDB4 )
		HDB4 = 1;
	else
	if( unitType == ID_HDB5 )
		HDB5 = 1;
	else
	if( unitType >= ID_APARTM70 && unitType <= ID_APARTM159 )
		Apartment = 1;
	else
	if( unitType >= ID_CONDO60 && unitType <= ID_CONDO134 )
		Condo = 1;
	else
	if( unitType >= ID_TERRACE180 && unitType <= ID_TERRACE379 )
		Terrace = 1;
	else
	if( unitType >= ID_SEMID230 && unitType <= ID_DETACHED1199 )
		DetachedAndSemidetaced = 1;
	else
	if( unitType == 6 )
		HDB5 = 1;
	else
	if( unitType >= 32 && unitType <= 51 )
		Condo = 1;
	else
	if( unitType == 64 )
		Apartment = 1;
	else
	if( unitType == 65 )
		HDB5 = 1;
	else
		return 0.0;


	if( unitType <= 6  || unitType == 65 )
	{
		sde 	 = 0.05;
		barea 	 = 0.8095874824;
		blogsum	 = 0.0035517989;
		bchin 	 = 0.0555546991;
		bmalay 	 = -0.0056135472;
		bHighInc = 0.0229342784;
	}

	if( household->getSize() == 1)
		HH_size1 = 1;
	else
	if( household->getSize() == 2)
		HH_size2 = 1;
	else
		HH_size3m = 1;

	DD_area = log( unit->getFloorArea() );

	BigSerial homeTaz = 0;
	BigSerial workTaz = 0;
	Individual* headOfHousehold = NULL;

	std::vector<BigSerial> householdOccupants = household->getIndividuals();

	for( int n = 0; n < householdOccupants.size(); n++ )
	{
		Individual * householdIndividual = getParent()->getModel()->getIndividualById( householdOccupants[n] );

		if( householdIndividual->getHouseholdHead() )
		{
			headOfHousehold = householdIndividual;
		}
	}

	//This household does not seem to have an head of household, let's just assign one.
	if(headOfHousehold == NULL)
	{
		int eldestHouseholdMemberAge = 0;
		for( int n = 0; n < householdOccupants.size(); n++ )
		{
			Individual * householdIndividual = getParent()->getModel()->getIndividualById( householdOccupants[n] );
			std::tm dob = householdIndividual->getDateOfBirth();

			int age = HITS_SURVEY_YEAR  - 1900 + ( day / 365 ) - dob.tm_year;

			if( age > eldestHouseholdMemberAge )
			{
				age =  eldestHouseholdMemberAge;
				headOfHousehold = householdIndividual;
			}
		}
	}

	const int ageOfUnitPrivate = HITS_SURVEY_YEAR  - 1900 + ( day / 365 ) - unit->getPhysicalFromDate().tm_year;


	double ZZ_ageOfUnitPrivate	 = ageOfUnitPrivate;
	int ZZ_ageBet25And50 = 0;
	int ZZ_ageGreater50  = 0;
	int ZZ_missingAge    = 0;
	int ZZ_freehold 	 = 0;

	if( ageOfUnitPrivate > 25 )
		ZZ_ageOfUnitPrivate = 25;

	if( ageOfUnitPrivate < 0 )
		ZZ_ageOfUnitPrivate = 0;

	ZZ_ageOfUnitPrivate = ZZ_ageOfUnitPrivate / 10.0;

	if( ageOfUnitPrivate > 25 && ageOfUnitPrivate < 50)
		ZZ_ageBet25And50 = 1;

	if( ageOfUnitPrivate > 50 )
		ZZ_ageGreater50 = 1;


	const int ageOfUnitHDB = HITS_SURVEY_YEAR - 1900 + ( day / 365 ) - unit->getPhysicalFromDate().tm_year;
	double ZZ_ageOfUnitHDB	 = ageOfUnitHDB;
	int ZZ_ageGreater30  = 0;

	if( ageOfUnitHDB > 30 )
		ZZ_ageOfUnitHDB = 30;

	if( ageOfUnitHDB  < 0 )
		ZZ_ageOfUnitHDB = 0;

	ZZ_ageOfUnitHDB = ZZ_ageOfUnitHDB / 10.0;

	if( ageOfUnitHDB > 30 )
		ZZ_ageGreater30 = 1;


	HM_Model *model = getParent()->getModel();
	Job *job = model->getJobById(headOfHousehold->getJobId());

	BigSerial hometazId = model->getUnitTazId( household->getUnitId() );
	Taz *homeTazObj = model->getTazById( hometazId );

	std::string homeTazStr;
	if( homeTazObj != NULL )
		homeTazStr = homeTazObj->getName();

	homeTaz = std::atoi( homeTazStr.c_str() );

	BigSerial worktazId = model->getEstablishmentTazId( job->getEstablishmentId() );
	Taz *workTazObj = model->getTazById( worktazId );

	std::string workTazStr;
	if( workTazObj != NULL )
		workTazStr =  workTazObj->getName();

	workTaz = std::atoi( workTazStr.c_str());

	if( workTazStr.size() == 0 )
	{
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "workTaz is empty for person:  %1%.") %  headOfHousehold->getId()).str());
		workTaz = homeTaz;
	}

	if( homeTazStr.size() == 0 )
	{
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "homeTaz is empty for person:  %1%.") %  headOfHousehold->getId()).str());
		homeTaz = -1;
		workTaz = -1;
	}

	if( homeTaz == -1 || workTaz == -1 )
	{
		ZZ_logsumhh = 0;
		return 0;
	}
	else
	{
		HouseHoldHitsSample *hitssample = model->getHouseHoldHitsById( household->getId() );

		for(int n = 0; n < model->householdGroupVec.size(); n++ )
		{
			BigSerial thisGroupId = model->householdGroupVec[n].getGroupId();
			BigSerial thisHomeTaz = model->householdGroupVec[n].getHomeTaz();

			if( thisGroupId == hitssample->getGroupId() &&  thisHomeTaz == homeTaz )
			{
				ZZ_logsumhh = model->householdGroupVec[n].getLogsum();
				break;
			}
		}

		if( ZZ_logsumhh == -1 )
		{
			PredayPersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( headOfHousehold->getId(), homeTaz, workTaz );
			ZZ_logsumhh = personParam.getDpbLogsum();

			BigSerial groupId = hitssample->getGroupId();
			boost::shared_ptr<HM_Model::HouseholdGroup> thisHHGroup(new HM_Model::HouseholdGroup(groupId, homeTaz, ZZ_logsumhh ));

			model->householdGroupVec.push_back( *(thisHHGroup.get()));

			printHouseholdGroupLogsum( homeTaz, hitssample->getGroupId(), headOfHousehold->getId(), ZZ_logsumhh );
		}

		Household* householdT = const_cast<Household*>(household);
		householdT->setLogsum(ZZ_logsumhh);

	}

	const HM_Model::TazStats *tazstats  = getParent()->getModel()->getTazStatsByUnitId(unit->getId());

	if( tazstats->getChinesePercentage() > 0.76 ) //chetan TODO: add to xml file
		ZZ_hhchinese = 1;

	if( tazstats->getMalayPercentage() > 0.10 )
		ZZ_hhmalay 	 = 1;

	double ZZ_highInc = household->getIncome();
	double ZZ_middleInc = household->getIncome();
	double ZZ_lowInc  =  household->getIncome();

	if( ZZ_highInc >= 11000 )
		ZZ_highInc = 1;
	else
		ZZ_highInc = 0;


	if( ZZ_middleInc >= 2750 && ZZ_middleInc < 11000)
		ZZ_middleInc = 1;
	else
		ZZ_middleInc = 0;


	if( ZZ_lowInc < 2750 )
		ZZ_lowInc = 1;
	else
		ZZ_lowInc = 0;

	int ZZ_children = 0;

	if( household->getChildUnder15() > 0 )
		ZZ_children = 1;

	int chineseHousehold = 0;
	int malayHousehold   = 0;

	if( household->getEthnicityId() == 1 )
		chineseHousehold = 1;

	if( household->getEthnicityId() == 2 )
		malayHousehold = 1;



	double Vpriv = 	(barea		*  DD_area 		) +
					(blogsum	* ZZ_logsumhh 	) +
					(bchin	  	* ZZ_hhchinese 	* chineseHousehold ) +
					(bmalay		* ZZ_hhmalay 	* malayHousehold   ) +
					(bHighInc   * ZZ_highInc 	) +
					(bHIncChildApart * ZZ_children * ZZ_highInc	* Apartment 	) +
					(bHIncChildCondo * ZZ_children * ZZ_highInc	* Condo 		) +
					(bapartment  * Apartment ) +
					(bcondo 	 * Condo 	 ) +
					(bdetachedAndSemiDetached * DetachedAndSemidetaced ) +
					(terrace	* Terrace		) +
					(bageOfUnit25 * ZZ_ageOfUnitPrivate 	) +
					(bageOfUnit25Squared 	* ZZ_ageOfUnitPrivate * ZZ_ageOfUnitPrivate ) +
					(bageGreaterT25LessT50  * ZZ_ageBet25And50 	) +
					(bageGreaterT50  		* ZZ_ageGreater50 	) +
					(bmissingAge  			* ZZ_missingAge 	) +
					(bfreeholdAppartm  		* ZZ_freehold * Apartment 	) +
					(bfreeholdCondo  		* ZZ_freehold * Condo 		) +
					(fbreeholdTerrace  		* ZZ_freehold * Terrace 	);


	double Vhdb = 	(barea		*  DD_area 		) +
					(blogsum	* ZZ_logsumhh 	) +
					(bchin	  	* ZZ_hhchinese 	* chineseHousehold ) +
					(bmalay		* ZZ_hhmalay 	* malayHousehold   ) +
					(bHighInc   * ZZ_highInc 	) +
					(midIncChildHDB3 * ZZ_children * ZZ_middleInc 	* HDB3	) +
					(midIncChildHDB4 * ZZ_children * ZZ_middleInc 	* HDB4	) +
					(midIncChildHDB5 * ZZ_children * ZZ_middleInc 	* HDB5	) +
					(bhdb12  * HDB12 ) +
					(bhdb3   * HDB3  ) +
					(bhdb4 	 * HDB4	 ) +
					(bhdb5 	 * HDB5	 ) +
					(bageOfUnit30 * ZZ_ageOfUnitHDB ) +
					(bageOfUnit30Squared * ZZ_ageOfUnitHDB * ZZ_ageOfUnitHDB ) +
					(bageOfUnitGreater30 * ZZ_ageGreater30 );

	if( unit->getUnitType() <= 6 || unitType == 65 )
		V = Vhdb;
	else
		V = Vpriv;

	boost::mt19937 rng( clock() );
	boost::normal_distribution<> nd( 0.0, sde);
	boost::variate_generator<boost::mt19937&,  boost::normal_distribution<> > var_nor(rng, nd);
	wtp_e  = var_nor();

	//needed when wtp model is expressed as log wtp
	V = exp(V);

	return V;
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
    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;

    market->getAvailableEntries(entries);

    const HousingMarket::Entry* maxEntry = nullptr;
    double maxSurplus = 0; // holds the wp of the entry with maximum surplus.
    double finalBid = 0;
    double maxWp	= 0;
    double maxWtpe  = 0;
    double maxAffordability = 0;

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
			std::vector<const HousingMarket::Entry*>::iterator screenedEntriesItr;
			screenedEntriesItr = std::find(screenedEntries.begin(), screenedEntries.end(), entry );

			if( screenedEntriesItr == screenedEntries.end() )
				screenedEntries.push_back(entry);
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

        if(entry && entry->getOwner() != getParent() && entry->getAskingPrice() > 0.01 )
        {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

            bool flatEligibility = true;

            if( unit && unit->getUnitType() == 2 && household->getTwoRoomHdbEligibility()  == false)
            	flatEligibility = false;

            if( unit && unit->getUnitType() == 3 && household->getThreeRoomHdbEligibility() == false )
                flatEligibility = false;

            if( unit && unit->getUnitType() == 4 && household->getFourRoomHdbEligibility() == false )
                flatEligibility = false;

            if( unit && stats && flatEligibility )
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
            	double wp = calculateWillingnessToPay(unit, household, wtp_e);

            	wtp_e = wtp_e * entry->getAskingPrice(); //wtp error is a fraction of the asking price.

            	wp += wtp_e; // adjusted willingness to pay in millions of dollars

           	    std::string oldPCStr = "empty";
            	std::string newPCStr = "empty";

            	if( oldPC )
            		oldPCStr = oldPC->getSlaPostcode();

            	if( newPC )
            		newPCStr = newPC->getSlaPostcode();

            	printHouseholdBiddingList( day, household->getId(), unit->getId(), oldPCStr, newPCStr, wp);

            	wp = std::max(0.0, wp );

            	double currentBid = 0;
            	double currentSurplus = 0;

            	if( entry->getAskingPrice() != 0 )
            		computeBidValueLogistic( entry->getAskingPrice(), wp, currentBid, currentSurplus );
            	else
            		PrintOutV("Asking price is zero for unit " << entry->getUnitId() << std::endl );

            	if( household->getAffordabilityAmount() > household->getCurrentUnitPrice() )
            		maxAffordability = household->getAffordabilityAmount();
            	else
            		maxAffordability = household->getCurrentUnitPrice();

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
