//-*-c++-*------------------------------------------------------------
// FILE: Constants.h
// AUTH: Qi Yang
// DATE: Fri Oct 20 22:52:55 1995
// WARNING:: Do NOT modify this file.  If you want to add or change
//           something, please ask me first.
//--------------------------------------------------------------------

#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER

typedef unsigned int UINT;

namespace sim_mob {

struct LaneSide {
	bool left;
	bool right;
	bool both() const { return left && right; }
	bool leftOnly() const { return left && !right; }
	bool rightOnly() const { return right && !left; }
};

//Which lane should we change to? Includes "none".
enum LANE_CHANGE_SIDE {
	LCS_LEFT = -1,
	LCS_SAME = 0,
	LCS_RIGHT = 1
};

enum LANE_CHANGE_MODE {	//as a mask
	DLC = 0,
	MLC = 2,
	MLC_C = 4,
	MLC_F = 6
};
enum TARGET_GAP {
	TG_Same = 0,
	TG_Left_Fwd = -3,
	TG_Left_Back = -1,
	TG_Left_Adj = -2,
	TG_Right_Fwd = 3,
	TG_Right_Back = 1,
	TG_Right_Adj  = 2
};

#define STATUS_LEFT_LANE "STATUS_LEFT_LANE"
#define STATUS_RIGHT_LANE "STATUS_RIGHT_LANE"
#define STATUS_CURRENT_LANE "STATUS_CURRENT_LANE"

const float MAX_ACCELERATION		= +10.0; // meter/sec2
const float MAX_DECELERATION		= -10.0; // meter/sec2
const float CF_CRITICAL_TIMER_RATIO	= 0.5;

const unsigned int FLAG_ESCAPE		      = 0x00000003;	// sum  //mandatory lc
const unsigned int FLAG_ESCAPE_RIGHT      = 0x00000001;
const unsigned int FLAG_ESCAPE_LEFT	      = 0x00000002;

const unsigned int FLAG_AVOID		      = 0x0000000C;	// sum  //discretionary lc
const unsigned int FLAG_AVOID_RIGHT       = 0x00000004;
const unsigned int FLAG_AVOID_LEFT	      = 0x00000008;

const unsigned int FLAG_BLOCK	          = 0x00000030;	// sum
const unsigned int FLAG_BLOCK_RIGHT       = 0x00000010;
const unsigned int FLAG_BLOCK_LEFT	      = 0x00000020;

const unsigned int FLAG_PREV_LC           = 0x00000300;	// sum
const unsigned int FLAG_PREV_LC_RIGHT     = 0x00000100;
const unsigned int FLAG_PREV_LC_LEFT      = 0x00000200;

const unsigned int FLAG_NOSING_FEASIBLE   = 0x00000400;

const unsigned int FLAG_YIELDING          = 0x00003000;	// sum
const unsigned int FLAG_YIELDING_RIGHT    = 0x00001000;
const unsigned int FLAG_YIELDING_LEFT     = 0x00002000;

const unsigned int FLAG_NOSING            = 0x0000C000;	// sum
const unsigned int FLAG_NOSING_RIGHT      = 0x00004000;
const unsigned int FLAG_NOSING_LEFT       = 0x00008000;

const unsigned int FLAG_MERGING           = 0x00010000;
const unsigned int FLAG_STUCK_AT_END      = 0x00020000;

const unsigned int FLAG_LC_FAILED         = 0x000C0000;	// sum
const unsigned int FLAG_LC_FAILED_LEAD    = 0x00040000;
const unsigned int FLAG_LC_FAILED_LAG     = 0x00080000;

const unsigned int FLAG_VMS_LANE_USE_BITS = 0x06F00000;	// sum
const unsigned int FLAG_VMS_LANE_USE_DIR  = 0x06000000;	// sum
const unsigned int FLAG_VMS_LANE_USE_RIGHT= 0x02000000;
const unsigned int FLAG_VMS_LANE_USE_LEFT = 0x04000000;
const unsigned int FLAG_VMS_LANE_USE_PIVOT= 0x00F00000;	// pivot index

// These macros are used to get and set lane use pivot index

#define VmsLaneUsePivotToIndex(p) (((p) & 0x000F0000) >> 16)
#define VmsLaneUseIndexToPivot(n) ((n) << 16)

const unsigned int FLAG_PROCESSED         = 0x10000000;
const unsigned int FLAG_IN_HOME           = 0x20000000;
const unsigned int FLAG_SHINE             = 0x40000000;	// for trace
const unsigned int FLAG_DESIRED_SPEED     = 0x80000000; // assigned in trace

// Tracking number of vehicles to yield

const unsigned YIELD_TYPE_CONNECTION	= 0x10000000;
const unsigned YIELD_TYPE_ESCAPE		= 0x20000000;
const unsigned YIELD_COUNTER	        = 0x0000000F;
const unsigned YIELD_REASON             = 0xF0000000;
#define YieldSignatureToLaneIndex(c)	(((c) & 0x0FFFFFF0) >> 4)
#define LaneIndexToYieldSignature(c)	(((c) & 0x0FFFFFF0) << 4)
/*------------------------------------------------------------------------
 * When a variable has a value less than this value, it is treated
 * as zero.
 */

const double SPEED_EPSILON = 1.0E-3; /* meter/sec */
const double ACC_EPSILON   = 1.0E-3; /* meter/sec2 */
const double DIS_EPSILON   = 1.0E-0; /* meter */
const double TIME_EPSILON  = 1.0E-2; /* second */


/*--------------------------------------------------------------------
 *  IDs for controller types
 */

const UINT CONTROLLER_UNSIGNALIZED	= 0; // enumeration start
const UINT CONTROLLER_PRETIMED		= 1;
const UINT CONTROLLER_ACTIVATED		= 2;
const UINT CONTROLLER_GENERIC		= 20; // (Angus)

//Algorithms for Ramp meters
const  UINT CONTROLLER_RESPONSIVE_LOCAL   = 3;      //Alinea
const  UINT CONTROLLER_RESPONSIVE_AREA    = 4;      // Matlab program
const  UINT CONTROLLER_RESPONSIVE_BILEVEL = 5;      // Area + Local
const  UINT CONTROLLER_RESPONSIVE_FLOW    = 6;      // BP/B CA/T design
const  UINT CONTROLLER_RESPONSIVE_BOTTLENECK   = 7;  // METAlinea
const  UINT CONTROLLER_RESPONSIVE_ENHANCEDBOTTLENECK   = 8;  // METAlinea with variable bottleneck

//Algorithms urban street signals (all responsive)
const  UINT CONTROLLER_WEBSTER     = 10;
const  UINT CONTROLLER_SMITH       = 11;
const  UINT CONTROLLER_DELAYMIN    = 12;
const  UINT CONTROLLER_BILEVELDELAYMIN   = 13;     // enumeration end

const double DEFAULT_DIS_TO_STOP = 1000;
/*--------------------------------------------------------------------
 * IDs for control devices and events
 */

const UINT CTRL_LINKWIDE	= 0x00000100; // 256

const UINT CTRL_SIGNAL_TYPE	= 0x000000FF; // mask the last 8 bits
const UINT CTRL_SIGNALS		= 0x0000000F; // mask the last 4 bits

const UINT CTRL_TS		= 0x00000001; // 1 enumeration start
const UINT CTRL_PS		= 0x00000002; // 2
const UINT CTRL_VSLS		= 0x00000003; // 3
const UINT CTRL_VMS		= 0x00000004; // 4
const UINT CTRL_LUS		= 0x0000000A; // 10
const UINT CTRL_RAMPMETER	= 0x0000000C; // 12
const UINT CTRL_TOLLBOOTH	= 0x00000010; // 16

const UINT CTRL_BUSSTOP 	= 0x00000020; // 32  margaret: bus stops

const UINT CTRL_INCIDENT	= 0x00000040; // 64 enumeration end


/*--------------------------------------------------------------------
 * Status IDs for traffic signals
 */

const UINT SIGNAL_BITS		= 4; // UINT = 32 bits
const UINT MAX_NUM_OF_IN_LINKS	= 6; // it could be upto 8
const UINT MAX_NUM_OF_OUT_LINKS	= 6; // it could be upto 8

const UINT SIGNAL_STATE		= 0x0000000F;

const UINT SIGNAL_COLOR		= 0x00000003; // mask the last 2 bits
const UINT SIGNAL_BLANK		= 0x00000000; // enumeration start
const UINT SIGNAL_RED		= 0x00000001;
const UINT SIGNAL_YELLOW	= 0x00000002;
const UINT SIGNAL_GREEN		= 0x00000003; // enumeration start

const UINT SIGNAL_USAGE		= 0x0000000C; // sum

// Signals at intersection

const UINT SIGNAL_ARROW		= 0x00000004; // the 3rd bit
const UINT SIGNAL_FLASHING	= 0x00000008; // the 4th bit

// Lane use signals

const UINT SIGNAL_RIGHT		= 0x00000010; // right arrow
const UINT SIGNAL_LEFT		= 0x00000020; // left arrow

/*--------------------------------------------------------------------
 * Message IDs for static and variable message signs.
 */

const UINT SIGN_ERROR		= 0xFFFFFFFF;

const UINT SIGN_TYPE		= 0xF0000000;

// The lowest 16 bits (the lowest 4 hex digits) is used to specify the
// subject the given message concerns (e.g. the id of a link, a vehicle
// type)

const UINT SIGN_PREFIX		= 0xFFFF0000;
const UINT SIGN_SUFFIX		= 0x0000FFFF;

#define SignPrefix(s)		((s) & 0xFFFF0000)
#define SignSuffix(s)		((s) & 0x0000FFFF)

// Actions: apply to the lane changes related both rule and path

const UINT SIGN_LANE_USE_LANES	= 0x00F00000; // number of lanes
const UINT SIGN_LANE_USE_DIR	= 0x03000000; // direction (sum)
const UINT SIGN_LANE_USE_DIR_R	= 0x01000000; // right
const UINT SIGN_LANE_USE_DIR_L	= 0x02000000; // left

#define SignLaneUseNumLanes(s) (((s) & 0x00F00000) >> 20)

// If this bit is set, the vehicles that DO NOT MEET the chooser
// condition apply the specified action.  If this bit is set, the
// vehicles MEET the chooser condition apply the action.

const UINT SIGN_NEGATIVE	= 0x04000000; // reverse chooser

// If this bit is set, all vehicles meet the condition will comply
// regardless their compliance attribute

const UINT SIGN_MANDATORY	= 0x08000000;

// Following sign constants should use only the highest 4 bits (the
// highest 1 hex digit) for sign type and the lowest 16 low bits (the
// lowest 4 hex digit) for information of choosing vehicles.

// Chooser: Lane use related to rules

const UINT SIGN_LANE_USE_RULE	= 0x10000000;
const UINT SIGN_LANE_USE_CLASS	= 0x0000000F; // vehicle class (sum)
const UINT SIGN_LANE_USE_GROUP	= 0x0000FFF0; // vehicle group (sum)

// Chooser: Lane use related to paths

const UINT SIGN_LANE_USE_PATH	= 0x20000000;
const UINT SIGN_LANE_USE_DEPTH	= 0x000F0000; // max links to search

#define SignLaneUseNumLinks(s)	(((s) & 0x000F0000) >> 16)

// Route guidance

const UINT SIGN_PATH		= 0x30000000;
const UINT SIGN_PATH_DN_INDEX   = 0x0F000000; // which dn link
const UINT SIGN_PATH_COMPLY     = 0x00F00000; // compliance rate
const UINT SIGN_PATH_DEPTH	= 0x000F0000; // max links to search

#define SignPathDnIndex(s)	(((s) & 0x0F000000) >> 24)
#define SignPATHComply(s)	(((s) & 0x00F00000) >> 20)
#define SignPathNumLinks(s)	(((s) & 0x000F0000) >> 16)

// Message for calling route switch model

const UINT SIGN_ENROUTE		    = 0x40000000;
const UINT SIGN_ENROUTE_CLASS	= 0x0F000000;
const UINT SIGN_ENROUTE_COMPLY	= 0x00F00000;
const UINT SIGN_ENROUTE_TIMETAG	= 0x000FFFFF;

#define SignEnrouteClass(s)	    (((s) & 0x0F000000) >> 24)
#define SignEnrouteComply(s)	(((s) & 0x00F00000) >> 20)
#define SignEnrouteTimeTag(s)	((s) & 0x000FFFFF)


/*--------------------------------------------------------------------
 * Severity and state of incident
 */

const UINT INCI_SEVERITY	= 0x0000000F; // mask the last 4 bits
const UINT INCI_IGNORED		= 0x00000000; // enumeration start
const UINT INCI_MINOR		= 0x00000001;
const UINT INCI_MAJOR		= 0x00000002; // enumeration end

const UINT INCI_STATE		= 0x00000070; // 112 sum
const UINT INCI_ACTIVE		= 0x00000010; // 16
const UINT INCI_KNOWN		= 0x00000020; // 32
const UINT INCI_REMOVED		= 0x00000040; // 64
const UINT INCI_STOP            = 0x00000100; // 256

/*--------------------------------------------------------------------
 * Surveillance sensors.  These data should be consistent with
 * headings written into output file by TS_FileManager, TS_Network;
 * data saved, sent, and received by TC_Sensor or its inherited
 * classes.
 */

const UINT SENSOR_LINKWIDE	   = 0x00000100;
const UINT SENSOR_DEVICE_TYPE  = 0x000000FF; // NOT used at this time
const UINT SENSOR_FOR_EQUIPPED_BUS = 0x00000010; //IEM(Jul2)

/* Data to be reported periodically
 */

const UINT SENSOR_AGGREGATE    = 0x0000000F;
const UINT SENSOR_ACOUNT       = 0x00000001;
const UINT SENSOR_ASPEED       = 0x00000002;
const UINT SENSOR_AOCCUPANCY   = 0x00000004;
#define SENSOR_ACODE(c) (SENSOR_AGGREGATE & (c))

/* Data to be reported immediately
 */

const UINT SENSOR_INDIVIDUAL   = 0x0000FFF0;
const UINT SENSOR_IID	       = 0x00000010;
const UINT SENSOR_ITYPE	       = 0x00000020;
const UINT SENSOR_IDEP         = 0x00000040;
const UINT SENSOR_IORI	       = 0x00000080;
const UINT SENSOR_IDES	       = 0x00000100;
const UINT SENSOR_IPATH	       = 0x00000200;
const UINT SENSOR_IHEIGHT      = 0x00000400;
const UINT SENSOR_ISPEED       = 0x00000800;
#define SENSOR_ICODE(c) ((SENSOR_INDIVIDUAL & (c)) >> 4)

const UINT SENSOR_SNAPSHOT     = 0x0FFF0000;
const UINT SENSOR_SID	       = 0x00010000;
const UINT SENSOR_STYPE	       = 0x00020000;
const UINT SENSOR_SHEIGHT      = 0x00400000;
const UINT SENSOR_SSPEED       = 0x00800000;
const UINT SENSOR_SLANE        = 0x01000000;
const UINT SENSOR_SPOSITION    = 0x02000000;
#define SENSOR_SCODE(c) ((SENSOR_SNAPSHOT & (c)) >> 16)

/*--------------------------------------------------------------------
 * State IDs for sensor devices
 */

const UINT SENSOR_ACTIVATED             = 0x0001;
const UINT SENSOR_BROKEN                = 0x0002;
const UINT SENSOR_DISABLED              = 0x0004;

const UINT SENSOR_INC_FLAG              = 0x0010; // 1=declarable
const UINT SENSOR_INC_DCR_PERSIST_COUNT = 0x0F00; // consecutive count
const UINT SENSOR_INC_DCR_PERSIST_ONE   = 0x0100;
const UINT SENSOR_INC_CLR_PERSIST_COUNT = 0xF000; // consecutive count
const UINT SENSOR_INC_CLR_PERSIST_ONE   = 0x1000;

#define IncDcrPersistCount(f) (((f) & SENSOR_INC_DCR_PERSIST_COUNT) >> 8)
#define IncClrPersistCount(f) (((f) & SENSOR_INC_CLR_PERSIST_COUNT) >> 12)

/*--------------------------------------------------------------------
 * These constants are used to mask the vehicle types in terms of
 * acceleration/deceleration profile, lane use privilege, and
 * information availability, etc.
 */

const UINT VEHICLE_CLASS	= 0x0000000F; // 15
const UINT VEHICLE_GROUP	= 0x0000FFF0;

const UINT VEHICLE_LANE_USE	= 0x000007F0; // 2032 sum

const UINT VEHICLE_SMALL	= 0x00000010; // 16
const UINT VEHICLE_LOW		= 0x00000020; // 32

const UINT VEHICLE_ETC		= 0x00000040; // 64
const UINT VEHICLE_HOV		= 0x00000080; // 128
const UINT VEHICLE_COMMERCIAL	= 0x00000100; // 256   // Dan - eligible for bus lane
const UINT VEHICLE_BUS_RAPID  	= 0x00000200; // 512   // Dan - bus rapid transit bus
const UINT VEHICLE_EMERGENCY	= 0x00000400; // 1024  // Dan - not used (4/9/02)

const UINT VEHICLE_GUIDED	= 0x00000800; // 2048
const UINT VEHICLE_FIXEDPATH    = 0x00001000; // 4096
const UINT VEHICLE_PROBE	= 0x00002000; // 8192
const UINT VEHICLE_CELLULAR	= 0x00004000; // 16384


/*--------------------------------------------------------------------
 * These constants are used with variable 'status' to mask
 * the vehicle status.
 */

const UINT STATUS_RIGHT_OK		= 0x00000001;
const UINT STATUS_CURRENT_OK		= 0x00000002;
const UINT STATUS_LEFT_OK		= 0x00000004;

// choose target lane
const UINT STATUS_CHANGING		= 0x00000018; // 24 sum
const UINT STATUS_RIGHT			= 0x00000008;
const UINT STATUS_LEFT			= 0x00000010;

// doing lane change
const UINT STATUS_LC_CHANGING		= 0x00300000; // 3 sum
const UINT STATUS_LC_RIGHT			= 0x00100000;
const UINT STATUS_LC_LEFT			= 0x00200000;

const UINT STATUS_MANDATORY		= 0x00000020; // 32

const UINT STATUS_REGIME		= 0x00000180; // sum 384
const UINT STATUS_REGIME_FREEFLOWING	= 0x00000000;
const UINT STATUS_REGIME_CARFOLLOWING	= 0x00000080; // 128
const UINT STATUS_REGIME_EMERGENCY	= 0x00000100; // 256

const UINT STATUS_ALREADY_PAID          = 0x00000200; // 512 tomer for toll plaza

const UINT STATUS_STOPPED		= 0x00000400; // 1024
const UINT STATUS_LANE_CHANGED		= 0x00000800; // 2048

const UINT STATUS_BUS_SLOWING           = 0x00001000; // Dan, 4096
const UINT STATUS_BUS_DEPARTING         = 0x00002000; // Dan, 8192

const UINT STATUS_FORWARD               = 0x00010000; // tomer
const UINT STATUS_BACKWARD              = 0x00020000; // tomer
const UINT STATUS_ADJACENT              = 0x00040000; // tomer
const UINT STATUS_TARGET_GAP            = 0x00070000; // tomer sum
/*------------------------------------------------------------------------
 * These constants are used to represent the current state of network
 * objects.
 */

const UINT STATE_MOE_SELECTED	= 0x00000100;

const UINT STATE_MULTI_SELECTED	= 0x00000010;

const UINT STATE_VISIBLE	= 0x00001000;
const UINT STATE_SELECTED	= 0x00002000;
const UINT STATE_MARKED		= 0x00008000;

}// end sim_mob
#endif
