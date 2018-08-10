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

		*bidsFile << "bid_timestamp, seller_id, bidder_id, unit_id, bidder wtp, bidder wp+wp_error, wp_error, affordability, currentUnitHP,target_price, hedonicprice, lagCoefficient, asking_price, bid_value, bids_counter (daily), bid_status, logsum, floor_area, type_id, HHPC, UPC,sale_from_date,occupancy_from_date" << std::endl;
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

		*vehicleOwnershipOptionFile << "householdId, vehicleOwnershipOption, workInTopayo, liveInTopayo" << std::endl;

		std::ofstream* vehicleOwnershipOptionFile2 = new std::ofstream("hhWithVehicleOwnership2.csv");
		streams.insert(std::make_pair(LOG_VEHICLE_OWNERSIP2, vehicleOwnershipOptionFile2));

		*vehicleOwnershipOptionFile2 << "householdId, vehicleOwnershipOption, workInTopayo, liveInTopayo" << std::endl;
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
		*individualHitsLogsumFile << "title, hitsId, householdId,individualId, paxId,tazId_1,tazId_2,tazId_3,tazId_4,tazId_5,tazId_6,tazId_7,tazId_8,tazId_9,tazId_10,tazId_11,tazId_12,tazId_13,tazId_14,tazId_15,tazId_16,tazId_17,tazId_18,tazId_19,tazId_20,tazId_21,tazId_22,tazId_23,tazId_24,tazId_25,tazId_26,tazId_27,tazId_28,tazId_29,tazId_30,tazId_31,tazId_32,tazId_33,tazId_34,tazId_35,tazId_36,tazId_37,tazId_38,tazId_39,tazId_40,tazId_41,tazId_42,tazId_43,tazId_44,tazId_45,tazId_46,tazId_47,tazId_48,tazId_49,tazId_50,tazId_51,tazId_52,tazId_53,tazId_54,tazId_55,tazId_56,tazId_57,tazId_58,tazId_59,tazId_60,tazId_61,tazId_62,tazId_63,tazId_64,tazId_65,tazId_66,tazId_67,tazId_68,tazId_69,tazId_70,tazId_71,tazId_72,tazId_73,tazId_74,tazId_75,tazId_76,tazId_77,tazId_78,tazId_79,tazId_80,tazId_81,tazId_82,tazId_83,tazId_84,tazId_85,tazId_86,tazId_87,tazId_88,tazId_89,tazId_90,tazId_91,tazId_92,tazId_93,tazId_94,tazId_95,tazId_96,tazId_97,tazId_98,tazId_99,tazId_100,tazId_101,tazId_102,tazId_103,tazId_104,tazId_105,tazId_106,tazId_107,tazId_108,tazId_109,tazId_110,tazId_111,tazId_112,tazId_113,tazId_114,tazId_115,tazId_116,tazId_117,tazId_118,tazId_119,tazId_120,tazId_121,tazId_122,tazId_123,tazId_124,tazId_125,tazId_126,tazId_127,tazId_128,tazId_129,tazId_130,tazId_131,tazId_132,tazId_133,tazId_134,tazId_135,tazId_136,tazId_137,tazId_138,tazId_139,tazId_140,tazId_141,tazId_142,tazId_143,tazId_144,tazId_145,tazId_146,tazId_147,tazId_148,tazId_149,tazId_150,tazId_151,tazId_152,tazId_153,tazId_154,tazId_155,tazId_156,tazId_157,tazId_158,tazId_159,tazId_160,tazId_161,tazId_162,tazId_163,tazId_164,tazId_165,tazId_166,tazId_167,tazId_168,tazId_169,tazId_170,tazId_171,tazId_172,tazId_173,tazId_174,tazId_175,tazId_176,tazId_177,tazId_178,tazId_179,tazId_180,tazId_181,tazId_182,tazId_183,tazId_184,tazId_185,tazId_186,tazId_187,tazId_188,tazId_189,tazId_190,tazId_191,tazId_192,tazId_193,tazId_194,tazId_195,tazId_196,tazId_197,tazId_198,tazId_199,tazId_200,tazId_201,tazId_202,tazId_203,tazId_204,tazId_205,tazId_206,tazId_207,tazId_208,tazId_209,tazId_210,tazId_211,tazId_212,tazId_213,tazId_214,tazId_215,tazId_216,tazId_217,tazId_218,tazId_219,tazId_220,tazId_221,tazId_222,tazId_223,tazId_224,tazId_225,tazId_226,tazId_227,tazId_228,tazId_229,tazId_230,tazId_231,tazId_232,tazId_233,tazId_234,tazId_235,tazId_236,tazId_237,tazId_238,tazId_239,tazId_240,tazId_241,tazId_242,tazId_243,tazId_244,tazId_245,tazId_246,tazId_247,tazId_248,tazId_249,tazId_250,tazId_251,tazId_252,tazId_253,tazId_254,tazId_255,tazId_256,tazId_257,tazId_258,tazId_259,tazId_260,tazId_261,tazId_262,tazId_263,tazId_264,tazId_265,tazId_266,tazId_267,tazId_268,tazId_269,tazId_270,tazId_271,tazId_272,tazId_273,tazId_274,tazId_275,tazId_276,tazId_277,tazId_278,tazId_279,tazId_280,tazId_281,tazId_282,tazId_283,tazId_284,tazId_285,tazId_286,tazId_287,tazId_288,tazId_289,tazId_290,tazId_291,tazId_292,tazId_293,tazId_294,tazId_295,tazId_296,tazId_297,tazId_298,tazId_299,tazId_300,tazId_301,tazId_302,tazId_303,tazId_304,tazId_305,tazId_306,tazId_307,tazId_308,tazId_309,tazId_310,tazId_311,tazId_312,tazId_313,tazId_314,tazId_315,tazId_316,tazId_317,tazId_318,tazId_319,tazId_320,tazId_321,tazId_322,tazId_323,tazId_324,tazId_325,tazId_326,tazId_327,tazId_328,tazId_329,tazId_330,tazId_331,tazId_332,tazId_333,tazId_334,tazId_335,tazId_336,tazId_337,tazId_338,tazId_339,tazId_340,tazId_341,tazId_342,tazId_343,tazId_344,tazId_345,tazId_346,tazId_347,tazId_348,tazId_349,tazId_350,tazId_351,tazId_352,tazId_353,tazId_354,tazId_355,tazId_356,tazId_357,tazId_358,tazId_359,tazId_360,tazId_361,tazId_362,tazId_363,tazId_364,tazId_365,tazId_366,tazId_367,tazId_368,tazId_369,tazId_370,tazId_371,tazId_372,tazId_373,tazId_374,tazId_375,tazId_376,tazId_377,tazId_378,tazId_379,tazId_380,tazId_381,tazId_382,tazId_383,tazId_384,tazId_385,tazId_386,tazId_387,tazId_388,tazId_389,tazId_390,tazId_391,tazId_392,tazId_393,tazId_394,tazId_395,tazId_396,tazId_397,tazId_398,tazId_399,tazId_400,tazId_401,tazId_402,tazId_403,tazId_404,tazId_405,tazId_406,tazId_407,tazId_408,tazId_409,tazId_410,tazId_411,tazId_412,tazId_413,tazId_414,tazId_415,tazId_416,tazId_417,tazId_418,tazId_419,tazId_420,tazId_421,tazId_422,tazId_423,tazId_424,tazId_425,tazId_426,tazId_427,tazId_428,tazId_429,tazId_430,tazId_431,tazId_432,tazId_433,tazId_434,tazId_435,tazId_436,tazId_437,tazId_438,tazId_439,tazId_440,tazId_441,tazId_442,tazId_443,tazId_444,tazId_445,tazId_446,tazId_447,tazId_448,tazId_449,tazId_450,tazId_451,tazId_452,tazId_453,tazId_454,tazId_455,tazId_456,tazId_457,tazId_458,tazId_459,tazId_460,tazId_461,tazId_462,tazId_463,tazId_464,tazId_465,tazId_466,tazId_467,tazId_468,tazId_469,tazId_470,tazId_471,tazId_472,tazId_473,tazId_474,tazId_475,tazId_476,tazId_477,tazId_478,tazId_479,tazId_480,tazId_481,tazId_482,tazId_483,tazId_484,tazId_485,tazId_486,tazId_487,tazId_488,tazId_489,tazId_490,tazId_491,tazId_492,tazId_493,tazId_494,tazId_495,tazId_496,tazId_497,tazId_498,tazId_499,tazId_500,tazId_501,tazId_502,tazId_503,tazId_504,tazId_505,tazId_506,tazId_507,tazId_508,tazId_509,tazId_510,tazId_511,tazId_512,tazId_513,tazId_514,tazId_515,tazId_516,tazId_517,tazId_518,tazId_519,tazId_520,tazId_521,tazId_522,tazId_523,tazId_524,tazId_525,tazId_526,tazId_527,tazId_528,tazId_529,tazId_530,tazId_531,tazId_532,tazId_533,tazId_534,tazId_535,tazId_536,tazId_537,tazId_538,tazId_539,tazId_540,tazId_541,tazId_542,tazId_543,tazId_544,tazId_545,tazId_546,tazId_547,tazId_548,tazId_549,tazId_550,tazId_551,tazId_552,tazId_553,tazId_554,tazId_555,tazId_556,tazId_557,tazId_558,tazId_559,tazId_560,tazId_561,tazId_562,tazId_563,tazId_564,tazId_565,tazId_566,tazId_567,tazId_568,tazId_569,tazId_570,tazId_571,tazId_572,tazId_573,tazId_574,tazId_575,tazId_576,tazId_577,tazId_578,tazId_579,tazId_580,tazId_581,tazId_582,tazId_583,tazId_584,tazId_585,tazId_586,tazId_587,tazId_588,tazId_589,tazId_590,tazId_591,tazId_592,tazId_593,tazId_594,tazId_595,tazId_596,tazId_597,tazId_598,tazId_599,tazId_600,tazId_601,tazId_602,tazId_603,tazId_604,tazId_605,tazId_606,tazId_607,tazId_608,tazId_609,tazId_610,tazId_611,tazId_612,tazId_613,tazId_614,tazId_615,tazId_616,tazId_617,tazId_618,tazId_619,tazId_620,tazId_621,tazId_622,tazId_623,tazId_624,tazId_625,tazId_626,tazId_627,tazId_628,tazId_629,tazId_630,tazId_631,tazId_632,tazId_633,tazId_634,tazId_635,tazId_636,tazId_637,tazId_638,tazId_639,tazId_640,tazId_641,tazId_642,tazId_643,tazId_644,tazId_645,tazId_646,tazId_647,tazId_648,tazId_649,tazId_650,tazId_651,tazId_652,tazId_653,tazId_654,tazId_655,tazId_656,tazId_657,tazId_658,tazId_659,tazId_660,tazId_661,tazId_662,tazId_663,tazId_664,tazId_665,tazId_666,tazId_667,tazId_668,tazId_669,tazId_670,tazId_671,tazId_672,tazId_673,tazId_674,tazId_675,tazId_676,tazId_677,tazId_678,tazId_679,tazId_680,tazId_681,tazId_682,tazId_683,tazId_684,tazId_685,tazId_686,tazId_687,tazId_688,tazId_689,tazId_690,tazId_691,tazId_692,tazId_693,tazId_694,tazId_695,tazId_696,tazId_697,tazId_698,tazId_699,tazId_700,tazId_701,tazId_702,tazId_703,tazId_704,tazId_705,tazId_706,tazId_707,tazId_708,tazId_709,tazId_710,tazId_711,tazId_712,tazId_713,tazId_714,tazId_715,tazId_716,tazId_717,tazId_718,tazId_719,tazId_720,tazId_721,tazId_722,tazId_723,tazId_724,tazId_725,tazId_726,tazId_727,tazId_728,tazId_729,tazId_730,tazId_731,tazId_732,tazId_733,tazId_734,tazId_735,tazId_736,tazId_737,tazId_738,tazId_739,tazId_740,tazId_741,tazId_742,tazId_743,tazId_744,tazId_745,tazId_746,tazId_747,tazId_748,tazId_749,tazId_750,tazId_751,tazId_752,tazId_753,tazId_754,tazId_755,tazId_756,tazId_757,tazId_758,tazId_759,tazId_760,tazId_761,tazId_762,tazId_763,tazId_764,tazId_765,tazId_766,tazId_767,tazId_768,tazId_769,tazId_770,tazId_771,tazId_772,tazId_773,tazId_774,tazId_775,tazId_776,tazId_777,tazId_778,tazId_779,tazId_780,tazId_781,tazId_782,tazId_783,tazId_784,tazId_785,tazId_786,tazId_787,tazId_788,tazId_789,tazId_790,tazId_791,tazId_792,tazId_793,tazId_794,tazId_795,tazId_796,tazId_797,tazId_798,tazId_799,tazId_800,tazId_801,tazId_802,tazId_803,tazId_804,tazId_805,tazId_806,tazId_807,tazId_808,tazId_809,tazId_810,tazId_811,tazId_812,tazId_813,tazId_814,tazId_815,tazId_816,tazId_817,tazId_818,tazId_819,tazId_820,tazId_821,tazId_822,tazId_823,tazId_824,tazId_825,tazId_826,tazId_827,tazId_828,tazId_829,tazId_830,tazId_831,tazId_832,tazId_833,tazId_834,tazId_835,tazId_836,tazId_837,tazId_838,tazId_839,tazId_840,tazId_841,tazId_842,tazId_843,tazId_844,tazId_845,tazId_846,tazId_847,tazId_848,tazId_849,tazId_850,tazId_851,tazId_852,tazId_853,tazId_854,tazId_855,tazId_856,tazId_857,tazId_858,tazId_859,tazId_860,tazId_861,tazId_862,tazId_863,tazId_864,tazId_865,tazId_866,tazId_867,tazId_868,tazId_869,tazId_870,tazId_871,tazId_872,tazId_873,tazId_874,tazId_875,tazId_876,tazId_877,tazId_878,tazId_879,tazId_880,tazId_881,tazId_882,tazId_883,tazId_884,tazId_885,tazId_886,tazId_887,tazId_888,tazId_889,tazId_890,tazId_891,tazId_892,tazId_893,tazId_894,tazId_895,tazId_896,tazId_897,tazId_898,tazId_899,tazId_900,tazId_901,tazId_902,tazId_903,tazId_904,tazId_905,tazId_906,tazId_907,tazId_908,tazId_909,tazId_910,tazId_911,tazId_912,tazId_913,tazId_914,tazId_915,tazId_916,tazId_917,tazId_918,tazId_919,tazId_920,tazId_921,tazId_922,tazId_923,tazId_924,tazId_925,tazId_926,tazId_927,tazId_928,tazId_929,tazId_930,tazId_931,tazId_932,tazId_933,tazId_934,tazId_935,tazId_936,tazId_937,tazId_938,tazId_939,tazId_940,tazId_941,tazId_942,tazId_943,tazId_944,tazId_945,tazId_946,tazId_947,tazId_948,tazId_949,tazId_950,tazId_951,tazId_952,tazId_953,tazId_954,tazId_955,tazId_956,tazId_957,tazId_958,tazId_959,tazId_960,tazId_961,tazId_962,tazId_963,tazId_964,tazId_965,tazId_966,tazId_967,tazId_968,tazId_969,tazId_970,tazId_971,tazId_972,tazId_973,tazId_974,tazId_975,tazId_976,tazId_977,tazId_978,tazId_979,tazId_980,tazId_981,tazId_982,tazId_983,tazId_984,tazId_985,tazId_986,tazId_987,tazId_988,tazId_989,tazId_990,tazId_991,tazId_992,tazId_993,tazId_994,tazId_995,tazId_996,tazId_997,tazId_998,tazId_999,tazId_1000,tazId_1001,tazId_1002,tazId_1003,tazId_1004,tazId_1005,tazId_1006,tazId_1007,tazId_1008,tazId_1009,tazId_1010,tazId_1011,tazId_1012,tazId_1013,tazId_1014,tazId_1015,tazId_1016,tazId_1017,tazId_1018,tazId_1019,tazId_1020,tazId_1021,tazId_1022,tazId_1023,tazId_1024,tazId_1025,tazId_1026,tazId_1027,tazId_1028,tazId_1029,tazId_1030,tazId_1031,tazId_1032,tazId_1033,tazId_1034,tazId_1035,tazId_1036,tazId_1037,tazId_1038,tazId_1039,tazId_1040,tazId_1041,tazId_1042,tazId_1043,tazId_1044,tazId_1045,tazId_1046,tazId_1047,tazId_1048,tazId_1049,tazId_1050,tazId_1051,tazId_1052,tazId_1053,tazId_1054,tazId_1055,tazId_1056,tazId_1057,tazId_1058,tazId_1059,tazId_1060,tazId_1061,tazId_1062,tazId_1063,tazId_1064,tazId_1065,tazId_1066,tazId_1067,tazId_1068,tazId_1069,tazId_1070,tazId_1071,tazId_1072,tazId_1073,tazId_1074,tazId_1075,tazId_1076,tazId_1077,tazId_1078,tazId_1079,tazId_1080,tazId_1081,tazId_1082,tazId_1083,tazId_1084,tazId_1085,tazId_1086,tazId_1087,tazId_1088,tazId_1089,tazId_1090,tazId_1091,tazId_1092,tazId_1093,tazId_1094,tazId_1095,tazId_1096,tazId_1097,tazId_1098,tazId_1099,tazId_1100,tazId_1101,tazId_1102,tazId_1103,tazId_1104,tazId_1105,tazId_1106,tazId_1107,tazId_1108,tazId_1109,tazId_1110,tazId_1111,tazId_1112,tazId_1113,tazId_1114,tazId_1115,tazId_1116,tazId_1117,tazId_1118,tazId_1119,tazId_1120,tazId_1121,tazId_1122,tazId_1123,tazId_1124,tazId_1125,tazId_1126,tazId_1127,tazId_1128,tazId_1129,tazId_1130,tazId_1131,tazId_1132,tazId_1133,tazId_1134,tazId_1135,tazId_1136,tazId_1137,tazId_1138,tazId_1139,tazId_1140,tazId_1141,tazId_1142,tazId_1143,tazId_1144,tazId_1145,tazId_1146,tazId_1147,tazId_1148,tazId_1149,tazId_1150,tazId_1151,tazId_1152,tazId_1153,tazId_1154,tazId_1155,tazId_1156,tazId_1157,tazId_1158,tazId_1159,tazId_1160,tazId_1161,tazId_1162,tazId_1163,tazId_1164,tazId_1165,tazId_1166,tazId_1167,tazId_1168,tazId_1169," << std::endl;

		std::ofstream* individualWorkLogsumFile = new std::ofstream("IndividualWorkLogsum.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_WORK_LOGSUM, individualWorkLogsumFile));
		*individualWorkLogsumFile << "title, hitsId, householdId,individualId, paxId,tazId_1,tazId_2,tazId_3,tazId_4,tazId_5,tazId_6,tazId_7,tazId_8,tazId_9,tazId_10,tazId_11,tazId_12,tazId_13,tazId_14,tazId_15,tazId_16,tazId_17,tazId_18,tazId_19,tazId_20,tazId_21,tazId_22,tazId_23,tazId_24,tazId_25,tazId_26,tazId_27,tazId_28,tazId_29,tazId_30,tazId_31,tazId_32,tazId_33,tazId_34,tazId_35,tazId_36,tazId_37,tazId_38,tazId_39,tazId_40,tazId_41,tazId_42,tazId_43,tazId_44,tazId_45,tazId_46,tazId_47,tazId_48,tazId_49,tazId_50,tazId_51,tazId_52,tazId_53,tazId_54,tazId_55,tazId_56,tazId_57,tazId_58,tazId_59,tazId_60,tazId_61,tazId_62,tazId_63,tazId_64,tazId_65,tazId_66,tazId_67,tazId_68,tazId_69,tazId_70,tazId_71,tazId_72,tazId_73,tazId_74,tazId_75,tazId_76,tazId_77,tazId_78,tazId_79,tazId_80,tazId_81,tazId_82,tazId_83,tazId_84,tazId_85,tazId_86,tazId_87,tazId_88,tazId_89,tazId_90,tazId_91,tazId_92,tazId_93,tazId_94,tazId_95,tazId_96,tazId_97,tazId_98,tazId_99,tazId_100,tazId_101,tazId_102,tazId_103,tazId_104,tazId_105,tazId_106,tazId_107,tazId_108,tazId_109,tazId_110,tazId_111,tazId_112,tazId_113,tazId_114,tazId_115,tazId_116,tazId_117,tazId_118,tazId_119,tazId_120,tazId_121,tazId_122,tazId_123,tazId_124,tazId_125,tazId_126,tazId_127,tazId_128,tazId_129,tazId_130,tazId_131,tazId_132,tazId_133,tazId_134,tazId_135,tazId_136,tazId_137,tazId_138,tazId_139,tazId_140,tazId_141,tazId_142,tazId_143,tazId_144,tazId_145,tazId_146,tazId_147,tazId_148,tazId_149,tazId_150,tazId_151,tazId_152,tazId_153,tazId_154,tazId_155,tazId_156,tazId_157,tazId_158,tazId_159,tazId_160,tazId_161,tazId_162,tazId_163,tazId_164,tazId_165,tazId_166,tazId_167,tazId_168,tazId_169,tazId_170,tazId_171,tazId_172,tazId_173,tazId_174,tazId_175,tazId_176,tazId_177,tazId_178,tazId_179,tazId_180,tazId_181,tazId_182,tazId_183,tazId_184,tazId_185,tazId_186,tazId_187,tazId_188,tazId_189,tazId_190,tazId_191,tazId_192,tazId_193,tazId_194,tazId_195,tazId_196,tazId_197,tazId_198,tazId_199,tazId_200,tazId_201,tazId_202,tazId_203,tazId_204,tazId_205,tazId_206,tazId_207,tazId_208,tazId_209,tazId_210,tazId_211,tazId_212,tazId_213,tazId_214,tazId_215,tazId_216,tazId_217,tazId_218,tazId_219,tazId_220,tazId_221,tazId_222,tazId_223,tazId_224,tazId_225,tazId_226,tazId_227,tazId_228,tazId_229,tazId_230,tazId_231,tazId_232,tazId_233,tazId_234,tazId_235,tazId_236,tazId_237,tazId_238,tazId_239,tazId_240,tazId_241,tazId_242,tazId_243,tazId_244,tazId_245,tazId_246,tazId_247,tazId_248,tazId_249,tazId_250,tazId_251,tazId_252,tazId_253,tazId_254,tazId_255,tazId_256,tazId_257,tazId_258,tazId_259,tazId_260,tazId_261,tazId_262,tazId_263,tazId_264,tazId_265,tazId_266,tazId_267,tazId_268,tazId_269,tazId_270,tazId_271,tazId_272,tazId_273,tazId_274,tazId_275,tazId_276,tazId_277,tazId_278,tazId_279,tazId_280,tazId_281,tazId_282,tazId_283,tazId_284,tazId_285,tazId_286,tazId_287,tazId_288,tazId_289,tazId_290,tazId_291,tazId_292,tazId_293,tazId_294,tazId_295,tazId_296,tazId_297,tazId_298,tazId_299,tazId_300,tazId_301,tazId_302,tazId_303,tazId_304,tazId_305,tazId_306,tazId_307,tazId_308,tazId_309,tazId_310,tazId_311,tazId_312,tazId_313,tazId_314,tazId_315,tazId_316,tazId_317,tazId_318,tazId_319,tazId_320,tazId_321,tazId_322,tazId_323,tazId_324,tazId_325,tazId_326,tazId_327,tazId_328,tazId_329,tazId_330,tazId_331,tazId_332,tazId_333,tazId_334,tazId_335,tazId_336,tazId_337,tazId_338,tazId_339,tazId_340,tazId_341,tazId_342,tazId_343,tazId_344,tazId_345,tazId_346,tazId_347,tazId_348,tazId_349,tazId_350,tazId_351,tazId_352,tazId_353,tazId_354,tazId_355,tazId_356,tazId_357,tazId_358,tazId_359,tazId_360,tazId_361,tazId_362,tazId_363,tazId_364,tazId_365,tazId_366,tazId_367,tazId_368,tazId_369,tazId_370,tazId_371,tazId_372,tazId_373,tazId_374,tazId_375,tazId_376,tazId_377,tazId_378,tazId_379,tazId_380,tazId_381,tazId_382,tazId_383,tazId_384,tazId_385,tazId_386,tazId_387,tazId_388,tazId_389,tazId_390,tazId_391,tazId_392,tazId_393,tazId_394,tazId_395,tazId_396,tazId_397,tazId_398,tazId_399,tazId_400,tazId_401,tazId_402,tazId_403,tazId_404,tazId_405,tazId_406,tazId_407,tazId_408,tazId_409,tazId_410,tazId_411,tazId_412,tazId_413,tazId_414,tazId_415,tazId_416,tazId_417,tazId_418,tazId_419,tazId_420,tazId_421,tazId_422,tazId_423,tazId_424,tazId_425,tazId_426,tazId_427,tazId_428,tazId_429,tazId_430,tazId_431,tazId_432,tazId_433,tazId_434,tazId_435,tazId_436,tazId_437,tazId_438,tazId_439,tazId_440,tazId_441,tazId_442,tazId_443,tazId_444,tazId_445,tazId_446,tazId_447,tazId_448,tazId_449,tazId_450,tazId_451,tazId_452,tazId_453,tazId_454,tazId_455,tazId_456,tazId_457,tazId_458,tazId_459,tazId_460,tazId_461,tazId_462,tazId_463,tazId_464,tazId_465,tazId_466,tazId_467,tazId_468,tazId_469,tazId_470,tazId_471,tazId_472,tazId_473,tazId_474,tazId_475,tazId_476,tazId_477,tazId_478,tazId_479,tazId_480,tazId_481,tazId_482,tazId_483,tazId_484,tazId_485,tazId_486,tazId_487,tazId_488,tazId_489,tazId_490,tazId_491,tazId_492,tazId_493,tazId_494,tazId_495,tazId_496,tazId_497,tazId_498,tazId_499,tazId_500,tazId_501,tazId_502,tazId_503,tazId_504,tazId_505,tazId_506,tazId_507,tazId_508,tazId_509,tazId_510,tazId_511,tazId_512,tazId_513,tazId_514,tazId_515,tazId_516,tazId_517,tazId_518,tazId_519,tazId_520,tazId_521,tazId_522,tazId_523,tazId_524,tazId_525,tazId_526,tazId_527,tazId_528,tazId_529,tazId_530,tazId_531,tazId_532,tazId_533,tazId_534,tazId_535,tazId_536,tazId_537,tazId_538,tazId_539,tazId_540,tazId_541,tazId_542,tazId_543,tazId_544,tazId_545,tazId_546,tazId_547,tazId_548,tazId_549,tazId_550,tazId_551,tazId_552,tazId_553,tazId_554,tazId_555,tazId_556,tazId_557,tazId_558,tazId_559,tazId_560,tazId_561,tazId_562,tazId_563,tazId_564,tazId_565,tazId_566,tazId_567,tazId_568,tazId_569,tazId_570,tazId_571,tazId_572,tazId_573,tazId_574,tazId_575,tazId_576,tazId_577,tazId_578,tazId_579,tazId_580,tazId_581,tazId_582,tazId_583,tazId_584,tazId_585,tazId_586,tazId_587,tazId_588,tazId_589,tazId_590,tazId_591,tazId_592,tazId_593,tazId_594,tazId_595,tazId_596,tazId_597,tazId_598,tazId_599,tazId_600,tazId_601,tazId_602,tazId_603,tazId_604,tazId_605,tazId_606,tazId_607,tazId_608,tazId_609,tazId_610,tazId_611,tazId_612,tazId_613,tazId_614,tazId_615,tazId_616,tazId_617,tazId_618,tazId_619,tazId_620,tazId_621,tazId_622,tazId_623,tazId_624,tazId_625,tazId_626,tazId_627,tazId_628,tazId_629,tazId_630,tazId_631,tazId_632,tazId_633,tazId_634,tazId_635,tazId_636,tazId_637,tazId_638,tazId_639,tazId_640,tazId_641,tazId_642,tazId_643,tazId_644,tazId_645,tazId_646,tazId_647,tazId_648,tazId_649,tazId_650,tazId_651,tazId_652,tazId_653,tazId_654,tazId_655,tazId_656,tazId_657,tazId_658,tazId_659,tazId_660,tazId_661,tazId_662,tazId_663,tazId_664,tazId_665,tazId_666,tazId_667,tazId_668,tazId_669,tazId_670,tazId_671,tazId_672,tazId_673,tazId_674,tazId_675,tazId_676,tazId_677,tazId_678,tazId_679,tazId_680,tazId_681,tazId_682,tazId_683,tazId_684,tazId_685,tazId_686,tazId_687,tazId_688,tazId_689,tazId_690,tazId_691,tazId_692,tazId_693,tazId_694,tazId_695,tazId_696,tazId_697,tazId_698,tazId_699,tazId_700,tazId_701,tazId_702,tazId_703,tazId_704,tazId_705,tazId_706,tazId_707,tazId_708,tazId_709,tazId_710,tazId_711,tazId_712,tazId_713,tazId_714,tazId_715,tazId_716,tazId_717,tazId_718,tazId_719,tazId_720,tazId_721,tazId_722,tazId_723,tazId_724,tazId_725,tazId_726,tazId_727,tazId_728,tazId_729,tazId_730,tazId_731,tazId_732,tazId_733,tazId_734,tazId_735,tazId_736,tazId_737,tazId_738,tazId_739,tazId_740,tazId_741,tazId_742,tazId_743,tazId_744,tazId_745,tazId_746,tazId_747,tazId_748,tazId_749,tazId_750,tazId_751,tazId_752,tazId_753,tazId_754,tazId_755,tazId_756,tazId_757,tazId_758,tazId_759,tazId_760,tazId_761,tazId_762,tazId_763,tazId_764,tazId_765,tazId_766,tazId_767,tazId_768,tazId_769,tazId_770,tazId_771,tazId_772,tazId_773,tazId_774,tazId_775,tazId_776,tazId_777,tazId_778,tazId_779,tazId_780,tazId_781,tazId_782,tazId_783,tazId_784,tazId_785,tazId_786,tazId_787,tazId_788,tazId_789,tazId_790,tazId_791,tazId_792,tazId_793,tazId_794,tazId_795,tazId_796,tazId_797,tazId_798,tazId_799,tazId_800,tazId_801,tazId_802,tazId_803,tazId_804,tazId_805,tazId_806,tazId_807,tazId_808,tazId_809,tazId_810,tazId_811,tazId_812,tazId_813,tazId_814,tazId_815,tazId_816,tazId_817,tazId_818,tazId_819,tazId_820,tazId_821,tazId_822,tazId_823,tazId_824,tazId_825,tazId_826,tazId_827,tazId_828,tazId_829,tazId_830,tazId_831,tazId_832,tazId_833,tazId_834,tazId_835,tazId_836,tazId_837,tazId_838,tazId_839,tazId_840,tazId_841,tazId_842,tazId_843,tazId_844,tazId_845,tazId_846,tazId_847,tazId_848,tazId_849,tazId_850,tazId_851,tazId_852,tazId_853,tazId_854,tazId_855,tazId_856,tazId_857,tazId_858,tazId_859,tazId_860,tazId_861,tazId_862,tazId_863,tazId_864,tazId_865,tazId_866,tazId_867,tazId_868,tazId_869,tazId_870,tazId_871,tazId_872,tazId_873,tazId_874,tazId_875,tazId_876,tazId_877,tazId_878,tazId_879,tazId_880,tazId_881,tazId_882,tazId_883,tazId_884,tazId_885,tazId_886,tazId_887,tazId_888,tazId_889,tazId_890,tazId_891,tazId_892,tazId_893,tazId_894,tazId_895,tazId_896,tazId_897,tazId_898,tazId_899,tazId_900,tazId_901,tazId_902,tazId_903,tazId_904,tazId_905,tazId_906,tazId_907,tazId_908,tazId_909,tazId_910,tazId_911,tazId_912,tazId_913,tazId_914,tazId_915,tazId_916,tazId_917,tazId_918,tazId_919,tazId_920,tazId_921,tazId_922,tazId_923,tazId_924,tazId_925,tazId_926,tazId_927,tazId_928,tazId_929,tazId_930,tazId_931,tazId_932,tazId_933,tazId_934,tazId_935,tazId_936,tazId_937,tazId_938,tazId_939,tazId_940,tazId_941,tazId_942,tazId_943,tazId_944,tazId_945,tazId_946,tazId_947,tazId_948,tazId_949,tazId_950,tazId_951,tazId_952,tazId_953,tazId_954,tazId_955,tazId_956,tazId_957,tazId_958,tazId_959,tazId_960,tazId_961,tazId_962,tazId_963,tazId_964,tazId_965,tazId_966,tazId_967,tazId_968,tazId_969,tazId_970,tazId_971,tazId_972,tazId_973,tazId_974,tazId_975,tazId_976,tazId_977,tazId_978,tazId_979,tazId_980,tazId_981,tazId_982,tazId_983,tazId_984,tazId_985,tazId_986,tazId_987,tazId_988,tazId_989,tazId_990,tazId_991,tazId_992,tazId_993,tazId_994,tazId_995,tazId_996,tazId_997,tazId_998,tazId_999,tazId_1000,tazId_1001,tazId_1002,tazId_1003,tazId_1004,tazId_1005,tazId_1006,tazId_1007,tazId_1008,tazId_1009,tazId_1010,tazId_1011,tazId_1012,tazId_1013,tazId_1014,tazId_1015,tazId_1016,tazId_1017,tazId_1018,tazId_1019,tazId_1020,tazId_1021,tazId_1022,tazId_1023,tazId_1024,tazId_1025,tazId_1026,tazId_1027,tazId_1028,tazId_1029,tazId_1030,tazId_1031,tazId_1032,tazId_1033,tazId_1034,tazId_1035,tazId_1036,tazId_1037,tazId_1038,tazId_1039,tazId_1040,tazId_1041,tazId_1042,tazId_1043,tazId_1044,tazId_1045,tazId_1046,tazId_1047,tazId_1048,tazId_1049,tazId_1050,tazId_1051,tazId_1052,tazId_1053,tazId_1054,tazId_1055,tazId_1056,tazId_1057,tazId_1058,tazId_1059,tazId_1060,tazId_1061,tazId_1062,tazId_1063,tazId_1064,tazId_1065,tazId_1066,tazId_1067,tazId_1068,tazId_1069,tazId_1070,tazId_1071,tazId_1072,tazId_1073,tazId_1074,tazId_1075,tazId_1076,tazId_1077,tazId_1078,tazId_1079,tazId_1080,tazId_1081,tazId_1082,tazId_1083,tazId_1084,tazId_1085,tazId_1086,tazId_1087,tazId_1088,tazId_1089,tazId_1090,tazId_1091,tazId_1092,tazId_1093,tazId_1094,tazId_1095,tazId_1096,tazId_1097,tazId_1098,tazId_1099,tazId_1100,tazId_1101,tazId_1102,tazId_1103,tazId_1104,tazId_1105,tazId_1106,tazId_1107,tazId_1108,tazId_1109,tazId_1110,tazId_1111,tazId_1112,tazId_1113,tazId_1114,tazId_1115,tazId_1116,tazId_1117,tazId_1118,tazId_1119,tazId_1120,tazId_1121,tazId_1122,tazId_1123,tazId_1124,tazId_1125,tazId_1126,tazId_1127,tazId_1128,tazId_1129,tazId_1130,tazId_1131,tazId_1132,tazId_1133,tazId_1134,tazId_1135,tazId_1136,tazId_1137,tazId_1138,tazId_1139,tazId_1140,tazId_1141,tazId_1142,tazId_1143,tazId_1144,tazId_1145,tazId_1146,tazId_1147,tazId_1148,tazId_1149,tazId_1150,tazId_1151,tazId_1152,tazId_1153,tazId_1154,tazId_1155,tazId_1156,tazId_1157,tazId_1158,tazId_1159,tazId_1160,tazId_1161,tazId_1162,tazId_1163,tazId_1164,tazId_1165,tazId_1166,tazId_1167,tazId_1168,tazId_1169," << std::endl;

		std::ofstream* individualEduLogsumFile = new std::ofstream("IndividualEduLogsum.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_EDU_LOGSUM, individualEduLogsumFile));
		*individualEduLogsumFile << "title, hitsId, householdId,individualId, paxId,tazId_1,tazId_2,tazId_3,tazId_4,tazId_5,tazId_6,tazId_7,tazId_8,tazId_9,tazId_10,tazId_11,tazId_12,tazId_13,tazId_14,tazId_15,tazId_16,tazId_17,tazId_18,tazId_19,tazId_20,tazId_21,tazId_22,tazId_23,tazId_24,tazId_25,tazId_26,tazId_27,tazId_28,tazId_29,tazId_30,tazId_31,tazId_32,tazId_33,tazId_34,tazId_35,tazId_36,tazId_37,tazId_38,tazId_39,tazId_40,tazId_41,tazId_42,tazId_43,tazId_44,tazId_45,tazId_46,tazId_47,tazId_48,tazId_49,tazId_50,tazId_51,tazId_52,tazId_53,tazId_54,tazId_55,tazId_56,tazId_57,tazId_58,tazId_59,tazId_60,tazId_61,tazId_62,tazId_63,tazId_64,tazId_65,tazId_66,tazId_67,tazId_68,tazId_69,tazId_70,tazId_71,tazId_72,tazId_73,tazId_74,tazId_75,tazId_76,tazId_77,tazId_78,tazId_79,tazId_80,tazId_81,tazId_82,tazId_83,tazId_84,tazId_85,tazId_86,tazId_87,tazId_88,tazId_89,tazId_90,tazId_91,tazId_92,tazId_93,tazId_94,tazId_95,tazId_96,tazId_97,tazId_98,tazId_99,tazId_100,tazId_101,tazId_102,tazId_103,tazId_104,tazId_105,tazId_106,tazId_107,tazId_108,tazId_109,tazId_110,tazId_111,tazId_112,tazId_113,tazId_114,tazId_115,tazId_116,tazId_117,tazId_118,tazId_119,tazId_120,tazId_121,tazId_122,tazId_123,tazId_124,tazId_125,tazId_126,tazId_127,tazId_128,tazId_129,tazId_130,tazId_131,tazId_132,tazId_133,tazId_134,tazId_135,tazId_136,tazId_137,tazId_138,tazId_139,tazId_140,tazId_141,tazId_142,tazId_143,tazId_144,tazId_145,tazId_146,tazId_147,tazId_148,tazId_149,tazId_150,tazId_151,tazId_152,tazId_153,tazId_154,tazId_155,tazId_156,tazId_157,tazId_158,tazId_159,tazId_160,tazId_161,tazId_162,tazId_163,tazId_164,tazId_165,tazId_166,tazId_167,tazId_168,tazId_169,tazId_170,tazId_171,tazId_172,tazId_173,tazId_174,tazId_175,tazId_176,tazId_177,tazId_178,tazId_179,tazId_180,tazId_181,tazId_182,tazId_183,tazId_184,tazId_185,tazId_186,tazId_187,tazId_188,tazId_189,tazId_190,tazId_191,tazId_192,tazId_193,tazId_194,tazId_195,tazId_196,tazId_197,tazId_198,tazId_199,tazId_200,tazId_201,tazId_202,tazId_203,tazId_204,tazId_205,tazId_206,tazId_207,tazId_208,tazId_209,tazId_210,tazId_211,tazId_212,tazId_213,tazId_214,tazId_215,tazId_216,tazId_217,tazId_218,tazId_219,tazId_220,tazId_221,tazId_222,tazId_223,tazId_224,tazId_225,tazId_226,tazId_227,tazId_228,tazId_229,tazId_230,tazId_231,tazId_232,tazId_233,tazId_234,tazId_235,tazId_236,tazId_237,tazId_238,tazId_239,tazId_240,tazId_241,tazId_242,tazId_243,tazId_244,tazId_245,tazId_246,tazId_247,tazId_248,tazId_249,tazId_250,tazId_251,tazId_252,tazId_253,tazId_254,tazId_255,tazId_256,tazId_257,tazId_258,tazId_259,tazId_260,tazId_261,tazId_262,tazId_263,tazId_264,tazId_265,tazId_266,tazId_267,tazId_268,tazId_269,tazId_270,tazId_271,tazId_272,tazId_273,tazId_274,tazId_275,tazId_276,tazId_277,tazId_278,tazId_279,tazId_280,tazId_281,tazId_282,tazId_283,tazId_284,tazId_285,tazId_286,tazId_287,tazId_288,tazId_289,tazId_290,tazId_291,tazId_292,tazId_293,tazId_294,tazId_295,tazId_296,tazId_297,tazId_298,tazId_299,tazId_300,tazId_301,tazId_302,tazId_303,tazId_304,tazId_305,tazId_306,tazId_307,tazId_308,tazId_309,tazId_310,tazId_311,tazId_312,tazId_313,tazId_314,tazId_315,tazId_316,tazId_317,tazId_318,tazId_319,tazId_320,tazId_321,tazId_322,tazId_323,tazId_324,tazId_325,tazId_326,tazId_327,tazId_328,tazId_329,tazId_330,tazId_331,tazId_332,tazId_333,tazId_334,tazId_335,tazId_336,tazId_337,tazId_338,tazId_339,tazId_340,tazId_341,tazId_342,tazId_343,tazId_344,tazId_345,tazId_346,tazId_347,tazId_348,tazId_349,tazId_350,tazId_351,tazId_352,tazId_353,tazId_354,tazId_355,tazId_356,tazId_357,tazId_358,tazId_359,tazId_360,tazId_361,tazId_362,tazId_363,tazId_364,tazId_365,tazId_366,tazId_367,tazId_368,tazId_369,tazId_370,tazId_371,tazId_372,tazId_373,tazId_374,tazId_375,tazId_376,tazId_377,tazId_378,tazId_379,tazId_380,tazId_381,tazId_382,tazId_383,tazId_384,tazId_385,tazId_386,tazId_387,tazId_388,tazId_389,tazId_390,tazId_391,tazId_392,tazId_393,tazId_394,tazId_395,tazId_396,tazId_397,tazId_398,tazId_399,tazId_400,tazId_401,tazId_402,tazId_403,tazId_404,tazId_405,tazId_406,tazId_407,tazId_408,tazId_409,tazId_410,tazId_411,tazId_412,tazId_413,tazId_414,tazId_415,tazId_416,tazId_417,tazId_418,tazId_419,tazId_420,tazId_421,tazId_422,tazId_423,tazId_424,tazId_425,tazId_426,tazId_427,tazId_428,tazId_429,tazId_430,tazId_431,tazId_432,tazId_433,tazId_434,tazId_435,tazId_436,tazId_437,tazId_438,tazId_439,tazId_440,tazId_441,tazId_442,tazId_443,tazId_444,tazId_445,tazId_446,tazId_447,tazId_448,tazId_449,tazId_450,tazId_451,tazId_452,tazId_453,tazId_454,tazId_455,tazId_456,tazId_457,tazId_458,tazId_459,tazId_460,tazId_461,tazId_462,tazId_463,tazId_464,tazId_465,tazId_466,tazId_467,tazId_468,tazId_469,tazId_470,tazId_471,tazId_472,tazId_473,tazId_474,tazId_475,tazId_476,tazId_477,tazId_478,tazId_479,tazId_480,tazId_481,tazId_482,tazId_483,tazId_484,tazId_485,tazId_486,tazId_487,tazId_488,tazId_489,tazId_490,tazId_491,tazId_492,tazId_493,tazId_494,tazId_495,tazId_496,tazId_497,tazId_498,tazId_499,tazId_500,tazId_501,tazId_502,tazId_503,tazId_504,tazId_505,tazId_506,tazId_507,tazId_508,tazId_509,tazId_510,tazId_511,tazId_512,tazId_513,tazId_514,tazId_515,tazId_516,tazId_517,tazId_518,tazId_519,tazId_520,tazId_521,tazId_522,tazId_523,tazId_524,tazId_525,tazId_526,tazId_527,tazId_528,tazId_529,tazId_530,tazId_531,tazId_532,tazId_533,tazId_534,tazId_535,tazId_536,tazId_537,tazId_538,tazId_539,tazId_540,tazId_541,tazId_542,tazId_543,tazId_544,tazId_545,tazId_546,tazId_547,tazId_548,tazId_549,tazId_550,tazId_551,tazId_552,tazId_553,tazId_554,tazId_555,tazId_556,tazId_557,tazId_558,tazId_559,tazId_560,tazId_561,tazId_562,tazId_563,tazId_564,tazId_565,tazId_566,tazId_567,tazId_568,tazId_569,tazId_570,tazId_571,tazId_572,tazId_573,tazId_574,tazId_575,tazId_576,tazId_577,tazId_578,tazId_579,tazId_580,tazId_581,tazId_582,tazId_583,tazId_584,tazId_585,tazId_586,tazId_587,tazId_588,tazId_589,tazId_590,tazId_591,tazId_592,tazId_593,tazId_594,tazId_595,tazId_596,tazId_597,tazId_598,tazId_599,tazId_600,tazId_601,tazId_602,tazId_603,tazId_604,tazId_605,tazId_606,tazId_607,tazId_608,tazId_609,tazId_610,tazId_611,tazId_612,tazId_613,tazId_614,tazId_615,tazId_616,tazId_617,tazId_618,tazId_619,tazId_620,tazId_621,tazId_622,tazId_623,tazId_624,tazId_625,tazId_626,tazId_627,tazId_628,tazId_629,tazId_630,tazId_631,tazId_632,tazId_633,tazId_634,tazId_635,tazId_636,tazId_637,tazId_638,tazId_639,tazId_640,tazId_641,tazId_642,tazId_643,tazId_644,tazId_645,tazId_646,tazId_647,tazId_648,tazId_649,tazId_650,tazId_651,tazId_652,tazId_653,tazId_654,tazId_655,tazId_656,tazId_657,tazId_658,tazId_659,tazId_660,tazId_661,tazId_662,tazId_663,tazId_664,tazId_665,tazId_666,tazId_667,tazId_668,tazId_669,tazId_670,tazId_671,tazId_672,tazId_673,tazId_674,tazId_675,tazId_676,tazId_677,tazId_678,tazId_679,tazId_680,tazId_681,tazId_682,tazId_683,tazId_684,tazId_685,tazId_686,tazId_687,tazId_688,tazId_689,tazId_690,tazId_691,tazId_692,tazId_693,tazId_694,tazId_695,tazId_696,tazId_697,tazId_698,tazId_699,tazId_700,tazId_701,tazId_702,tazId_703,tazId_704,tazId_705,tazId_706,tazId_707,tazId_708,tazId_709,tazId_710,tazId_711,tazId_712,tazId_713,tazId_714,tazId_715,tazId_716,tazId_717,tazId_718,tazId_719,tazId_720,tazId_721,tazId_722,tazId_723,tazId_724,tazId_725,tazId_726,tazId_727,tazId_728,tazId_729,tazId_730,tazId_731,tazId_732,tazId_733,tazId_734,tazId_735,tazId_736,tazId_737,tazId_738,tazId_739,tazId_740,tazId_741,tazId_742,tazId_743,tazId_744,tazId_745,tazId_746,tazId_747,tazId_748,tazId_749,tazId_750,tazId_751,tazId_752,tazId_753,tazId_754,tazId_755,tazId_756,tazId_757,tazId_758,tazId_759,tazId_760,tazId_761,tazId_762,tazId_763,tazId_764,tazId_765,tazId_766,tazId_767,tazId_768,tazId_769,tazId_770,tazId_771,tazId_772,tazId_773,tazId_774,tazId_775,tazId_776,tazId_777,tazId_778,tazId_779,tazId_780,tazId_781,tazId_782,tazId_783,tazId_784,tazId_785,tazId_786,tazId_787,tazId_788,tazId_789,tazId_790,tazId_791,tazId_792,tazId_793,tazId_794,tazId_795,tazId_796,tazId_797,tazId_798,tazId_799,tazId_800,tazId_801,tazId_802,tazId_803,tazId_804,tazId_805,tazId_806,tazId_807,tazId_808,tazId_809,tazId_810,tazId_811,tazId_812,tazId_813,tazId_814,tazId_815,tazId_816,tazId_817,tazId_818,tazId_819,tazId_820,tazId_821,tazId_822,tazId_823,tazId_824,tazId_825,tazId_826,tazId_827,tazId_828,tazId_829,tazId_830,tazId_831,tazId_832,tazId_833,tazId_834,tazId_835,tazId_836,tazId_837,tazId_838,tazId_839,tazId_840,tazId_841,tazId_842,tazId_843,tazId_844,tazId_845,tazId_846,tazId_847,tazId_848,tazId_849,tazId_850,tazId_851,tazId_852,tazId_853,tazId_854,tazId_855,tazId_856,tazId_857,tazId_858,tazId_859,tazId_860,tazId_861,tazId_862,tazId_863,tazId_864,tazId_865,tazId_866,tazId_867,tazId_868,tazId_869,tazId_870,tazId_871,tazId_872,tazId_873,tazId_874,tazId_875,tazId_876,tazId_877,tazId_878,tazId_879,tazId_880,tazId_881,tazId_882,tazId_883,tazId_884,tazId_885,tazId_886,tazId_887,tazId_888,tazId_889,tazId_890,tazId_891,tazId_892,tazId_893,tazId_894,tazId_895,tazId_896,tazId_897,tazId_898,tazId_899,tazId_900,tazId_901,tazId_902,tazId_903,tazId_904,tazId_905,tazId_906,tazId_907,tazId_908,tazId_909,tazId_910,tazId_911,tazId_912,tazId_913,tazId_914,tazId_915,tazId_916,tazId_917,tazId_918,tazId_919,tazId_920,tazId_921,tazId_922,tazId_923,tazId_924,tazId_925,tazId_926,tazId_927,tazId_928,tazId_929,tazId_930,tazId_931,tazId_932,tazId_933,tazId_934,tazId_935,tazId_936,tazId_937,tazId_938,tazId_939,tazId_940,tazId_941,tazId_942,tazId_943,tazId_944,tazId_945,tazId_946,tazId_947,tazId_948,tazId_949,tazId_950,tazId_951,tazId_952,tazId_953,tazId_954,tazId_955,tazId_956,tazId_957,tazId_958,tazId_959,tazId_960,tazId_961,tazId_962,tazId_963,tazId_964,tazId_965,tazId_966,tazId_967,tazId_968,tazId_969,tazId_970,tazId_971,tazId_972,tazId_973,tazId_974,tazId_975,tazId_976,tazId_977,tazId_978,tazId_979,tazId_980,tazId_981,tazId_982,tazId_983,tazId_984,tazId_985,tazId_986,tazId_987,tazId_988,tazId_989,tazId_990,tazId_991,tazId_992,tazId_993,tazId_994,tazId_995,tazId_996,tazId_997,tazId_998,tazId_999,tazId_1000,tazId_1001,tazId_1002,tazId_1003,tazId_1004,tazId_1005,tazId_1006,tazId_1007,tazId_1008,tazId_1009,tazId_1010,tazId_1011,tazId_1012,tazId_1013,tazId_1014,tazId_1015,tazId_1016,tazId_1017,tazId_1018,tazId_1019,tazId_1020,tazId_1021,tazId_1022,tazId_1023,tazId_1024,tazId_1025,tazId_1026,tazId_1027,tazId_1028,tazId_1029,tazId_1030,tazId_1031,tazId_1032,tazId_1033,tazId_1034,tazId_1035,tazId_1036,tazId_1037,tazId_1038,tazId_1039,tazId_1040,tazId_1041,tazId_1042,tazId_1043,tazId_1044,tazId_1045,tazId_1046,tazId_1047,tazId_1048,tazId_1049,tazId_1050,tazId_1051,tazId_1052,tazId_1053,tazId_1054,tazId_1055,tazId_1056,tazId_1057,tazId_1058,tazId_1059,tazId_1060,tazId_1061,tazId_1062,tazId_1063,tazId_1064,tazId_1065,tazId_1066,tazId_1067,tazId_1068,tazId_1069,tazId_1070,tazId_1071,tazId_1072,tazId_1073,tazId_1074,tazId_1075,tazId_1076,tazId_1077,tazId_1078,tazId_1079,tazId_1080,tazId_1081,tazId_1082,tazId_1083,tazId_1084,tazId_1085,tazId_1086,tazId_1087,tazId_1088,tazId_1089,tazId_1090,tazId_1091,tazId_1092,tazId_1093,tazId_1094,tazId_1095,tazId_1096,tazId_1097,tazId_1098,tazId_1099,tazId_1100,tazId_1101,tazId_1102,tazId_1103,tazId_1104,tazId_1105,tazId_1106,tazId_1107,tazId_1108,tazId_1109,tazId_1110,tazId_1111,tazId_1112,tazId_1113,tazId_1114,tazId_1115,tazId_1116,tazId_1117,tazId_1118,tazId_1119,tazId_1120,tazId_1121,tazId_1122,tazId_1123,tazId_1124,tazId_1125,tazId_1126,tazId_1127,tazId_1128,tazId_1129,tazId_1130,tazId_1131,tazId_1132,tazId_1133,tazId_1134,tazId_1135,tazId_1136,tazId_1137,tazId_1138,tazId_1139,tazId_1140,tazId_1141,tazId_1142,tazId_1143,tazId_1144,tazId_1145,tazId_1146,tazId_1147,tazId_1148,tazId_1149,tazId_1150,tazId_1151,tazId_1152,tazId_1153,tazId_1154,tazId_1155,tazId_1156,tazId_1157,tazId_1158,tazId_1159,tazId_1160,tazId_1161,tazId_1162,tazId_1163,tazId_1164,tazId_1165,tazId_1166,tazId_1167,tazId_1168,tazId_1169," << std::endl;

		std::ofstream* individualShopLogsumFile = new std::ofstream("IndividualShopLogsum.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_SHOP_LOGSUM, individualShopLogsumFile));
		*individualShopLogsumFile << "title, hitsId, householdId,individualId, paxId,tazId_1,tazId_2,tazId_3,tazId_4,tazId_5,tazId_6,tazId_7,tazId_8,tazId_9,tazId_10,tazId_11,tazId_12,tazId_13,tazId_14,tazId_15,tazId_16,tazId_17,tazId_18,tazId_19,tazId_20,tazId_21,tazId_22,tazId_23,tazId_24,tazId_25,tazId_26,tazId_27,tazId_28,tazId_29,tazId_30,tazId_31,tazId_32,tazId_33,tazId_34,tazId_35,tazId_36,tazId_37,tazId_38,tazId_39,tazId_40,tazId_41,tazId_42,tazId_43,tazId_44,tazId_45,tazId_46,tazId_47,tazId_48,tazId_49,tazId_50,tazId_51,tazId_52,tazId_53,tazId_54,tazId_55,tazId_56,tazId_57,tazId_58,tazId_59,tazId_60,tazId_61,tazId_62,tazId_63,tazId_64,tazId_65,tazId_66,tazId_67,tazId_68,tazId_69,tazId_70,tazId_71,tazId_72,tazId_73,tazId_74,tazId_75,tazId_76,tazId_77,tazId_78,tazId_79,tazId_80,tazId_81,tazId_82,tazId_83,tazId_84,tazId_85,tazId_86,tazId_87,tazId_88,tazId_89,tazId_90,tazId_91,tazId_92,tazId_93,tazId_94,tazId_95,tazId_96,tazId_97,tazId_98,tazId_99,tazId_100,tazId_101,tazId_102,tazId_103,tazId_104,tazId_105,tazId_106,tazId_107,tazId_108,tazId_109,tazId_110,tazId_111,tazId_112,tazId_113,tazId_114,tazId_115,tazId_116,tazId_117,tazId_118,tazId_119,tazId_120,tazId_121,tazId_122,tazId_123,tazId_124,tazId_125,tazId_126,tazId_127,tazId_128,tazId_129,tazId_130,tazId_131,tazId_132,tazId_133,tazId_134,tazId_135,tazId_136,tazId_137,tazId_138,tazId_139,tazId_140,tazId_141,tazId_142,tazId_143,tazId_144,tazId_145,tazId_146,tazId_147,tazId_148,tazId_149,tazId_150,tazId_151,tazId_152,tazId_153,tazId_154,tazId_155,tazId_156,tazId_157,tazId_158,tazId_159,tazId_160,tazId_161,tazId_162,tazId_163,tazId_164,tazId_165,tazId_166,tazId_167,tazId_168,tazId_169,tazId_170,tazId_171,tazId_172,tazId_173,tazId_174,tazId_175,tazId_176,tazId_177,tazId_178,tazId_179,tazId_180,tazId_181,tazId_182,tazId_183,tazId_184,tazId_185,tazId_186,tazId_187,tazId_188,tazId_189,tazId_190,tazId_191,tazId_192,tazId_193,tazId_194,tazId_195,tazId_196,tazId_197,tazId_198,tazId_199,tazId_200,tazId_201,tazId_202,tazId_203,tazId_204,tazId_205,tazId_206,tazId_207,tazId_208,tazId_209,tazId_210,tazId_211,tazId_212,tazId_213,tazId_214,tazId_215,tazId_216,tazId_217,tazId_218,tazId_219,tazId_220,tazId_221,tazId_222,tazId_223,tazId_224,tazId_225,tazId_226,tazId_227,tazId_228,tazId_229,tazId_230,tazId_231,tazId_232,tazId_233,tazId_234,tazId_235,tazId_236,tazId_237,tazId_238,tazId_239,tazId_240,tazId_241,tazId_242,tazId_243,tazId_244,tazId_245,tazId_246,tazId_247,tazId_248,tazId_249,tazId_250,tazId_251,tazId_252,tazId_253,tazId_254,tazId_255,tazId_256,tazId_257,tazId_258,tazId_259,tazId_260,tazId_261,tazId_262,tazId_263,tazId_264,tazId_265,tazId_266,tazId_267,tazId_268,tazId_269,tazId_270,tazId_271,tazId_272,tazId_273,tazId_274,tazId_275,tazId_276,tazId_277,tazId_278,tazId_279,tazId_280,tazId_281,tazId_282,tazId_283,tazId_284,tazId_285,tazId_286,tazId_287,tazId_288,tazId_289,tazId_290,tazId_291,tazId_292,tazId_293,tazId_294,tazId_295,tazId_296,tazId_297,tazId_298,tazId_299,tazId_300,tazId_301,tazId_302,tazId_303,tazId_304,tazId_305,tazId_306,tazId_307,tazId_308,tazId_309,tazId_310,tazId_311,tazId_312,tazId_313,tazId_314,tazId_315,tazId_316,tazId_317,tazId_318,tazId_319,tazId_320,tazId_321,tazId_322,tazId_323,tazId_324,tazId_325,tazId_326,tazId_327,tazId_328,tazId_329,tazId_330,tazId_331,tazId_332,tazId_333,tazId_334,tazId_335,tazId_336,tazId_337,tazId_338,tazId_339,tazId_340,tazId_341,tazId_342,tazId_343,tazId_344,tazId_345,tazId_346,tazId_347,tazId_348,tazId_349,tazId_350,tazId_351,tazId_352,tazId_353,tazId_354,tazId_355,tazId_356,tazId_357,tazId_358,tazId_359,tazId_360,tazId_361,tazId_362,tazId_363,tazId_364,tazId_365,tazId_366,tazId_367,tazId_368,tazId_369,tazId_370,tazId_371,tazId_372,tazId_373,tazId_374,tazId_375,tazId_376,tazId_377,tazId_378,tazId_379,tazId_380,tazId_381,tazId_382,tazId_383,tazId_384,tazId_385,tazId_386,tazId_387,tazId_388,tazId_389,tazId_390,tazId_391,tazId_392,tazId_393,tazId_394,tazId_395,tazId_396,tazId_397,tazId_398,tazId_399,tazId_400,tazId_401,tazId_402,tazId_403,tazId_404,tazId_405,tazId_406,tazId_407,tazId_408,tazId_409,tazId_410,tazId_411,tazId_412,tazId_413,tazId_414,tazId_415,tazId_416,tazId_417,tazId_418,tazId_419,tazId_420,tazId_421,tazId_422,tazId_423,tazId_424,tazId_425,tazId_426,tazId_427,tazId_428,tazId_429,tazId_430,tazId_431,tazId_432,tazId_433,tazId_434,tazId_435,tazId_436,tazId_437,tazId_438,tazId_439,tazId_440,tazId_441,tazId_442,tazId_443,tazId_444,tazId_445,tazId_446,tazId_447,tazId_448,tazId_449,tazId_450,tazId_451,tazId_452,tazId_453,tazId_454,tazId_455,tazId_456,tazId_457,tazId_458,tazId_459,tazId_460,tazId_461,tazId_462,tazId_463,tazId_464,tazId_465,tazId_466,tazId_467,tazId_468,tazId_469,tazId_470,tazId_471,tazId_472,tazId_473,tazId_474,tazId_475,tazId_476,tazId_477,tazId_478,tazId_479,tazId_480,tazId_481,tazId_482,tazId_483,tazId_484,tazId_485,tazId_486,tazId_487,tazId_488,tazId_489,tazId_490,tazId_491,tazId_492,tazId_493,tazId_494,tazId_495,tazId_496,tazId_497,tazId_498,tazId_499,tazId_500,tazId_501,tazId_502,tazId_503,tazId_504,tazId_505,tazId_506,tazId_507,tazId_508,tazId_509,tazId_510,tazId_511,tazId_512,tazId_513,tazId_514,tazId_515,tazId_516,tazId_517,tazId_518,tazId_519,tazId_520,tazId_521,tazId_522,tazId_523,tazId_524,tazId_525,tazId_526,tazId_527,tazId_528,tazId_529,tazId_530,tazId_531,tazId_532,tazId_533,tazId_534,tazId_535,tazId_536,tazId_537,tazId_538,tazId_539,tazId_540,tazId_541,tazId_542,tazId_543,tazId_544,tazId_545,tazId_546,tazId_547,tazId_548,tazId_549,tazId_550,tazId_551,tazId_552,tazId_553,tazId_554,tazId_555,tazId_556,tazId_557,tazId_558,tazId_559,tazId_560,tazId_561,tazId_562,tazId_563,tazId_564,tazId_565,tazId_566,tazId_567,tazId_568,tazId_569,tazId_570,tazId_571,tazId_572,tazId_573,tazId_574,tazId_575,tazId_576,tazId_577,tazId_578,tazId_579,tazId_580,tazId_581,tazId_582,tazId_583,tazId_584,tazId_585,tazId_586,tazId_587,tazId_588,tazId_589,tazId_590,tazId_591,tazId_592,tazId_593,tazId_594,tazId_595,tazId_596,tazId_597,tazId_598,tazId_599,tazId_600,tazId_601,tazId_602,tazId_603,tazId_604,tazId_605,tazId_606,tazId_607,tazId_608,tazId_609,tazId_610,tazId_611,tazId_612,tazId_613,tazId_614,tazId_615,tazId_616,tazId_617,tazId_618,tazId_619,tazId_620,tazId_621,tazId_622,tazId_623,tazId_624,tazId_625,tazId_626,tazId_627,tazId_628,tazId_629,tazId_630,tazId_631,tazId_632,tazId_633,tazId_634,tazId_635,tazId_636,tazId_637,tazId_638,tazId_639,tazId_640,tazId_641,tazId_642,tazId_643,tazId_644,tazId_645,tazId_646,tazId_647,tazId_648,tazId_649,tazId_650,tazId_651,tazId_652,tazId_653,tazId_654,tazId_655,tazId_656,tazId_657,tazId_658,tazId_659,tazId_660,tazId_661,tazId_662,tazId_663,tazId_664,tazId_665,tazId_666,tazId_667,tazId_668,tazId_669,tazId_670,tazId_671,tazId_672,tazId_673,tazId_674,tazId_675,tazId_676,tazId_677,tazId_678,tazId_679,tazId_680,tazId_681,tazId_682,tazId_683,tazId_684,tazId_685,tazId_686,tazId_687,tazId_688,tazId_689,tazId_690,tazId_691,tazId_692,tazId_693,tazId_694,tazId_695,tazId_696,tazId_697,tazId_698,tazId_699,tazId_700,tazId_701,tazId_702,tazId_703,tazId_704,tazId_705,tazId_706,tazId_707,tazId_708,tazId_709,tazId_710,tazId_711,tazId_712,tazId_713,tazId_714,tazId_715,tazId_716,tazId_717,tazId_718,tazId_719,tazId_720,tazId_721,tazId_722,tazId_723,tazId_724,tazId_725,tazId_726,tazId_727,tazId_728,tazId_729,tazId_730,tazId_731,tazId_732,tazId_733,tazId_734,tazId_735,tazId_736,tazId_737,tazId_738,tazId_739,tazId_740,tazId_741,tazId_742,tazId_743,tazId_744,tazId_745,tazId_746,tazId_747,tazId_748,tazId_749,tazId_750,tazId_751,tazId_752,tazId_753,tazId_754,tazId_755,tazId_756,tazId_757,tazId_758,tazId_759,tazId_760,tazId_761,tazId_762,tazId_763,tazId_764,tazId_765,tazId_766,tazId_767,tazId_768,tazId_769,tazId_770,tazId_771,tazId_772,tazId_773,tazId_774,tazId_775,tazId_776,tazId_777,tazId_778,tazId_779,tazId_780,tazId_781,tazId_782,tazId_783,tazId_784,tazId_785,tazId_786,tazId_787,tazId_788,tazId_789,tazId_790,tazId_791,tazId_792,tazId_793,tazId_794,tazId_795,tazId_796,tazId_797,tazId_798,tazId_799,tazId_800,tazId_801,tazId_802,tazId_803,tazId_804,tazId_805,tazId_806,tazId_807,tazId_808,tazId_809,tazId_810,tazId_811,tazId_812,tazId_813,tazId_814,tazId_815,tazId_816,tazId_817,tazId_818,tazId_819,tazId_820,tazId_821,tazId_822,tazId_823,tazId_824,tazId_825,tazId_826,tazId_827,tazId_828,tazId_829,tazId_830,tazId_831,tazId_832,tazId_833,tazId_834,tazId_835,tazId_836,tazId_837,tazId_838,tazId_839,tazId_840,tazId_841,tazId_842,tazId_843,tazId_844,tazId_845,tazId_846,tazId_847,tazId_848,tazId_849,tazId_850,tazId_851,tazId_852,tazId_853,tazId_854,tazId_855,tazId_856,tazId_857,tazId_858,tazId_859,tazId_860,tazId_861,tazId_862,tazId_863,tazId_864,tazId_865,tazId_866,tazId_867,tazId_868,tazId_869,tazId_870,tazId_871,tazId_872,tazId_873,tazId_874,tazId_875,tazId_876,tazId_877,tazId_878,tazId_879,tazId_880,tazId_881,tazId_882,tazId_883,tazId_884,tazId_885,tazId_886,tazId_887,tazId_888,tazId_889,tazId_890,tazId_891,tazId_892,tazId_893,tazId_894,tazId_895,tazId_896,tazId_897,tazId_898,tazId_899,tazId_900,tazId_901,tazId_902,tazId_903,tazId_904,tazId_905,tazId_906,tazId_907,tazId_908,tazId_909,tazId_910,tazId_911,tazId_912,tazId_913,tazId_914,tazId_915,tazId_916,tazId_917,tazId_918,tazId_919,tazId_920,tazId_921,tazId_922,tazId_923,tazId_924,tazId_925,tazId_926,tazId_927,tazId_928,tazId_929,tazId_930,tazId_931,tazId_932,tazId_933,tazId_934,tazId_935,tazId_936,tazId_937,tazId_938,tazId_939,tazId_940,tazId_941,tazId_942,tazId_943,tazId_944,tazId_945,tazId_946,tazId_947,tazId_948,tazId_949,tazId_950,tazId_951,tazId_952,tazId_953,tazId_954,tazId_955,tazId_956,tazId_957,tazId_958,tazId_959,tazId_960,tazId_961,tazId_962,tazId_963,tazId_964,tazId_965,tazId_966,tazId_967,tazId_968,tazId_969,tazId_970,tazId_971,tazId_972,tazId_973,tazId_974,tazId_975,tazId_976,tazId_977,tazId_978,tazId_979,tazId_980,tazId_981,tazId_982,tazId_983,tazId_984,tazId_985,tazId_986,tazId_987,tazId_988,tazId_989,tazId_990,tazId_991,tazId_992,tazId_993,tazId_994,tazId_995,tazId_996,tazId_997,tazId_998,tazId_999,tazId_1000,tazId_1001,tazId_1002,tazId_1003,tazId_1004,tazId_1005,tazId_1006,tazId_1007,tazId_1008,tazId_1009,tazId_1010,tazId_1011,tazId_1012,tazId_1013,tazId_1014,tazId_1015,tazId_1016,tazId_1017,tazId_1018,tazId_1019,tazId_1020,tazId_1021,tazId_1022,tazId_1023,tazId_1024,tazId_1025,tazId_1026,tazId_1027,tazId_1028,tazId_1029,tazId_1030,tazId_1031,tazId_1032,tazId_1033,tazId_1034,tazId_1035,tazId_1036,tazId_1037,tazId_1038,tazId_1039,tazId_1040,tazId_1041,tazId_1042,tazId_1043,tazId_1044,tazId_1045,tazId_1046,tazId_1047,tazId_1048,tazId_1049,tazId_1050,tazId_1051,tazId_1052,tazId_1053,tazId_1054,tazId_1055,tazId_1056,tazId_1057,tazId_1058,tazId_1059,tazId_1060,tazId_1061,tazId_1062,tazId_1063,tazId_1064,tazId_1065,tazId_1066,tazId_1067,tazId_1068,tazId_1069,tazId_1070,tazId_1071,tazId_1072,tazId_1073,tazId_1074,tazId_1075,tazId_1076,tazId_1077,tazId_1078,tazId_1079,tazId_1080,tazId_1081,tazId_1082,tazId_1083,tazId_1084,tazId_1085,tazId_1086,tazId_1087,tazId_1088,tazId_1089,tazId_1090,tazId_1091,tazId_1092,tazId_1093,tazId_1094,tazId_1095,tazId_1096,tazId_1097,tazId_1098,tazId_1099,tazId_1100,tazId_1101,tazId_1102,tazId_1103,tazId_1104,tazId_1105,tazId_1106,tazId_1107,tazId_1108,tazId_1109,tazId_1110,tazId_1111,tazId_1112,tazId_1113,tazId_1114,tazId_1115,tazId_1116,tazId_1117,tazId_1118,tazId_1119,tazId_1120,tazId_1121,tazId_1122,tazId_1123,tazId_1124,tazId_1125,tazId_1126,tazId_1127,tazId_1128,tazId_1129,tazId_1130,tazId_1131,tazId_1132,tazId_1133,tazId_1134,tazId_1135,tazId_1136,tazId_1137,tazId_1138,tazId_1139,tazId_1140,tazId_1141,tazId_1142,tazId_1143,tazId_1144,tazId_1145,tazId_1146,tazId_1147,tazId_1148,tazId_1149,tazId_1150,tazId_1151,tazId_1152,tazId_1153,tazId_1154,tazId_1155,tazId_1156,tazId_1157,tazId_1158,tazId_1159,tazId_1160,tazId_1161,tazId_1162,tazId_1163,tazId_1164,tazId_1165,tazId_1166,tazId_1167,tazId_1168,tazId_1169," << std::endl;

		std::ofstream* individualOtherLogsumFile = new std::ofstream("IndividualOtherLogsum.csv");
		streams.insert(std::make_pair(LOG_INDIVIDUAL_OTHER_LOGSUM, individualOtherLogsumFile));
		*individualOtherLogsumFile << "title, hitsId, householdId,individualId, paxIdtazId_1,tazId_2,tazId_3,tazId_4,tazId_5,tazId_6,tazId_7,tazId_8,tazId_9,tazId_10,tazId_11,tazId_12,tazId_13,tazId_14,tazId_15,tazId_16,tazId_17,tazId_18,tazId_19,tazId_20,tazId_21,tazId_22,tazId_23,tazId_24,tazId_25,tazId_26,tazId_27,tazId_28,tazId_29,tazId_30,tazId_31,tazId_32,tazId_33,tazId_34,tazId_35,tazId_36,tazId_37,tazId_38,tazId_39,tazId_40,tazId_41,tazId_42,tazId_43,tazId_44,tazId_45,tazId_46,tazId_47,tazId_48,tazId_49,tazId_50,tazId_51,tazId_52,tazId_53,tazId_54,tazId_55,tazId_56,tazId_57,tazId_58,tazId_59,tazId_60,tazId_61,tazId_62,tazId_63,tazId_64,tazId_65,tazId_66,tazId_67,tazId_68,tazId_69,tazId_70,tazId_71,tazId_72,tazId_73,tazId_74,tazId_75,tazId_76,tazId_77,tazId_78,tazId_79,tazId_80,tazId_81,tazId_82,tazId_83,tazId_84,tazId_85,tazId_86,tazId_87,tazId_88,tazId_89,tazId_90,tazId_91,tazId_92,tazId_93,tazId_94,tazId_95,tazId_96,tazId_97,tazId_98,tazId_99,tazId_100,tazId_101,tazId_102,tazId_103,tazId_104,tazId_105,tazId_106,tazId_107,tazId_108,tazId_109,tazId_110,tazId_111,tazId_112,tazId_113,tazId_114,tazId_115,tazId_116,tazId_117,tazId_118,tazId_119,tazId_120,tazId_121,tazId_122,tazId_123,tazId_124,tazId_125,tazId_126,tazId_127,tazId_128,tazId_129,tazId_130,tazId_131,tazId_132,tazId_133,tazId_134,tazId_135,tazId_136,tazId_137,tazId_138,tazId_139,tazId_140,tazId_141,tazId_142,tazId_143,tazId_144,tazId_145,tazId_146,tazId_147,tazId_148,tazId_149,tazId_150,tazId_151,tazId_152,tazId_153,tazId_154,tazId_155,tazId_156,tazId_157,tazId_158,tazId_159,tazId_160,tazId_161,tazId_162,tazId_163,tazId_164,tazId_165,tazId_166,tazId_167,tazId_168,tazId_169,tazId_170,tazId_171,tazId_172,tazId_173,tazId_174,tazId_175,tazId_176,tazId_177,tazId_178,tazId_179,tazId_180,tazId_181,tazId_182,tazId_183,tazId_184,tazId_185,tazId_186,tazId_187,tazId_188,tazId_189,tazId_190,tazId_191,tazId_192,tazId_193,tazId_194,tazId_195,tazId_196,tazId_197,tazId_198,tazId_199,tazId_200,tazId_201,tazId_202,tazId_203,tazId_204,tazId_205,tazId_206,tazId_207,tazId_208,tazId_209,tazId_210,tazId_211,tazId_212,tazId_213,tazId_214,tazId_215,tazId_216,tazId_217,tazId_218,tazId_219,tazId_220,tazId_221,tazId_222,tazId_223,tazId_224,tazId_225,tazId_226,tazId_227,tazId_228,tazId_229,tazId_230,tazId_231,tazId_232,tazId_233,tazId_234,tazId_235,tazId_236,tazId_237,tazId_238,tazId_239,tazId_240,tazId_241,tazId_242,tazId_243,tazId_244,tazId_245,tazId_246,tazId_247,tazId_248,tazId_249,tazId_250,tazId_251,tazId_252,tazId_253,tazId_254,tazId_255,tazId_256,tazId_257,tazId_258,tazId_259,tazId_260,tazId_261,tazId_262,tazId_263,tazId_264,tazId_265,tazId_266,tazId_267,tazId_268,tazId_269,tazId_270,tazId_271,tazId_272,tazId_273,tazId_274,tazId_275,tazId_276,tazId_277,tazId_278,tazId_279,tazId_280,tazId_281,tazId_282,tazId_283,tazId_284,tazId_285,tazId_286,tazId_287,tazId_288,tazId_289,tazId_290,tazId_291,tazId_292,tazId_293,tazId_294,tazId_295,tazId_296,tazId_297,tazId_298,tazId_299,tazId_300,tazId_301,tazId_302,tazId_303,tazId_304,tazId_305,tazId_306,tazId_307,tazId_308,tazId_309,tazId_310,tazId_311,tazId_312,tazId_313,tazId_314,tazId_315,tazId_316,tazId_317,tazId_318,tazId_319,tazId_320,tazId_321,tazId_322,tazId_323,tazId_324,tazId_325,tazId_326,tazId_327,tazId_328,tazId_329,tazId_330,tazId_331,tazId_332,tazId_333,tazId_334,tazId_335,tazId_336,tazId_337,tazId_338,tazId_339,tazId_340,tazId_341,tazId_342,tazId_343,tazId_344,tazId_345,tazId_346,tazId_347,tazId_348,tazId_349,tazId_350,tazId_351,tazId_352,tazId_353,tazId_354,tazId_355,tazId_356,tazId_357,tazId_358,tazId_359,tazId_360,tazId_361,tazId_362,tazId_363,tazId_364,tazId_365,tazId_366,tazId_367,tazId_368,tazId_369,tazId_370,tazId_371,tazId_372,tazId_373,tazId_374,tazId_375,tazId_376,tazId_377,tazId_378,tazId_379,tazId_380,tazId_381,tazId_382,tazId_383,tazId_384,tazId_385,tazId_386,tazId_387,tazId_388,tazId_389,tazId_390,tazId_391,tazId_392,tazId_393,tazId_394,tazId_395,tazId_396,tazId_397,tazId_398,tazId_399,tazId_400,tazId_401,tazId_402,tazId_403,tazId_404,tazId_405,tazId_406,tazId_407,tazId_408,tazId_409,tazId_410,tazId_411,tazId_412,tazId_413,tazId_414,tazId_415,tazId_416,tazId_417,tazId_418,tazId_419,tazId_420,tazId_421,tazId_422,tazId_423,tazId_424,tazId_425,tazId_426,tazId_427,tazId_428,tazId_429,tazId_430,tazId_431,tazId_432,tazId_433,tazId_434,tazId_435,tazId_436,tazId_437,tazId_438,tazId_439,tazId_440,tazId_441,tazId_442,tazId_443,tazId_444,tazId_445,tazId_446,tazId_447,tazId_448,tazId_449,tazId_450,tazId_451,tazId_452,tazId_453,tazId_454,tazId_455,tazId_456,tazId_457,tazId_458,tazId_459,tazId_460,tazId_461,tazId_462,tazId_463,tazId_464,tazId_465,tazId_466,tazId_467,tazId_468,tazId_469,tazId_470,tazId_471,tazId_472,tazId_473,tazId_474,tazId_475,tazId_476,tazId_477,tazId_478,tazId_479,tazId_480,tazId_481,tazId_482,tazId_483,tazId_484,tazId_485,tazId_486,tazId_487,tazId_488,tazId_489,tazId_490,tazId_491,tazId_492,tazId_493,tazId_494,tazId_495,tazId_496,tazId_497,tazId_498,tazId_499,tazId_500,tazId_501,tazId_502,tazId_503,tazId_504,tazId_505,tazId_506,tazId_507,tazId_508,tazId_509,tazId_510,tazId_511,tazId_512,tazId_513,tazId_514,tazId_515,tazId_516,tazId_517,tazId_518,tazId_519,tazId_520,tazId_521,tazId_522,tazId_523,tazId_524,tazId_525,tazId_526,tazId_527,tazId_528,tazId_529,tazId_530,tazId_531,tazId_532,tazId_533,tazId_534,tazId_535,tazId_536,tazId_537,tazId_538,tazId_539,tazId_540,tazId_541,tazId_542,tazId_543,tazId_544,tazId_545,tazId_546,tazId_547,tazId_548,tazId_549,tazId_550,tazId_551,tazId_552,tazId_553,tazId_554,tazId_555,tazId_556,tazId_557,tazId_558,tazId_559,tazId_560,tazId_561,tazId_562,tazId_563,tazId_564,tazId_565,tazId_566,tazId_567,tazId_568,tazId_569,tazId_570,tazId_571,tazId_572,tazId_573,tazId_574,tazId_575,tazId_576,tazId_577,tazId_578,tazId_579,tazId_580,tazId_581,tazId_582,tazId_583,tazId_584,tazId_585,tazId_586,tazId_587,tazId_588,tazId_589,tazId_590,tazId_591,tazId_592,tazId_593,tazId_594,tazId_595,tazId_596,tazId_597,tazId_598,tazId_599,tazId_600,tazId_601,tazId_602,tazId_603,tazId_604,tazId_605,tazId_606,tazId_607,tazId_608,tazId_609,tazId_610,tazId_611,tazId_612,tazId_613,tazId_614,tazId_615,tazId_616,tazId_617,tazId_618,tazId_619,tazId_620,tazId_621,tazId_622,tazId_623,tazId_624,tazId_625,tazId_626,tazId_627,tazId_628,tazId_629,tazId_630,tazId_631,tazId_632,tazId_633,tazId_634,tazId_635,tazId_636,tazId_637,tazId_638,tazId_639,tazId_640,tazId_641,tazId_642,tazId_643,tazId_644,tazId_645,tazId_646,tazId_647,tazId_648,tazId_649,tazId_650,tazId_651,tazId_652,tazId_653,tazId_654,tazId_655,tazId_656,tazId_657,tazId_658,tazId_659,tazId_660,tazId_661,tazId_662,tazId_663,tazId_664,tazId_665,tazId_666,tazId_667,tazId_668,tazId_669,tazId_670,tazId_671,tazId_672,tazId_673,tazId_674,tazId_675,tazId_676,tazId_677,tazId_678,tazId_679,tazId_680,tazId_681,tazId_682,tazId_683,tazId_684,tazId_685,tazId_686,tazId_687,tazId_688,tazId_689,tazId_690,tazId_691,tazId_692,tazId_693,tazId_694,tazId_695,tazId_696,tazId_697,tazId_698,tazId_699,tazId_700,tazId_701,tazId_702,tazId_703,tazId_704,tazId_705,tazId_706,tazId_707,tazId_708,tazId_709,tazId_710,tazId_711,tazId_712,tazId_713,tazId_714,tazId_715,tazId_716,tazId_717,tazId_718,tazId_719,tazId_720,tazId_721,tazId_722,tazId_723,tazId_724,tazId_725,tazId_726,tazId_727,tazId_728,tazId_729,tazId_730,tazId_731,tazId_732,tazId_733,tazId_734,tazId_735,tazId_736,tazId_737,tazId_738,tazId_739,tazId_740,tazId_741,tazId_742,tazId_743,tazId_744,tazId_745,tazId_746,tazId_747,tazId_748,tazId_749,tazId_750,tazId_751,tazId_752,tazId_753,tazId_754,tazId_755,tazId_756,tazId_757,tazId_758,tazId_759,tazId_760,tazId_761,tazId_762,tazId_763,tazId_764,tazId_765,tazId_766,tazId_767,tazId_768,tazId_769,tazId_770,tazId_771,tazId_772,tazId_773,tazId_774,tazId_775,tazId_776,tazId_777,tazId_778,tazId_779,tazId_780,tazId_781,tazId_782,tazId_783,tazId_784,tazId_785,tazId_786,tazId_787,tazId_788,tazId_789,tazId_790,tazId_791,tazId_792,tazId_793,tazId_794,tazId_795,tazId_796,tazId_797,tazId_798,tazId_799,tazId_800,tazId_801,tazId_802,tazId_803,tazId_804,tazId_805,tazId_806,tazId_807,tazId_808,tazId_809,tazId_810,tazId_811,tazId_812,tazId_813,tazId_814,tazId_815,tazId_816,tazId_817,tazId_818,tazId_819,tazId_820,tazId_821,tazId_822,tazId_823,tazId_824,tazId_825,tazId_826,tazId_827,tazId_828,tazId_829,tazId_830,tazId_831,tazId_832,tazId_833,tazId_834,tazId_835,tazId_836,tazId_837,tazId_838,tazId_839,tazId_840,tazId_841,tazId_842,tazId_843,tazId_844,tazId_845,tazId_846,tazId_847,tazId_848,tazId_849,tazId_850,tazId_851,tazId_852,tazId_853,tazId_854,tazId_855,tazId_856,tazId_857,tazId_858,tazId_859,tazId_860,tazId_861,tazId_862,tazId_863,tazId_864,tazId_865,tazId_866,tazId_867,tazId_868,tazId_869,tazId_870,tazId_871,tazId_872,tazId_873,tazId_874,tazId_875,tazId_876,tazId_877,tazId_878,tazId_879,tazId_880,tazId_881,tazId_882,tazId_883,tazId_884,tazId_885,tazId_886,tazId_887,tazId_888,tazId_889,tazId_890,tazId_891,tazId_892,tazId_893,tazId_894,tazId_895,tazId_896,tazId_897,tazId_898,tazId_899,tazId_900,tazId_901,tazId_902,tazId_903,tazId_904,tazId_905,tazId_906,tazId_907,tazId_908,tazId_909,tazId_910,tazId_911,tazId_912,tazId_913,tazId_914,tazId_915,tazId_916,tazId_917,tazId_918,tazId_919,tazId_920,tazId_921,tazId_922,tazId_923,tazId_924,tazId_925,tazId_926,tazId_927,tazId_928,tazId_929,tazId_930,tazId_931,tazId_932,tazId_933,tazId_934,tazId_935,tazId_936,tazId_937,tazId_938,tazId_939,tazId_940,tazId_941,tazId_942,tazId_943,tazId_944,tazId_945,tazId_946,tazId_947,tazId_948,tazId_949,tazId_950,tazId_951,tazId_952,tazId_953,tazId_954,tazId_955,tazId_956,tazId_957,tazId_958,tazId_959,tazId_960,tazId_961,tazId_962,tazId_963,tazId_964,tazId_965,tazId_966,tazId_967,tazId_968,tazId_969,tazId_970,tazId_971,tazId_972,tazId_973,tazId_974,tazId_975,tazId_976,tazId_977,tazId_978,tazId_979,tazId_980,tazId_981,tazId_982,tazId_983,tazId_984,tazId_985,tazId_986,tazId_987,tazId_988,tazId_989,tazId_990,tazId_991,tazId_992,tazId_993,tazId_994,tazId_995,tazId_996,tazId_997,tazId_998,tazId_999,tazId_1000,tazId_1001,tazId_1002,tazId_1003,tazId_1004,tazId_1005,tazId_1006,tazId_1007,tazId_1008,tazId_1009,tazId_1010,tazId_1011,tazId_1012,tazId_1013,tazId_1014,tazId_1015,tazId_1016,tazId_1017,tazId_1018,tazId_1019,tazId_1020,tazId_1021,tazId_1022,tazId_1023,tazId_1024,tazId_1025,tazId_1026,tazId_1027,tazId_1028,tazId_1029,tazId_1030,tazId_1031,tazId_1032,tazId_1033,tazId_1034,tazId_1035,tazId_1036,tazId_1037,tazId_1038,tazId_1039,tazId_1040,tazId_1041,tazId_1042,tazId_1043,tazId_1044,tazId_1045,tazId_1046,tazId_1047,tazId_1048,tazId_1049,tazId_1050,tazId_1051,tazId_1052,tazId_1053,tazId_1054,tazId_1055,tazId_1056,tazId_1057,tazId_1058,tazId_1059,tazId_1060,tazId_1061,tazId_1062,tazId_1063,tazId_1064,tazId_1065,tazId_1066,tazId_1067,tazId_1068,tazId_1069,tazId_1070,tazId_1071,tazId_1072,tazId_1073,tazId_1074,tazId_1075,tazId_1076,tazId_1077,tazId_1078,tazId_1079,tazId_1080,tazId_1081,tazId_1082,tazId_1083,tazId_1084,tazId_1085,tazId_1086,tazId_1087,tazId_1088,tazId_1089,tazId_1090,tazId_1091,tazId_1092,tazId_1093,tazId_1094,tazId_1095,tazId_1096,tazId_1097,tazId_1098,tazId_1099,tazId_1100,tazId_1101,tazId_1102,tazId_1103,tazId_1104,tazId_1105,tazId_1106,tazId_1107,tazId_1108,tazId_1109,tazId_1110,tazId_1111,tazId_1112,tazId_1113,tazId_1114,tazId_1115,tazId_1116,tazId_1117,tazId_1118,tazId_1119,tazId_1120,tazId_1121,tazId_1122,tazId_1123,tazId_1124,tazId_1125,tazId_1126,tazId_1127,tazId_1128,tazId_1129,tazId_1130,tazId_1131,tazId_1132,tazId_1133,tazId_1134,tazId_1135,tazId_1136,tazId_1137,tazId_1138,tazId_1139,tazId_1140,tazId_1141,tazId_1142,tazId_1143,tazId_1144,tazId_1145,tazId_1146,tazId_1147,tazId_1148,tazId_1149,tazId_1150,tazId_1151,tazId_1152,tazId_1153,tazId_1154,tazId_1155,tazId_1156,tazId_1157,tazId_1158,tazId_1159,tazId_1160,tazId_1161,tazId_1162,tazId_1163,tazId_1164,tazId_1165,tazId_1166,tazId_1167,tazId_1168,tazId_1169," << std::endl;
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
		*individualHitsLogsumForVOFile << "hitsId, paxId, householdId, individualId,  employmentStatusId, ageCategoryId, income, fixed_workplace	memberId , tazH , tazW , vo_logsum0 , vo_logsum1 ,vo_logsum2 , vo_logsum3 , vo_logsum4, vo_logsum5" << std::endl;
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

		std::ofstream* hhChoiceSetFile2 = new std::ofstream("HHChoiceSet2.csv");
		streams.insert(std::make_pair(LOG_HHCHOICESET2, hhChoiceSetFile2));

		*hhChoiceSetFile2 << "day, householdId, unitId"<<std::endl;

    }

    if(log_error)
    {
		//errors
		std::ofstream* errorFile = new std::ofstream("Errors.csv");
		streams.insert(std::make_pair(LOG_ERROR, errorFile));
    }

    if(log_school_assignment)
    {
		//primary school assignment
		std::ofstream* primarySchoolAssignmentFile = new std::ofstream("primarySchools.csv");
		streams.insert(std::make_pair(LOG_PRIMARY_SCHOOL_ASSIGNMENT, primarySchoolAssignmentFile));
		*primarySchoolAssignmentFile << "individualId, primarySchoolId" << std::endl;

		//seconary school assignment
		std::ofstream* secondarySchoolAssignmentFile = new std::ofstream("secondarySchools.csv");
		streams.insert(std::make_pair(LOG_SECONDARY_SCHOOL_ASSIGNMENT, secondarySchoolAssignmentFile));
		*secondarySchoolAssignmentFile << "individualId, secondarySchoolId" << std::endl;

		//university assignment
		std::ofstream* uniAssignmentFile = new std::ofstream("universities.csv");
		streams.insert(std::make_pair(LOG_UNIVERSITY_ASSIGNMENT, uniAssignmentFile));
		*uniAssignmentFile << "individualId, universityId" << std::endl;

		//polytech assignment
		std::ofstream* polyTechAssignmentFile = new std::ofstream("polyTechnics.csv");
		streams.insert(std::make_pair(LOG_POLYTECH_ASSIGNMENT, polyTechAssignmentFile));
		*polyTechAssignmentFile << "individualId, polyTechId" << std::endl;

		//school desks
		std::ofstream* schoolDesksFile = new std::ofstream("schoolDesks.csv");
		streams.insert(std::make_pair(LOG_SCHOOL_DESK, schoolDesksFile));
		*schoolDesksFile << "individualId, schoolId, schoolDeskId" << std::endl;

    }

    if(log_pre_school_assignment)
    {
		//pre school assignment
		std::ofstream* preSchoolAssignmentFile = new std::ofstream("preSchools.csv");
		streams.insert(std::make_pair(LOG_PRE_SCHOOL_ASSIGNMENT, preSchoolAssignmentFile));

		*preSchoolAssignmentFile << "individual_id, pre_school_id" << std::endl;
    }

    if(log_hh_awakening)
    {
		//awakenings
		std::ofstream* hhawakeningFile = new std::ofstream("HH_Awakenings.csv");
		streams.insert(std::make_pair(LOG_HH_AWAKENING, hhawakeningFile));

		*hhawakeningFile << "awakening_day, householdId, TimeOnMarket, ageCategory, tenureStatus, futureTransitionPercentage, randomDrawFutureTransition, movingPercentage, randomDrawMovingRate" << std::endl;
		//*hhawakeningFile << "awakening_day, householdId, TimeOnMarket, ageCategory, tenureStatus" << std::endl;
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

	 //job assignment probs
	 std::ofstream* jobAsignProbsFile = new std::ofstream("jobAssignProbs.csv");
	 streams.insert(std::make_pair(LOG_JOB_ASIGN_PROBS, jobAsignProbsFile));
	 *jobAsignProbsFile << "individualId, tazId, probability" << std::endl;

	 //individual job assignments
	 std::ofstream* indJobAsignFile = new std::ofstream("indJobAssign.csv");
	 streams.insert(std::make_pair(LOG_INDIVIDUAL_JOB_ASSIGN, indJobAsignFile));
	 *indJobAsignFile << "individualId, jobId" << std::endl;


	 //daily hm units
	 std::ofstream* dailyHousingMarketUnitsFile = new std::ofstream("dailyHousingMarketUnits.csv");
	 streams.insert(std::make_pair(LOG_DAILY_HOUSING_MARKET_UNITS, dailyHousingMarketUnitsFile));
	 *dailyHousingMarketUnitsFile << "day, unitId" << std::endl;

	 //unit time on-off values
	 std::ofstream* unitTimesFile = new std::ofstream("unitTimes.csv");
	 streams.insert(std::make_pair(LOG_DAILY_HOUSING_MARKET_UNIT_TIMES, unitTimesFile));
	 *unitTimesFile << "unitId, timeOnMarket, timeOffMarket, biddingMarketEntryDay" << std::endl;

	 //unit hedonic price values
	std::ofstream* unitHedonicPriceFile = new std::ofstream("unitHedonicPrices.csv");
	streams.insert(std::make_pair(LOG_UNIT_HEDONIC_PRICE, unitHedonicPriceFile));
	 *unitHedonicPriceFile << "unitId, hedonicPrice" << std::endl;

	//new bids
	std::ofstream* newBidsFile = new std::ofstream("bidderSideBids.csv");
        streams.insert(std::make_pair(LOG_NEW_BIDS, newBidsFile));
         *newBidsFile << "bidId,currentUnitId,newUnitId,bidderId,bidValue,simday" << std::endl;

	 //ezlink stops with nearest uni
	 std::ofstream* ezLinkStopsWithNearestUniFile = new std::ofstream("ezLinkStopsWithNearestUni.csv");
	 streams.insert(std::make_pair(LOG_NEARSET_UNI_EZ_LINK, ezLinkStopsWithNearestUniFile));
	 *ezLinkStopsWithNearestUniFile << "ezLinkStopId, uniId" << std::endl;

	 //ezlink stops with nearest poly
	 std::ofstream* eLinkStopsWithNearestPolyFile = new std::ofstream("ezLinkStopsWithNearestPoly.csv");
	 streams.insert(std::make_pair(LOG_NEARSET_POLYTECH_EZ_LINK, eLinkStopsWithNearestPolyFile));
	 *eLinkStopsWithNearestPolyFile << "ezLinkStopId, polyId" << std::endl;
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
