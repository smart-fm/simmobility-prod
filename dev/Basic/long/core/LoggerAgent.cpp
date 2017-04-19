/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LoggerAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 21, 2014, 1:32 PM
 */

#include "LoggerAgent.hpp"
#include "message/MessageBus.hpp"
#include "Common.hpp"
#include "util/HelperFunctions.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ConfigManager.hpp"


using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;

namespace
{
    class LogMsg : public Message
    {
    public:

        LogMsg(const std::string& logMsg, LoggerAgent::LogFile fileType) : logMsg(logMsg), fileType(fileType)
    	{
            priority = INTERNAL_MESSAGE_PRIORITY;
        }

        std::string logMsg;
        LoggerAgent::LogFile fileType;
    };
}

LoggerAgent::LoggerAgent() : Entity(-1)
{

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	bool bids = config.ltParams.outputFiles.bids;
    bool expectations = config.ltParams.outputFiles.expectations;
    bool parcels = config.ltParams.outputFiles.parcels;
    bool units = config.ltParams.outputFiles.units;
    bool projects = config.ltParams.outputFiles.projects;
    bool hh_pc = config.ltParams.outputFiles.hh_pc;
    bool units_in_market = config.ltParams.outputFiles.units_in_market;
    bool log_taxi_availability = config.ltParams.outputFiles.log_taxi_availability;
    bool log_vehicle_ownership = config.ltParams.outputFiles.log_vehicle_ownership;
    bool log_taz_level_logsum = config.ltParams.outputFiles.log_taz_level_logsum;
    bool log_householdgrouplogsum = config.ltParams.outputFiles.log_householdgrouplogsum;
    bool log_individual_hits_logsum = config.ltParams.outputFiles.log_individual_hits_logsum;
    bool log_householdbidlist = config.ltParams.outputFiles.log_householdbidlist;
    bool log_individual_logsum_vo = config.ltParams.outputFiles.log_individual_logsum_vo;
    bool log_screeningprobabilities = config.ltParams.outputFiles.log_screeningprobabilities;
    bool log_hhchoiceset = config.ltParams.outputFiles.log_hhchoiceset;
    bool log_error = config.ltParams.outputFiles.log_error;
    bool log_school_assignment = config.ltParams.outputFiles.log_school_assignment;
    bool log_pre_school_assignment = config.ltParams.outputFiles.log_pre_school_assignment;
    bool log_hh_awakening = config.ltParams.outputFiles.log_hh_awakening;
    bool log_hh_exit = config.ltParams.outputFiles.log_hh_exit;
    bool log_random_nums = config.ltParams.outputFiles.log_random_nums;
    bool log_dev_roi = config.ltParams.outputFiles.log_dev_roi;
    bool log_household_statistics = config.ltParams.outputFiles.log_household_statistics;


    PrintOutV(">>>>>>>>>>>>>>>" << std::endl);
    PrintOutV("Output CSV generation. " << std::endl);
	PrintOutV("Output CSV generation. bids: " << bids << std::endl);
	PrintOutV("Output CSV generation. expectations: " << expectations << std::endl);
	PrintOutV("Output CSV generation. parcels: " << parcels << std::endl);
	PrintOutV("Output CSV generation. units: " << units << std::endl);
	PrintOutV("Output CSV generation. projects " << projects << std::endl);
	PrintOutV("Output CSV generation. hh_pc: " << hh_pc << std::endl);
	PrintOutV("Output CSV generation. units_in_market: " << units_in_market << std::endl);
	PrintOutV("Output CSV generation. log_taxi_availability: " << log_taxi_availability << std::endl);
	PrintOutV("Output CSV generation. log_vehicle_ownersip: " << log_vehicle_ownership << std::endl);
	PrintOutV("Output CSV generation. log_taz_level_logsum: " << log_taz_level_logsum << std::endl);
	PrintOutV("Output CSV generation. log_householdgrouplogsum: " << log_householdgrouplogsum << std::endl);
	PrintOutV("Output CSV generation. log_individual_hits_logsum: " << log_individual_hits_logsum << std::endl);
	PrintOutV("Output CSV generation. log_householdbidlist: " << log_householdbidlist << std::endl);
	PrintOutV("Output CSV generation. log_individual_logsum_vo: " << log_individual_logsum_vo << std::endl);
	PrintOutV("Output CSV generation. log_screeningprobabilities: " << log_screeningprobabilities << std::endl);
	PrintOutV("Output CSV generation. log_hhchoiceset: " << log_hhchoiceset << std::endl);
	PrintOutV("Output CSV generation. log_error: " << log_error << std::endl);
	PrintOutV("Output CSV generation. log_school_assignment: " << log_school_assignment << std::endl);
	PrintOutV("Output CSV generation. log_pre_school_assignment: " << log_pre_school_assignment << std::endl);
	PrintOutV("Output CSV generation. log_hh_awakening: " << log_hh_awakening << std::endl);
	PrintOutV("Output CSV generation. log_hh_exit: " << log_hh_exit << std::endl);
	PrintOutV("Output CSV generation. log_random_nums: " << log_random_nums << std::endl);
	PrintOutV("Output CSV generation. log_dev_roi: " << log_dev_roi << std::endl);
	PrintOutV("Output CSV generation. log_household_statistics: " << log_household_statistics << std::endl);
	PrintOutV("Output CSV generation. log_out_xx_files: " << config.ltParams.outputFiles.log_out_xx_files << std::endl);
	PrintOutV(">>>>>>>>>>>>>>>" << std::endl);



    if(bids)
    {
		//bids
		std::ofstream* bidsFile = new std::ofstream("bids.csv");
		streams.insert(std::make_pair(BIDS, bidsFile));

		*bidsFile << "awakening_day, householdId, TimeOnMarket, ageCategory, tenureStatus, futureTransitionPercentage, randomDrawFutureTransition, movingPercentage, randomDrawMovingRate" << std::endl;
    }

    if(expectations)
    {
		//expectations;
		std::ofstream* expectationsFile = new std::ofstream("expectations.csv");
		streams.insert(std::make_pair(EXPECTATIONS, expectationsFile));

		*expectationsFile << "bid_timestamp, day_to_apply, seller_id, unit_id, hedonic_price, asking_price, target_price" << std::endl;
    }

    if(parcels)
    {
		//eligible parcels
		std::ofstream* parcelsFile = new std::ofstream("parcels.csv");
		streams.insert(std::make_pair(PARCELS, parcelsFile));

		*parcelsFile << "id,lot_size, gpr, land_use_type_id, owner_name, owner_category, last_transaction_date, last_transaction_type_total, psm_per_gps, lease_type, lease_start_date, centroid_x, centroid_y, award_date,award_status,use_restriction,development_type_code,successful_tender_id,successful_tender_price,tender_closing_date,lease,status,developmentAllowed,nextAvailableDate" << std::endl;
    }

    if(units)
    {
		//units
		std::ofstream* unitsFile = new std::ofstream("units.csv");
		streams.insert(std::make_pair(UNITS, unitsFile));

		*unitsFile << "Id, BuildingId, UnitType, StoreyRange, ConstructionStatus, FloorArea, Storey, MonthlyRent, SaleFromDate.tm_year, OccupancyFromDate.tm_year, SaleStatus, OccupancyStatus, LastChangedDate.tm_year, unitProfit, parcelId, demolitionCost, quarter" << std::endl;
    }

    if(projects)
    {
		//projects
		std::ofstream* projectsFile = new std::ofstream("projects.csv");
		streams.insert(std::make_pair(PROJECTS, projectsFile));

		*projectsFile << "projectId,parcelId,developerId,templateId,projectName,constructionDate,completionDate,constructionCost,demolitionCost,totalCost,fmLotSize,grossRatio,grossArea" << std::endl;
    }

    if(hh_pc)
    {
		//hhpc postcodes
		std::ofstream* hhpcFile = new std::ofstream("HouseholdPostcodes.csv");
		streams.insert(std::make_pair(HH_PC, hhpcFile));
    }

    if(units_in_market)
    {
		//new units in the market
		std::ofstream* unitsInMarketFile = new std::ofstream("unitsInMarket.csv");
		streams.insert(std::make_pair(UNITS_IN_MARKET, unitsInMarketFile));

		*unitsInMarketFile << "sellerId, unitId, entryday, timeOnMarket, timeOffMarket" << std::endl;
    }

    if(log_taxi_availability)
    {
		//hh with taxi availability
		std::ofstream* taxiAvailabilityFile = new std::ofstream("hhWithtaxiAvailability.csv");
		streams.insert(std::make_pair(LOG_TAXI_AVAILABILITY, taxiAvailabilityFile));

		*taxiAvailabilityFile << "household_id, probability_taxiAccess, randonNum" << std::endl;
    }

    if(log_vehicle_ownership)
    {
		//vehicle ownership options of hh
		std::ofstream* vehicleOwnershipOptionFile = new std::ofstream("hhWithVehicleOwnership.csv");
		streams.insert(std::make_pair(LOG_VEHICLE_OWNERSIP, vehicleOwnershipOptionFile));

		*vehicleOwnershipOptionFile << "householdId, vehicleOwnershipOption" << std::endl;
    }

    if(log_taz_level_logsum)
    {
		//Tas level group logsum
		std::ofstream* tazLevelLogsumFile = new std::ofstream("tazLevelLogsum.csv");
		streams.insert(std::make_pair(LOG_TAZ_LEVEL_LOGSUM, tazLevelLogsumFile));

		*tazLevelLogsumFile << "taz, logsum" << std::endl;
    }

    if(log_householdgrouplogsum)
    {
		//household group logsum
		std::ofstream* householdGroupLogsumFile = new std::ofstream("householdGroupLogsum.csv");
		streams.insert(std::make_pair(LOG_HOUSEHOLDGROUPLOGSUM, householdGroupLogsumFile));
    }

    if(log_individual_hits_logsum)
    {
		//individual hits logsum
		std::ofstream* individualHitsLogsumFile = new std::ofstream("IndividualHitsLogsum.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_HITS_LOGSUM, individualHitsLogsumFile));

		*individualHitsLogsumFile << "individualId, logsum" << std::endl;
    }

    if(log_householdbidlist)
    {
		//household bid list
		std::ofstream* householdBidFile = new std::ofstream("householdBidList.csv");
		streams.insert(std::make_pair(LOG_HOUSEHOLDBIDLIST, householdBidFile));

		*householdBidFile << "day, householdId, unitId, willingnessToPay, AskingPrice, Affordability, BidAmount, Surplus, currentPostcode, unitPostcode" << std::endl;

    }

    if(log_individual_logsum_vo)
    {
		//individual hits logsum for vehicle ownership
		std::ofstream* individualHitsLogsumForVOFile = new std::ofstream("IndividualHitsLogsum4VO.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_LOGSUM_VO, individualHitsLogsumForVOFile));

		*individualHitsLogsumForVOFile << "hitsId , paxId , householdId , individualId , memberId , tazH , tazW , logsum[0] , logsum[1] ,logsum[2] , logsum[3] ,logsum[4] , logsum[5] ,travelProbability[0] , travelProbability[1] , travelProbability[2] , travelProbability[3] ,travelProbability[4] , travelProbability[5] ,tripsExpected[0] , tripsExpected[1], tripsExpected[2] , tripsExpected[3], tripsExpected[4] , tripsExpected[5]" << std::endl;
    }


    if(log_screeningprobabilities)
    {
		//screening probabilities
		std::ofstream* screeningProbabilitiesFile = new std::ofstream("ScreeningProbabilities.csv");
		streams.insert(std::make_pair(LOG_SCREENINGPROBABILITIES, screeningProbabilitiesFile));

		*screeningProbabilitiesFile << "householdId , probabilities[0], probabilities[1], probabilities[2], probabilities[3], probabilities[4], probabilities[5], probabilities[6], probabilities[7], probabilities[8], probabilities[9], probabilities[10], probabilities[11], probabilities[12], probabilities[13], probabilities[14], probabilities[15], probabilities[16], probabilities[17], probabilities[18], probabilities[19], probabilities[20], probabilities[21], probabilities[22], probabilities[23], probabilities[24], probabilities[25], probabilities[26], probabilities[27], probabilities[28], probabilities[29], probabilities[30], probabilities[31], probabilities[32], probabilities[33], probabilities[34], probabilities[35], probabilities[36], probabilities[37], probabilities[38], probabilities[39], probabilities[40], probabilities[41], probabilities[42], probabilities[43], probabilities[44], probabilities[45], probabilities[46], probabilities[47], probabilities[48], probabilities[49], probabilities[50], probabilities[51], probabilities[52], probabilities[53], probabilities[54], probabilities[55], probabilities[56], probabilities[57], probabilities[58], probabilities[59], probabilities[60], probabilities[61], probabilities[62], probabilities[63], probabilities[64], probabilities[65], probabilities[66], probabilities[67], probabilities[68], probabilities[69], probabilities[70], probabilities[71], probabilities[72], probabilities[73], probabilities[74], probabilities[75], probabilities[76], probabilities[77], probabilities[78], probabilities[79], probabilities[80], probabilities[81], probabilities[82], probabilities[83], probabilities[84], probabilities[85], probabilities[86], probabilities[87], probabilities[88], probabilities[89], probabilities[90], probabilities[91], probabilities[92], probabilities[93], probabilities[94], probabilities[95], probabilities[96], probabilities[97], probabilities[98], probabilities[99], probabilities[100], probabilities[101], probabilities[102], probabilities[103], probabilities[104], probabilities[105], probabilities[106], probabilities[107], probabilities[108], probabilities[109], probabilities[110], probabilities[111], probabilities[112], probabilities[113], probabilities[114], probabilities[115], probabilities[116], probabilities[117], probabilities[118], probabilities[119], probabilities[120], probabilities[121], probabilities[122], probabilities[123], probabilities[124], probabilities[125], probabilities[126], probabilities[127], probabilities[128], probabilities[129], probabilities[130], probabilities[131], probabilities[132], probabilities[133], probabilities[134], probabilities[135], probabilities[136], probabilities[137], probabilities[138], probabilities[139], probabilities[140], probabilities[141], probabilities[142], probabilities[143], probabilities[144], probabilities[145], probabilities[146], probabilities[147], probabilities[148], probabilities[149], probabilities[150], probabilities[151], probabilities[152], probabilities[153], probabilities[154], probabilities[155], probabilities[156], probabilities[157], probabilities[158], probabilities[159], probabilities[160], probabilities[161], probabilities[162], probabilities[163], probabilities[164], probabilities[165], probabilities[166], probabilities[167], probabilities[168], probabilities[169], probabilities[170], probabilities[171], probabilities[172], probabilities[173], probabilities[174], probabilities[175], probabilities[176], probabilities[177], probabilities[178], probabilities[179], probabilities[180], probabilities[181], probabilities[182], probabilities[183], probabilities[184], probabilities[185], probabilities[186], probabilities[187], probabilities[188], probabilities[189], probabilities[190], probabilities[191], probabilities[192], probabilities[193], probabilities[194], probabilities[195], probabilities[196], probabilities[197], probabilities[198], probabilities[199], probabilities[200], probabilities[201], probabilities[202], probabilities[203], probabilities[204], probabilities[205], probabilities[206], probabilities[207], probabilities[208], probabilities[209], probabilities[210], probabilities[211], probabilities[212], probabilities[213], probabilities[214], probabilities[215]" << std::endl;
    }

    if(log_hhchoiceset)
    {
		//HH Choice set
		std::ofstream* hhChoiceSetFile = new std::ofstream("HHChoiceSet.csv");
		streams.insert(std::make_pair(LOG_HHCHOICESET, hhChoiceSetFile));

		*hhChoiceSetFile << "day, householdId, unitId1, unitId2, unitId3, unitId4, unitId5, unitId6, unitId7, unitId8, unitId9, unitId10, unitId11, unitId12, unitId13, unitId14, unitId15, unitId16, unitId17, unitId18, unitId19, unitId20, unitId21, unitId22, unitId23, unitId24, unitId25, unitId26, unitId27, unitId28, unitId29, unitId30, unitId31, unitId32, unitId33, unitId34, unitId35, unitId36, unitId37, unitId38, unitId39, unitId40, unitId41, unitId42, unitId43, unitId44, unitId45, unitId46, unitId47, unitId48, unitId49, unitId50, unitId51, unitId52, unitId53, unitId54, unitId55, unitId56, unitId57, unitId58, unitId59, unitId60, unitId61, unitId62, unitId63, unitId64, unitId65, unitId66, unitId67, unitId68, unitId69, unitId70" << std::endl;
    }

    if(log_error)
    {
		//errors
		std::ofstream* errorFile = new std::ofstream("Errors.csv");
		streams.insert(std::make_pair(LOG_ERROR, errorFile));
    }

    if(log_school_assignment)
    {
		//school assignment
		std::ofstream* schoolAssignmentFile = new std::ofstream("schools.csv");
		streams.insert(std::make_pair(LOG_SCHOOL_ASSIGNMENT, schoolAssignmentFile));

		*schoolAssignmentFile << "individualId, primarySchoolId" << std::endl;
    }

    if(log_pre_school_assignment)
    {
		//pre school assignment
		std::ofstream* preSchoolAssignmentFile = new std::ofstream("preSchools.csv");
		streams.insert(std::make_pair(LOG_PRE_SCHOOL_ASSIGNMENT, preSchoolAssignmentFile));

		*preSchoolAssignmentFile << "hhid, individual_id, school_id" << std::endl;
    }

    if(log_hh_awakening)
    {
		//awakenings
		std::ofstream* hhawakeningFile = new std::ofstream("HH_Awakenings.csv");
		streams.insert(std::make_pair(LOG_HH_AWAKENING, hhawakeningFile));

		*hhawakeningFile << "awakening_day, householdId, TimeOnMarket, ageCategory, tenureStatus" << std::endl;
    }

    if(log_hh_exit)
    {
		//Exits
		std::ofstream* hhexitsFile = new std::ofstream("HH_Exits.csv");
		streams.insert(std::make_pair(LOG_HH_EXIT, hhexitsFile));

		*hhexitsFile << "day, household, exit_status" << std::endl;
    }

    if(log_random_nums)
    {
		//random nums
		std::ofstream* randomNumFile = new std::ofstream("randomNums.csv");
		streams.insert(std::make_pair(LOG_RANDOM_NUMS, randomNumFile));

		*randomNumFile << "randomNumber" << std::endl;
    }

    if(log_dev_roi)
    {
		//dev roi
		std::ofstream* roiFile = new std::ofstream("rdevROI.csv");
		streams.insert(std::make_pair(LOG_DEV_ROI, roiFile));

		*roiFile << "parcel.getId, newDevelopment, profit, devType, threshold_roi, roi" << std::endl;
    }

    if(log_household_statistics)
    {
		//household statistics
		std::ofstream* householdStatisticsFile = new std::ofstream("householdStatistics.csv");
		streams.insert(std::make_pair(LOG_HOUSEHOLD_STATISTICS, householdStatisticsFile));

		*householdStatisticsFile << "householdId, TwoRoomHdbEligibility, ThreeRoomHdbEligibility, FourRoomHdbEligibility, FamilyType, hhSize, adultSingaporean, coupleAndChild, engagedCouple, femaleAdultElderly, femaleAdultMiddleAged, femaleAdultYoung, femaleChild, maleAdultElderly, maleAdultMiddleAged, maleAdultYoung, maleChild, multigeneration, orphanSiblings, siblingsAndParents, singleParent" << std::endl;
    }

    //non eligible parcels
	 std::ofstream* nonEligibleParcelsFile = new std::ofstream("nonEligibleParcels.csv");
	 streams.insert(std::make_pair(LOG_NON_ELIGIBLE_PARCELS, nonEligibleParcelsFile));

	 *nonEligibleParcelsFile << "parcelId, reason" << std::endl;

	 //non eligible parcels
	 std::ofstream* eligibleParcelsFile = new std::ofstream("eligibleParcels.csv");
	 streams.insert(std::make_pair(LOG_ELIGIBLE_PARCELS, eligibleParcelsFile));

	 *eligibleParcelsFile << "parcelId, newDevelopment" << std::endl;

	 //gpr
	 std::ofstream* gprInfoFile = new std::ofstream("gprInfo.csv");
	 streams.insert(std::make_pair(LOG_GPR, gprInfoFile));

	 *gprInfoFile << "parcelId, parcelGPR, actualGPR, gap" << std::endl;


}

LoggerAgent::~LoggerAgent()
{
    typename Files::iterator it;
    for (it = streams.begin(); it != streams.end(); it++)
    {
        if (it->second)
        {
            it->second->close();
            delete (it->second);
        }
    }
    streams.clear();
}

bool LoggerAgent::isNonspatial()
{
    return false;
}

std::vector<sim_mob::BufferedBase*> LoggerAgent::buildSubscriptionList() 
{
	return std::vector<sim_mob::BufferedBase*>();
}

void LoggerAgent::onWorkerEnter() {}

void LoggerAgent::onWorkerExit() {}

Entity::UpdateStatus LoggerAgent::update(timeslice now)
{
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

void LoggerAgent::log(LogFile outputType, const std::string& logMsg)
{
	boost::mutex::scoped_lock lock( mtx );

    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_LOG, MessageBus::MessagePtr(new LogMsg(logMsg, outputType)));
}

void LoggerAgent::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
    switch (type)
    {
        case LTMID_LOG:
        {
            const LogMsg& msg = MSG_CAST(LogMsg, message);

            if (msg.fileType == STDOUT)
            {
                PrintOut(msg.logMsg << std::endl);
            }
            else
            {
                (*streams[msg.fileType]) << msg.logMsg << std::endl;
            }
            break;
        }
        default:break;
    };
}
