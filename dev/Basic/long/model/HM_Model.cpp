/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on October 21, 2013, 3:08 PM
 */

#include "HM_Model.hpp"
#include <boost/unordered_map.hpp>
#include "util/LangHelpers.hpp"
#include "database/DB_Connection.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/IndividualDao.hpp"
#include "database/dao/AwakeningDao.hpp"
#include "database/dao/PostcodeDao.hpp"
#include "database/dao/VehicleOwnershipCoefficientsDao.hpp"
#include "database/dao/TaxiAccessCoefficientsDao.hpp"
#include "database/dao/EstablishmentDao.hpp"
#include "database/dao/JobDao.hpp"
#include "database/dao/HousingInterestRateDao.hpp"
#include "database/dao/LogSumVehicleOwnershipDao.hpp"
#include "database/dao/DistanceMRTDao.hpp"
#include "database/dao/TazDao.hpp"
#include "database/dao/HouseHoldHitsSampleDao.hpp"
#include "database/dao/TazLogsumWeightDao.hpp"
#include "database/dao/LogsumMtzV2Dao.hpp"
#include "database/dao/PlanningAreaDao.hpp"
#include "database/dao/PlanningSubzoneDao.hpp"
#include "database/dao/MtzDao.hpp"
#include "database/dao/MtzTazDao.hpp"
#include "database/dao/AlternativeDao.hpp"
#include "database/dao/Hits2008ScreeningProbDao.hpp"
#include "database/dao/ZonalLanduseVariableValuesDao.hpp"
#include "database/dao/PopulationPerPlanningAreaDao.hpp"
#include "database/dao/HitsIndividualLogsumDao.hpp"
#include "database/dao/IndvidualVehicleOwnershipLogsumDao.hpp"
#include "database/dao/AccessibilityFixedPzidDao.hpp"
#include "database/dao/ScreeningCostTimeDao.hpp"
#include "database/dao/TenureTransitionRateDao.hpp"
#include "database/dao/OwnerTenantMovingRateDao.hpp"
#include "database/dao/SimulationStoppedPointDao.hpp"
#include "database/dao/BidDao.hpp"
#include "database/dao/VehicleOwnershipChangesDao.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "event/SystemEvents.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"
#include "util/HelperFunctions.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "message/LT_Message.hpp"
#include "message/MessageBus.hpp"
#include "behavioral/PredayLT_Logsum.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using namespace sim_mob::messaging;
using std::vector;
using std::map;
using boost::unordered_map;

using std::string;

namespace
{
	const string MODEL_NAME = "Housing Market Model";

	enum RESIDENTIAL_STATUS
	{
		RESIDENT = 1,
		EMPLYMENT_PASS,
		SP_PASS,
		W_PASS,
		DORM_WORKERS,
		DORM_STUDENTS,
		CROSSBORDER
	};

	enum AGE_CATEGORY
	{
		MINOR = 21, YOUNG_ADULT = 45, MIDDLE_AGED_ADULT = 65,
	};

	enum GENDER
	{
		MALE = 1, FEMALE = 2
	};

	enum INCOME_CEILING
	{
		TWOBEDROOM = 5000, THREEBEDROOM = 10000, THREEBEDROOMMATURE = 15000
	};

	enum TaxiAccessParamId
	{
		INTERCEPT = 1, HDB1, AGE5064_1, AGE5064_2, AGE65UP_1, AGE65UP_2, AGE3549_2, AGE1019_2, EMPLOYED_SELF_1, EMPLOYED_SELF_2, INC_LOW, INC_HIGH, RETIRED_1, RETIRED_2, OPERATOR_1,
	    OPERATOR_2, SERVICE_2, PROF_1, LABOR_1, MANAGER_1, INDIAN_TAXI_ACCESS, MALAY_TAXI_ACCESS
	};

	const int YEAR = 365;
	

	//These three units are from the unit_type table
	//"7";"less than 70 Apartment"
	//"52";"larger than 379 Mixed R and C"
	const int LS70_APT = 7;
	const int LG379_RC = 52;
	const int NON_RESIDENTIAL_PROPERTY = 66;
	const std::string LOG_TAXI_AVAILABILITY = "%1%";

	inline void writeTaxiAvailabilityToFile(BigSerial hhId) {

		boost::format fmtr = boost::format(LOG_TAXI_AVAILABILITY) % hhId;
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_TAXI_AVAILABILITY,fmtr.str());

	}

	//taz logsum
	const std::string LOG_TAZ_LOGSUM = "%1%, %2%";

	inline void printTazLevelLogsum(int taz, double logsum)
	{
		boost::format fmtr = boost::format(LOG_TAZ_LOGSUM) 	% taz
															% logsum;

		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_TAZ_LEVEL_LOGSUM, fmtr.str());
	}



	inline void printIndividualHitsLogsum( BigSerial individualId, double logsum )
	{
		boost::format fmtr = boost::format("%1%, %2%") % individualId % logsum;

		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_HITS_LOGSUM, fmtr.str());

	}

	inline void printHouseholdHitsLogsum( std::string title, std::string hitsId, BigSerial householdId, BigSerial individualId, int paxId, vector<double> logsum )
	{
		boost::format fmtr = boost::format( "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25%, %26%, %27%, %28%, %29%, %30%, %31%, %32%, %33%, %34%, %35%, %36%, %37%, %38%, %39%, %40%, %41%, %42%, "
											"%43%, %44%, %45%, %46%, %47%, %48%, %49%, %50%, %51%, %52%, %53%, %54%, %55%, %56%, %57%, %58%, %59%, %60%, %61%, %62%, %63%, %64%, %65%, %66%, %67%, %68%, %69%, %70%, %71%, %72%, %73%, %74%, %75%, %76%, %77%, %78%, %79%, %80%, %81%, %82%, "
											"%83%, %84%, %85%, %86%, %87%, %88%, %89%, %90%, %91%, %92%, %93%, %94%, %95%, %96%, %97%, %98%, %99%, %100%, %101%, %102%, %103%, %104%, %105%, %106%, %107%, %108%, %109%, %110%, %111%, %112%, %113%, %114%, %115%, %116%, %117%, %118%, "
											"%119%, %120%, %121%, %122%, %123%, %124%, %125%, %126%, %127%, %128%, %129%, %130%, %131%, %132%, %133%, %134%, %135%, %136%, %137%, %138%, %139%, %140%, %141%, %142%, %143%, %144%, %145%, %146%, %147%, %148%, %149%, %150%, %151%, "
											"%152%, %153%, %154%, %155%, %156%, %157%, %158%, %159%, %160%, %161%, %162%, %163%, %164%, %165%, %166%, %167%, %168%, %169%, %170%, %171%, %172%, %173%, %174%, %175%, %176%, %177%, %178%, %179%, %180%, %181%, %182%, %183%, %184%, "
											"%185%, %186%, %187%, %188%, %189%, %190%, %191%, %192%, %193%, %194%, %195%, %196%, %197%, %198%, %199%, %200%, %201%, %202%, %203%, %204%, %205%, %206%, %207%, %208%, %209%, %210%, %211%, %212%, %213%, %214%, %215%, %216%, %217%, "
											"%218%, %219%, %220%, %221%, %222%, %223%, %224%, %225%, %226%, %227%, %228%, %229%, %230%, %231%, %232%, %233%, %234%, %235%, %236%, %237%, %238%, %239%, %240%, %241%, %242%, %243%, %244%, %245%, %246%, %247%, %248%, %249%, %250%, "
											"%251%, %252%, %253%, %254%, %255%, %256%, %257%, %258%, %259%, %260%, %261%, %262%, %263%, %264%, %265%, %266%, %267%, %268%, %269%, %270%, %271%, %272%, %273%, %274%, %275%, %276%, %277%, %278%, %279%, %280%, %281%, %282%, %283%, "
											"%284%, %285%, %286%, %287%, %288%, %289%, %290%, %291%, %292%, %293%, %294%, %295%, %296%, %297%, %298%, %299%, %300%, %301%, %302%, %303%, %304%, %305%, %306%, %307%, %308%, %309%, %310%, %311%, %312%, %313%, %314%, %315%, %316%, "
											"%317%, %318%, %319%, %320%, %321%, %322%, %323%, %324%, %325%, %326%, %327%, %328%, %329%, %330%, %331%, %332%, %333%, %334%, %335%, %336%, %337%, %338%, %339%, %340%, %341%, %342%, %343%, %344%, %345%, %346%, %347%, %348%, %349%, "
											"%350%, %351%, %352%, %353%, %354%, %355%, %356%, %357%, %358%, %359%, %360%, %361%, %362%, %363%, %364%, %365%, %366%, %367%, %368%, %369%, %370%, %371%, %372%, %373%, %374%, %375%, %376%, %377%, %378%, %379%, %380%, %381%, %382%, "
											"%383%, %384%, %385%, %386%, %387%, %388%, %389%, %390%, %391%, %392%, %393%, %394%, %395%, %396%, %397%, %398%, %399%, %400%, %401%, %402%, %403%, %404%, %405%, %406%, %407%, %408%, %409%, %410%, %411%, %412%, %413%, %414%, %415%, "
											"%416%, %417%, %418%, %419%, %420%, %421%, %422%, %423%, %424%, %425%, %426%, %427%, %428%, %429%, %430%, %431%, %432%, %433%, %434%, %435%, %436%, %437%, %438%, %439%, %440%, %441%, %442%, %443%, %444%, %445%, %446%, %447%, %448%, "
											"%449%, %450%, %451%, %452%, %453%, %454%, %455%, %456%, %457%, %458%, %459%, %460%, %461%, %462%, %463%, %464%, %465%, %466%, %467%, %468%, %469%, %470%, %471%, %472%, %473%, %474%, %475%, %476%, %477%, %478%, %479%, %480%, %481%, "
											"%482%, %483%, %484%, %485%, %486%, %487%, %488%, %489%, %490%, %491%, %492%, %493%, %494%, %495%, %496%, %497%, %498%, %499%, %500%, %501%, %502%, %503%, %504%, %505%, %506%, %507%, %508%, %509%, %510%, %511%, %512%, %513%, %514%, "
											"%515%, %516%, %517%, %518%, %519%, %520%, %521%, %522%, %523%, %524%, %525%, %526%, %527%, %528%, %529%, %530%, %531%, %532%, %533%, %534%, %535%, %536%, %537%, %538%, %539%, %540%, %541%, %542%, %543%, %544%, %545%, %546%, %547%, "
											"%548%, %549%, %550%, %551%, %552%, %553%, %554%, %555%, %556%, %557%, %558%, %559%, %560%, %561%, %562%, %563%, %564%, %565%, %566%, %567%, %568%, %569%, %570%, %571%, %572%, %573%, %574%, %575%, %576%, %577%, %578%, %579%, %580%, "
											"%581%, %582%, %583%, %584%, %585%, %586%, %587%, %588%, %589%, %590%, %591%, %592%, %593%, %594%, %595%, %596%, %597%, %598%, %599%, %600%, %601%, %602%, %603%, %604%, %605%, %606%, %607%, %608%, %609%, %610%, %611%, %612%, %613%, "
											"%614%, %615%, %616%, %617%, %618%, %619%, %620%, %621%, %622%, %623%, %624%, %625%, %626%, %627%, %628%, %629%, %630%, %631%, %632%, %633%, %634%, %635%, %636%, %637%, %638%, %639%, %640%, %641%, %642%, %643%, %644%, %645%, %646%, "
											"%647%, %648%, %649%, %650%, %651%, %652%, %653%, %654%, %655%, %656%, %657%, %658%, %659%, %660%, %661%, %662%, %663%, %664%, %665%, %666%, %667%, %668%, %669%, %670%, %671%, %672%, %673%, %674%, %675%, %676%, %677%, %678%, %679%, "
											"%680%, %681%, %682%, %683%, %684%, %685%, %686%, %687%, %688%, %689%, %690%, %691%, %692%, %693%, %694%, %695%, %696%, %697%, %698%, %699%, %700%, %701%, %702%, %703%, %704%, %705%, %706%, %707%, %708%, %709%, %710%, %711%, %712%, "
											"%713%, %714%, %715%, %716%, %717%, %718%, %719%, %720%, %721%, %722%, %723%, %724%, %725%, %726%, %727%, %728%, %729%, %730%, %731%, %732%, %733%, %734%, %735%, %736%, %737%, %738%, %739%, %740%, %741%, %742%, %743%, %744%, %745%, "
											"%746%, %747%, %748%, %749%, %750%, %751%, %752%, %753%, %754%, %755%, %756%, %757%, %758%, %759%, %760%, %761%, %762%, %763%, %764%, %765%, %766%, %767%, %768%, %769%, %770%, %771%, %772%, %773%, %774%, %775%, %776%, %777%, %778%, "
											"%779%, %780%, %781%, %782%, %783%, %784%, %785%, %786%, %787%, %788%, %789%, %790%, %791%, %792%, %793%, %794%, %795%, %796%, %797%, %798%, %799%, %800%, %801%, %802%, %803%, %804%, %805%, %806%, %807%, %808%, %809%, %810%, %811%, "
											"%812%, %813%, %814%, %815%, %816%, %817%, %818%, %819%, %820%, %821%, %822%, %823%, %824%, %825%, %826%, %827%, %828%, %829%, %830%, %831%, %832%, %833%, %834%, %835%, %836%, %837%, %838%, %839%, %840%, %841%, %842%, %843%, %844%, "
											"%845%, %846%, %847%, %848%, %849%, %850%, %851%, %852%, %853%, %854%, %855%, %856%, %857%, %858%, %859%, %860%, %861%, %862%, %863%, %864%, %865%, %866%, %867%, %868%, %869%, %870%, %871%, %872%, %873%, %874%, %875%, %876%, %877%, "
											"%878%, %879%, %880%, %881%, %882%, %883%, %884%, %885%, %886%, %887%, %888%, %889%, %890%, %891%, %892%, %893%, %894%, %895%, %896%, %897%, %898%, %899%, %900%, %901%, %902%, %903%, %904%, %905%, %906%, %907%, %908%, %909%, %910%, "
											"%911%, %912%, %913%, %914%, %915%, %916%, %917%, %918%, %919%, %920%, %921%, %922%, %923%, %924%, %925%, %926%, %927%, %928%, %929%, %930%, %931%, %932%, %933%, %934%, %935%, %936%, %937%, %938%, %939%, %940%, %941%, %942%, %943%, "
											"%944%, %945%, %946%, %947%, %948%, %949%, %950%, %951%, %952%, %953%, %954%, %955%, %956%, %957%, %958%, %959%, %960%, %961%, %962%, %963%, %964%, %965%, %966%, %967%, %968%, %969%, %970%, %971%, %972%, %973%, %974%, %975%, %976%, "
											"%977%, %978%, %979%, %980%, %981%, %982%, %983%, %984%, %985%, %986%, %987%, %988%, %989%, %990%, %991%, %992%, %993%, %994%, %995%, %996%, %997%, %998%, %999%, %1000%, %1001%, %1002%, %1003%, %1004%, %1005%, %1006%, %1007%, "
											"%1008%, %1009%, %1010%, %1011%, %1012%, %1013%, %1014%, %1015%, %1016%, %1017%, %1018%, %1019%, %1020%, %1021%, %1022%, %1023%, %1024%, %1025%, %1026%, %1027%, %1028%, %1029%, %1030%, %1031%, %1032%, %1033%, %1034%, %1035%, "
											"%1036%, %1037%, %1038%, %1039%, %1040%, %1041%, %1042%, %1043%, %1044%, %1045%, %1046%, %1047%, %1048%, %1049%, %1050%, %1051%, %1052%, %1053%, %1054%, %1055%, %1056%, %1057%, %1058%, %1059%, %1060%, %1061%, %1062%, %1063%, "
											"%1064%, %1065%, %1066%, %1067%, %1068%, %1069%, %1070%, %1071%, %1072%, %1073%, %1074%, %1075%, %1076%, %1077%, %1078%, %1079%, %1080%, %1081%, %1082%, %1083%, %1084%, %1085%, %1086%, %1087%, %1088%, %1089%, %1090%, %1091%, "
											"%1092%, %1093%, %1094%, %1095%, %1096%, %1097%, %1098%, %1099%, %1100%, %1101%, %1102%, %1103%, %1104%, %1105%, %1106%, %1107%, %1108%, %1109%, %1110%, %1111%, %1112%, %1113%, %1114%, %1115%, %1116%, %1117%, %1118%, %1119%, "
											"%1120%, %1121%, %1122%, %1123%, %1124%, %1125%, %1126%, %1127%, %1128%, %1129%, %1130%, %1131%, %1132%, %1133%, %1134%, %1135%, %1136%, %1137%, %1138%, %1139%, %1140%, %1141%, %1142%, %1143%, %1144%, %1145%, %1146%, %1147%, "
											"%1148%, %1149%, %1150%, %1151%, %1152%, %1153%, %1154%, %1155%, %1156%, %1157%, %1158%, %1159%, %1160%, %1161%, %1162%, %1163%, %1164%, %1165%, %1166%, %1167%, %1168%, %1169%, %1170%, %1171%, %1172%, %1173%, %1174%, %1175%, "
											"%1176%, %1177%, %1178%, %1179%, %1180%, %1181%, %1182%, %1183%, %1184%, %1185%, %1186%, %1187%, %1188%, %1189%, %1190%, %1191%, %1192%, %1193%, %1194%, %1195%, %1196%, %1197%, %1198%, %1199%, %1200%, %1201%, %1202%, %1203%, "
											"%1204%, %1205%, %1206%, %1207%, %1208%, %1209%, %1210%, %1211%, %1212%, %1213%, %1214%, %1215%, %1216%, %1217%, %1218%, %1219%, %1220%, %1221%, %1222%, %1223%, %1224%, %1225%, %1226%, %1227%, %1228%, %1229%, %1230%, %1231%, "
											"%1232%, %1233%, %1234%, %1235%, %1236%, %1237%, %1238%, %1239%, %1240%, %1241%, %1242%, %1243%, %1244%, %1245%, %1246%, %1247%, %1248%, %1249%, %1250%, %1251%, %1252%, %1253%, %1254%, %1255%, %1256%, %1257%, %1258%, %1259%, "
											"%1260%, %1261%, %1262%, %1263%, %1264%, %1265%, %1266%, %1267%, %1268%, %1269%, %1270%, %1271%, %1272%") % title % hitsId % householdId % individualId % paxId
											% logsum[0]  % logsum[1]  % logsum[2]  % logsum[3]  % logsum[4]  % logsum[5]  % logsum[6]  % logsum[7]  % logsum[8]  % logsum[9]  % logsum[10]  % logsum[11]  % logsum[12]  % logsum[13]
											% logsum[14]  % logsum[15]  % logsum[16]  % logsum[17]  % logsum[18]  % logsum[19]  % logsum[20]  % logsum[21]  % logsum[22]  % logsum[23]  % logsum[24]  % logsum[25]  % logsum[26]  % logsum[27]
											% logsum[28]  % logsum[29]  % logsum[30]  % logsum[31]  % logsum[32]  % logsum[33]  % logsum[34]  % logsum[35]  % logsum[36]  % logsum[37]  % logsum[38]  % logsum[39]  % logsum[40]  % logsum[41]
											% logsum[42]  % logsum[43]  % logsum[44]  % logsum[45]  % logsum[46]  % logsum[47]  % logsum[48]  % logsum[49]  % logsum[50]  % logsum[51]  % logsum[52]  % logsum[53]  % logsum[54]  % logsum[55]
											% logsum[56]  % logsum[57]  % logsum[58]  % logsum[59]  % logsum[60]  % logsum[61]  % logsum[62]  % logsum[63]  % logsum[64]  % logsum[65]  % logsum[66]  % logsum[67]  % logsum[68]  % logsum[69]
											% logsum[70]  % logsum[71]  % logsum[72]  % logsum[73]  % logsum[74]  % logsum[75]  % logsum[76]  % logsum[77]  % logsum[78]  % logsum[79]  % logsum[80]  % logsum[81]  % logsum[82]  % logsum[83]
											% logsum[84]  % logsum[85]  % logsum[86]  % logsum[87]  % logsum[88]  % logsum[89]  % logsum[90]  % logsum[91]  % logsum[92]  % logsum[93]  % logsum[94]  % logsum[95]  % logsum[96]  % logsum[97]
											% logsum[98]  % logsum[99]  % logsum[100]  % logsum[101]  % logsum[102]  % logsum[103]  % logsum[104]  % logsum[105]  % logsum[106]  % logsum[107]  % logsum[108]  % logsum[109]  % logsum[110]
											% logsum[111]  % logsum[112]  % logsum[113]  % logsum[114]  % logsum[115]  % logsum[116]  % logsum[117]  % logsum[118]  % logsum[119]  % logsum[120]  % logsum[121]  % logsum[122]  % logsum[123]
											% logsum[124]  % logsum[125]  % logsum[126]  % logsum[127]  % logsum[128]  % logsum[129]  % logsum[130]  % logsum[131]  % logsum[132]  % logsum[133]  % logsum[134]  % logsum[135]  % logsum[136]
											% logsum[137]  % logsum[138]  % logsum[139]  % logsum[140]  % logsum[141]  % logsum[142]  % logsum[143]  % logsum[144]  % logsum[145]  % logsum[146]  % logsum[147]  % logsum[148]  % logsum[149]
											% logsum[150]  % logsum[151]  % logsum[152]  % logsum[153]  % logsum[154]  % logsum[155]  % logsum[156]  % logsum[157]  % logsum[158]  % logsum[159]  % logsum[160]  % logsum[161]  % logsum[162]
											% logsum[163]  % logsum[164]  % logsum[165]  % logsum[166]  % logsum[167]  % logsum[168]  % logsum[169]  % logsum[170]  % logsum[171]  % logsum[172]  % logsum[173]  % logsum[174]  % logsum[175]
											% logsum[176]  % logsum[177]  % logsum[178]  % logsum[179]  % logsum[180]  % logsum[181]  % logsum[182]  % logsum[183]  % logsum[184]  % logsum[185]  % logsum[186]  % logsum[187]  % logsum[188]
											% logsum[189]  % logsum[190]  % logsum[191]  % logsum[192]  % logsum[193]  % logsum[194]  % logsum[195]  % logsum[196]  % logsum[197]  % logsum[198]  % logsum[199]  % logsum[200]  % logsum[201]
											% logsum[202]  % logsum[203]  % logsum[204]  % logsum[205]  % logsum[206]  % logsum[207]  % logsum[208]  % logsum[209]  % logsum[210]  % logsum[211]  % logsum[212]  % logsum[213]  % logsum[214]
											% logsum[215]  % logsum[216]  % logsum[217]  % logsum[218]  % logsum[219]  % logsum[220]  % logsum[221]  % logsum[222]  % logsum[223]  % logsum[224]  % logsum[225]  % logsum[226]  % logsum[227]
											% logsum[228]  % logsum[229]  % logsum[230]  % logsum[231]  % logsum[232]  % logsum[233]  % logsum[234]  % logsum[235]  % logsum[236]  % logsum[237]  % logsum[238]  % logsum[239]  % logsum[240]
											% logsum[241]  % logsum[242]  % logsum[243]  % logsum[244]  % logsum[245]  % logsum[246]  % logsum[247]  % logsum[248]  % logsum[249]  % logsum[250]  % logsum[251]  % logsum[252]  % logsum[253]
											% logsum[254]  % logsum[255]  % logsum[256]  % logsum[257]  % logsum[258]  % logsum[259]  % logsum[260]  % logsum[261]  % logsum[262]  % logsum[263]  % logsum[264]  % logsum[265]  % logsum[266]
											% logsum[267]  % logsum[268]  % logsum[269]  % logsum[270]  % logsum[271]  % logsum[272]  % logsum[273]  % logsum[274]  % logsum[275]  % logsum[276]  % logsum[277]  % logsum[278]  % logsum[279]
											% logsum[280]  % logsum[281]  % logsum[282]  % logsum[283]  % logsum[284]  % logsum[285]  % logsum[286]  % logsum[287]  % logsum[288]  % logsum[289]  % logsum[290]  % logsum[291]  % logsum[292]
											% logsum[293]  % logsum[294]  % logsum[295]  % logsum[296]  % logsum[297]  % logsum[298]  % logsum[299]  % logsum[300]  % logsum[301]  % logsum[302]  % logsum[303]  % logsum[304]  % logsum[305]
											% logsum[306]  % logsum[307]  % logsum[308]  % logsum[309]  % logsum[310]  % logsum[311]  % logsum[312]  % logsum[313]  % logsum[314]  % logsum[315]  % logsum[316]  % logsum[317]  % logsum[318]
											% logsum[319]  % logsum[320]  % logsum[321]  % logsum[322]  % logsum[323]  % logsum[324]  % logsum[325]  % logsum[326]  % logsum[327]  % logsum[328]  % logsum[329]  % logsum[330]  % logsum[331]
											% logsum[332]  % logsum[333]  % logsum[334]  % logsum[335]  % logsum[336]  % logsum[337]  % logsum[338]  % logsum[339]  % logsum[340]  % logsum[341]  % logsum[342]  % logsum[343]  % logsum[344]
											% logsum[345]  % logsum[346]  % logsum[347]  % logsum[348]  % logsum[349]  % logsum[350]  % logsum[351]  % logsum[352]  % logsum[353]  % logsum[354]  % logsum[355]  % logsum[356]  % logsum[357]
											% logsum[358]  % logsum[359]  % logsum[360]  % logsum[361]  % logsum[362]  % logsum[363]  % logsum[364]  % logsum[365]  % logsum[366]  % logsum[367]  % logsum[368]  % logsum[369]  % logsum[370]
											% logsum[371]  % logsum[372]  % logsum[373]  % logsum[374]  % logsum[375]  % logsum[376]  % logsum[377]  % logsum[378]  % logsum[379]  % logsum[380]  % logsum[381]  % logsum[382]  % logsum[383]
											% logsum[384]  % logsum[385]  % logsum[386]  % logsum[387]  % logsum[388]  % logsum[389]  % logsum[390]  % logsum[391]  % logsum[392]  % logsum[393]  % logsum[394]  % logsum[395]  % logsum[396]
											% logsum[397]  % logsum[398]  % logsum[399]  % logsum[400]  % logsum[401]  % logsum[402]  % logsum[403]  % logsum[404]  % logsum[405]  % logsum[406]  % logsum[407]  % logsum[408]  % logsum[409]
											% logsum[410]  % logsum[411]  % logsum[412]  % logsum[413]  % logsum[414]  % logsum[415]  % logsum[416]  % logsum[417]  % logsum[418]  % logsum[419]  % logsum[420]  % logsum[421]  % logsum[422]
											% logsum[423]  % logsum[424]  % logsum[425]  % logsum[426]  % logsum[427]  % logsum[428]  % logsum[429]  % logsum[430]  % logsum[431]  % logsum[432]  % logsum[433]  % logsum[434]  % logsum[435]
											% logsum[436]  % logsum[437]  % logsum[438]  % logsum[439]  % logsum[440]  % logsum[441]  % logsum[442]  % logsum[443]  % logsum[444]  % logsum[445]  % logsum[446]  % logsum[447]  % logsum[448]
											% logsum[449]  % logsum[450]  % logsum[451]  % logsum[452]  % logsum[453]  % logsum[454]  % logsum[455]  % logsum[456]  % logsum[457]  % logsum[458]  % logsum[459]  % logsum[460]  % logsum[461]
											% logsum[462]  % logsum[463]  % logsum[464]  % logsum[465]  % logsum[466]  % logsum[467]  % logsum[468]  % logsum[469]  % logsum[470]  % logsum[471]  % logsum[472]  % logsum[473]  % logsum[474]
											% logsum[475]  % logsum[476]  % logsum[477]  % logsum[478]  % logsum[479]  % logsum[480]  % logsum[481]  % logsum[482]  % logsum[483]  % logsum[484]  % logsum[485]  % logsum[486]  % logsum[487]
											% logsum[488]  % logsum[489]  % logsum[490]  % logsum[491]  % logsum[492]  % logsum[493]  % logsum[494]  % logsum[495]  % logsum[496]  % logsum[497]  % logsum[498]  % logsum[499]  % logsum[500]
											% logsum[501]  % logsum[502]  % logsum[503]  % logsum[504]  % logsum[505]  % logsum[506]  % logsum[507]  % logsum[508]  % logsum[509]  % logsum[510]  % logsum[511]  % logsum[512]  % logsum[513]
											% logsum[514]  % logsum[515]  % logsum[516]  % logsum[517]  % logsum[518]  % logsum[519]  % logsum[520]  % logsum[521]  % logsum[522]  % logsum[523]  % logsum[524]  % logsum[525]  % logsum[526]
											% logsum[527]  % logsum[528]  % logsum[529]  % logsum[530]  % logsum[531]  % logsum[532]  % logsum[533]  % logsum[534]  % logsum[535]  % logsum[536]  % logsum[537]  % logsum[538]  % logsum[539]
											% logsum[540]  % logsum[541]  % logsum[542]  % logsum[543]  % logsum[544]  % logsum[545]  % logsum[546]  % logsum[547]  % logsum[548]  % logsum[549]  % logsum[550]  % logsum[551]  % logsum[552]
											% logsum[553]  % logsum[554]  % logsum[555]  % logsum[556]  % logsum[557]  % logsum[558]  % logsum[559]  % logsum[560]  % logsum[561]  % logsum[562]  % logsum[563]  % logsum[564]  % logsum[565]
											% logsum[566]  % logsum[567]  % logsum[568]  % logsum[569]  % logsum[570]  % logsum[571]  % logsum[572]  % logsum[573]  % logsum[574]  % logsum[575]  % logsum[576]  % logsum[577]  % logsum[578]
											% logsum[579]  % logsum[580]  % logsum[581]  % logsum[582]  % logsum[583]  % logsum[584]  % logsum[585]  % logsum[586]  % logsum[587]  % logsum[588]  % logsum[589]  % logsum[590]  % logsum[591]
											% logsum[592]  % logsum[593]  % logsum[594]  % logsum[595]  % logsum[596]  % logsum[597]  % logsum[598]  % logsum[599]  % logsum[600]  % logsum[601]  % logsum[602]  % logsum[603]  % logsum[604]
											% logsum[605]  % logsum[606]  % logsum[607]  % logsum[608]  % logsum[609]  % logsum[610]  % logsum[611]  % logsum[612]  % logsum[613]  % logsum[614]  % logsum[615]  % logsum[616]  % logsum[617]
											% logsum[618]  % logsum[619]  % logsum[620]  % logsum[621]  % logsum[622]  % logsum[623]  % logsum[624]  % logsum[625]  % logsum[626]  % logsum[627]  % logsum[628]  % logsum[629]  % logsum[630]
											% logsum[631]  % logsum[632]  % logsum[633]  % logsum[634]  % logsum[635]  % logsum[636]  % logsum[637]  % logsum[638]  % logsum[639]  % logsum[640]  % logsum[641]  % logsum[642]  % logsum[643]
											% logsum[644]  % logsum[645]  % logsum[646]  % logsum[647]  % logsum[648]  % logsum[649]  % logsum[650]  % logsum[651]  % logsum[652]  % logsum[653]  % logsum[654]  % logsum[655]  % logsum[656]
											% logsum[657]  % logsum[658]  % logsum[659]  % logsum[660]  % logsum[661]  % logsum[662]  % logsum[663]  % logsum[664]  % logsum[665]  % logsum[666]  % logsum[667]  % logsum[668]  % logsum[669]
											% logsum[670]  % logsum[671]  % logsum[672]  % logsum[673]  % logsum[674]  % logsum[675]  % logsum[676]  % logsum[677]  % logsum[678]  % logsum[679]  % logsum[680]  % logsum[681]  % logsum[682]
											% logsum[683]  % logsum[684]  % logsum[685]  % logsum[686]  % logsum[687]  % logsum[688]  % logsum[689]  % logsum[690]  % logsum[691]  % logsum[692]  % logsum[693]  % logsum[694]  % logsum[695]
											% logsum[696]  % logsum[697]  % logsum[698]  % logsum[699]  % logsum[700]  % logsum[701]  % logsum[702]  % logsum[703]  % logsum[704]  % logsum[705]  % logsum[706]  % logsum[707]  % logsum[708]
											% logsum[709]  % logsum[710]  % logsum[711]  % logsum[712]  % logsum[713]  % logsum[714]  % logsum[715]  % logsum[716]  % logsum[717]  % logsum[718]  % logsum[719]  % logsum[720]  % logsum[721]
											% logsum[722]  % logsum[723]  % logsum[724]  % logsum[725]  % logsum[726]  % logsum[727]  % logsum[728]  % logsum[729]  % logsum[730]  % logsum[731]  % logsum[732]  % logsum[733]  % logsum[734]
											% logsum[735]  % logsum[736]  % logsum[737]  % logsum[738]  % logsum[739]  % logsum[740]  % logsum[741]  % logsum[742]  % logsum[743]  % logsum[744]  % logsum[745]  % logsum[746]  % logsum[747]
											% logsum[748]  % logsum[749]  % logsum[750]  % logsum[751]  % logsum[752]  % logsum[753]  % logsum[754]  % logsum[755]  % logsum[756]  % logsum[757]  % logsum[758]  % logsum[759]  % logsum[760]
											% logsum[761]  % logsum[762]  % logsum[763]  % logsum[764]  % logsum[765]  % logsum[766]  % logsum[767]  % logsum[768]  % logsum[769]  % logsum[770]  % logsum[771]  % logsum[772]  % logsum[773]
											% logsum[774]  % logsum[775]  % logsum[776]  % logsum[777]  % logsum[778]  % logsum[779]  % logsum[780]  % logsum[781]  % logsum[782]  % logsum[783]  % logsum[784]  % logsum[785]  % logsum[786]
											% logsum[787]  % logsum[788]  % logsum[789]  % logsum[790]  % logsum[791]  % logsum[792]  % logsum[793]  % logsum[794]  % logsum[795]  % logsum[796]  % logsum[797]  % logsum[798]  % logsum[799]
											% logsum[800]  % logsum[801]  % logsum[802]  % logsum[803]  % logsum[804]  % logsum[805]  % logsum[806]  % logsum[807]  % logsum[808]  % logsum[809]  % logsum[810]  % logsum[811]  % logsum[812]
											% logsum[813]  % logsum[814]  % logsum[815]  % logsum[816]  % logsum[817]  % logsum[818]  % logsum[819]  % logsum[820]  % logsum[821]  % logsum[822]  % logsum[823]  % logsum[824]  % logsum[825]
											% logsum[826]  % logsum[827]  % logsum[828]  % logsum[829]  % logsum[830]  % logsum[831]  % logsum[832]  % logsum[833]  % logsum[834]  % logsum[835]  % logsum[836]  % logsum[837]  % logsum[838]
											% logsum[839]  % logsum[840]  % logsum[841]  % logsum[842]  % logsum[843]  % logsum[844]  % logsum[845]  % logsum[846]  % logsum[847]  % logsum[848]  % logsum[849]  % logsum[850]  % logsum[851]
											% logsum[852]  % logsum[853]  % logsum[854]  % logsum[855]  % logsum[856]  % logsum[857]  % logsum[858]  % logsum[859]  % logsum[860]  % logsum[861]  % logsum[862]  % logsum[863]  % logsum[864]
											% logsum[865]  % logsum[866]  % logsum[867]  % logsum[868]  % logsum[869]  % logsum[870]  % logsum[871]  % logsum[872]  % logsum[873]  % logsum[874]  % logsum[875]  % logsum[876]  % logsum[877]
											% logsum[878]  % logsum[879]  % logsum[880]  % logsum[881]  % logsum[882]  % logsum[883]  % logsum[884]  % logsum[885]  % logsum[886]  % logsum[887]  % logsum[888]  % logsum[889]  % logsum[890]
											% logsum[891]  % logsum[892]  % logsum[893]  % logsum[894]  % logsum[895]  % logsum[896]  % logsum[897]  % logsum[898]  % logsum[899]  % logsum[900]  % logsum[901]  % logsum[902]  % logsum[903]
											% logsum[904]  % logsum[905]  % logsum[906]  % logsum[907]  % logsum[908]  % logsum[909]  % logsum[910]  % logsum[911]  % logsum[912]  % logsum[913]  % logsum[914]  % logsum[915]  % logsum[916]
											% logsum[917]  % logsum[918]  % logsum[919]  % logsum[920]  % logsum[921]  % logsum[922]  % logsum[923]  % logsum[924]  % logsum[925]  % logsum[926]  % logsum[927]  % logsum[928]  % logsum[929]
											% logsum[930]  % logsum[931]  % logsum[932]  % logsum[933]  % logsum[934]  % logsum[935]  % logsum[936]  % logsum[937]  % logsum[938]  % logsum[939]  % logsum[940]  % logsum[941]  % logsum[942]
											% logsum[943]  % logsum[944]  % logsum[945]  % logsum[946]  % logsum[947]  % logsum[948]  % logsum[949]  % logsum[950]  % logsum[951]  % logsum[952]  % logsum[953]  % logsum[954]  % logsum[955]
											% logsum[956]  % logsum[957]  % logsum[958]  % logsum[959]  % logsum[960]  % logsum[961]  % logsum[962]  % logsum[963]  % logsum[964]  % logsum[965]  % logsum[966]  % logsum[967]  % logsum[968]
											% logsum[969]  % logsum[970]  % logsum[971]  % logsum[972]  % logsum[973]  % logsum[974]  % logsum[975]  % logsum[976]  % logsum[977]  % logsum[978]  % logsum[979]  % logsum[980]  % logsum[981]
											% logsum[982]  % logsum[983]  % logsum[984]  % logsum[985]  % logsum[986]  % logsum[987]  % logsum[988]  % logsum[989]  % logsum[990]  % logsum[991]  % logsum[992]  % logsum[993]  % logsum[994]
											% logsum[995]  % logsum[996]  % logsum[997]  % logsum[998]  % logsum[999]  % logsum[1000]  % logsum[1001]  % logsum[1002]  % logsum[1003]  % logsum[1004]  % logsum[1005]  % logsum[1006]
											% logsum[1007]  % logsum[1008]  % logsum[1009]  % logsum[1010]  % logsum[1011]  % logsum[1012]  % logsum[1013]  % logsum[1014]  % logsum[1015]  % logsum[1016]  % logsum[1017]  % logsum[1018]
											% logsum[1019]  % logsum[1020]  % logsum[1021]  % logsum[1022]  % logsum[1023]  % logsum[1024]  % logsum[1025]  % logsum[1026]  % logsum[1027]  % logsum[1028]  % logsum[1029]  % logsum[1030]
											% logsum[1031]  % logsum[1032]  % logsum[1033]  % logsum[1034]  % logsum[1035]  % logsum[1036]  % logsum[1037]  % logsum[1038]  % logsum[1039]  % logsum[1040]  % logsum[1041]  % logsum[1042]
											% logsum[1043]  % logsum[1044]  % logsum[1045]  % logsum[1046]  % logsum[1047]  % logsum[1048]  % logsum[1049]  % logsum[1050]  % logsum[1051]  % logsum[1052]  % logsum[1053]  % logsum[1054]
											% logsum[1055]  % logsum[1056]  % logsum[1057]  % logsum[1058]  % logsum[1059]  % logsum[1060]  % logsum[1061]  % logsum[1062]  % logsum[1063]  % logsum[1064]  % logsum[1065]  % logsum[1066]
											% logsum[1067]  % logsum[1068]  % logsum[1069]  % logsum[1070]  % logsum[1071]  % logsum[1072]  % logsum[1073]  % logsum[1074]  % logsum[1075]  % logsum[1076]  % logsum[1077]  % logsum[1078]
											% logsum[1079]  % logsum[1080]  % logsum[1081]  % logsum[1082]  % logsum[1083]  % logsum[1084]  % logsum[1085]  % logsum[1086]  % logsum[1087]  % logsum[1088]  % logsum[1089]  % logsum[1090]
											% logsum[1091]  % logsum[1092]  % logsum[1093]  % logsum[1094]  % logsum[1095]  % logsum[1096]  % logsum[1097]  % logsum[1098]  % logsum[1099]  % logsum[1100]  % logsum[1101]  % logsum[1102]
											% logsum[1103]  % logsum[1104]  % logsum[1105]  % logsum[1106]  % logsum[1107]  % logsum[1108]  % logsum[1109]  % logsum[1110]  % logsum[1111]  % logsum[1112]  % logsum[1113]  % logsum[1114]
											% logsum[1115]  % logsum[1116]  % logsum[1117]  % logsum[1118]  % logsum[1119]  % logsum[1120]  % logsum[1121]  % logsum[1122]  % logsum[1123]  % logsum[1124]  % logsum[1125]  % logsum[1126]
											% logsum[1127]  % logsum[1128]  % logsum[1129]  % logsum[1130]  % logsum[1131]  % logsum[1132]  % logsum[1133]  % logsum[1134]  % logsum[1135]  % logsum[1136]  % logsum[1137]  % logsum[1138]
											% logsum[1139]  % logsum[1140]  % logsum[1141]  % logsum[1142]  % logsum[1143]  % logsum[1144]  % logsum[1145]  % logsum[1146]  % logsum[1147]  % logsum[1148]  % logsum[1149]  % logsum[1150]
											% logsum[1151]  % logsum[1152]  % logsum[1153]  % logsum[1154]  % logsum[1155]  % logsum[1156]  % logsum[1157]  % logsum[1158]  % logsum[1159]  % logsum[1160]  % logsum[1161]  % logsum[1162]
											% logsum[1163]  % logsum[1164]  % logsum[1165]  % logsum[1166]  % logsum[1167]  % logsum[1168]  % logsum[1169]  % logsum[1170]  % logsum[1171]  % logsum[1172]  % logsum[1173]  % logsum[1174]
											% logsum[1175]  % logsum[1176]  % logsum[1177]  % logsum[1178]  % logsum[1179]  % logsum[1180]  % logsum[1181]  % logsum[1182]  % logsum[1183]  % logsum[1184]  % logsum[1185]  % logsum[1186]
											% logsum[1187]  % logsum[1188]  % logsum[1189]  % logsum[1190]  % logsum[1191]  % logsum[1192]  % logsum[1193]  % logsum[1194]  % logsum[1195]  % logsum[1196]  % logsum[1197]  % logsum[1198]
											% logsum[1199]  % logsum[1200]  % logsum[1201]  % logsum[1202]  % logsum[1203]  % logsum[1204]  % logsum[1205]  % logsum[1206]  % logsum[1207]  % logsum[1208]  % logsum[1209]  % logsum[1210]
											% logsum[1211]  % logsum[1212]  % logsum[1213]  % logsum[1214]  % logsum[1215]  % logsum[1216]  % logsum[1217]  % logsum[1218]  % logsum[1219]  % logsum[1220]  % logsum[1221]  % logsum[1222]
											% logsum[1223]  % logsum[1224]  % logsum[1225]  % logsum[1226]  % logsum[1227]  % logsum[1228]  % logsum[1229]  % logsum[1230]  % logsum[1231]  % logsum[1232]  % logsum[1233]  % logsum[1234]
											% logsum[1235]  % logsum[1236]  % logsum[1237]  % logsum[1238]  % logsum[1239]  % logsum[1240]  % logsum[1241]  % logsum[1242]  % logsum[1243]  % logsum[1244]  % logsum[1245]  % logsum[1246]
											% logsum[1247]  % logsum[1248]  % logsum[1249]  % logsum[1250]  % logsum[1251]  % logsum[1252]  % logsum[1253]  % logsum[1254]  % logsum[1255]  % logsum[1256]  % logsum[1257]  % logsum[1258]
											% logsum[1259]  % logsum[1260]  % logsum[1261]  % logsum[1262]  % logsum[1263]  % logsum[1264]  % logsum[1265]  % logsum[1266];


		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_HITS_LOGSUM, fmtr.str());
	}

	inline void printHouseholdHitsLogsumFVO( std::string hitsId, int paxId, BigSerial householdId, BigSerial individualId, int memberId, int tazH, int tazW, vector<double> logsum, vector<double> travelProbability, vector<double> tripsExpected )
	{

		boost::format fmtr = boost::format( "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%") % hitsId % paxId % householdId % individualId % memberId % tazH % tazW % logsum[0] % logsum[1] % travelProbability[0] % travelProbability[1] % tripsExpected[0] % tripsExpected[1];
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_LOGSUM_VO, fmtr.str());
	}
}

HM_Model::TazStats::TazStats(BigSerial tazId) :	tazId(tazId), hhNum(0), hhTotalIncome(0), numChinese(0), numIndian(0), numMalay(0), householdSize(0),individuals(0) {}

HM_Model::TazStats::~TazStats() {}

void HM_Model::TazStats::updateStats(const Household& household)
{
	hhNum++;
	hhTotalIncome += household.getIncome();
	individuals +=  household.getSize();

	if( household.getEthnicityId() == 1 ) //chinese
		numChinese++;

	if( household.getEthnicityId() == 2 )  //malay
		numMalay++;

	if( household.getEthnicityId() == 3 ) // indian
		numIndian++;

	householdSize += household.getSize();

}

BigSerial HM_Model::TazStats::getTazId() const
{
	return tazId;
}

long int HM_Model::TazStats::getHH_Num() const
{
	return hhNum;
}

int HM_Model::TazStats::getIndividuals() const
{
	return individuals;
}

double HM_Model::TazStats::getHH_TotalIncome() const
{
	return hhTotalIncome;
}

double HM_Model::TazStats::getHH_AvgIncome() const
{
	return hhTotalIncome / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getChinesePercentage() const
{
	return numChinese / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getMalayPercentage() const
{
	return numMalay / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getIndianPercentage() const
{
	return numIndian / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getAvgHHSize() const
{
	return householdSize / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}


HM_Model::HM_Model(WorkGroup& workGroup) :	Model(MODEL_NAME, workGroup),numberOfBidders(0), initialHHAwakeningCounter(0), numLifestyle1HHs(0), numLifestyle2HHs(0), numLifestyle3HHs(0), hasTaxiAccess(false),
											householdLogsumCounter(0), simulationStopCounter(0), developerModel(nullptr), startDay(0), bidId(0), numberOfBids(0), numberOfExits(0),	numberOfSuccessfulBids(0), unitSaleId(0){}

HM_Model::~HM_Model()
{
	stopImpl(); //for now
}

void HM_Model::incrementBidders()
{
	numberOfBidders++;
}

void HM_Model::decrementBidders()
{
	numberOfBidders--;
}

int HM_Model::getNumberOfBidders()
{
	return numberOfBidders;
}

void HM_Model::incrementBids()
{
	numberOfBids++;
}

void HM_Model::incrementExits()
{
	numberOfExits++;
}

void HM_Model::incrementSuccessfulBids()
{
	numberOfSuccessfulBids++;
}

void HM_Model::resetBAEStatistics() //BAE is Bids, Awakenings and Exits
{
	initialHHAwakeningCounter = 0;
	numberOfBids = 0;
	numberOfExits = 0;
	numberOfSuccessfulBids = 0;
}

int HM_Model::getBids()
{
	return numberOfBids;
}

int HM_Model::getExits()
{
	return numberOfExits;
}

int HM_Model::getSuccessfulBids()
{
	return numberOfSuccessfulBids;
}

void HM_Model::setDeveloperModel(DeveloperModel *developerModelPointer)
{
	developerModel = developerModelPointer;
}

DeveloperModel* HM_Model::getDeveloperModel() const
{
	return developerModel;
}

Job* HM_Model::getJobById(BigSerial id) const
{
	JobMap::const_iterator itr = jobsById.find(id);

	if (itr != jobsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}



Establishment* HM_Model::getEstablishmentById(BigSerial id) const
{
	EstablishmentMap::const_iterator itr = establishmentsById.find(id);

	if (itr != establishmentsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}


Individual* HM_Model::getIndividualById(BigSerial id) const
{
	IndividualMap::const_iterator itr = individualsById.find(id);

	if (itr != individualsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

HM_Model::HouseholdList* HM_Model::getHouseholdList()
{
	return &households;
}

HM_Model::HousingInterestRateList* HM_Model::getHousingInterestRateList()
{
	return &housingInterestRates;
}


Postcode* HM_Model::getPostcodeById(BigSerial id) const
{
	PostcodeMap::const_iterator itr = postcodesById.find(id);

	if (itr != postcodesById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}


Household* HM_Model::getHouseholdById(BigSerial id) const
{
	HouseholdMap::const_iterator itr = householdsById.find(id);

	if (itr != householdsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

Awakening* HM_Model::getAwakeningById( BigSerial id) const
{
	AwakeningMap::const_iterator itr = awakeningById.find(id);

	if( itr != awakeningById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

const Unit* HM_Model::getUnitById(BigSerial id) const
{
	UnitMap::const_iterator itr = unitsById.find(id);
	if (itr != unitsById.end())
	{
		return (*itr).second;
	}
	return nullptr;
}

BigSerial HM_Model::getEstablishmentTazId(BigSerial establishmentId) const
{
	const Establishment* establishment = getEstablishmentById(establishmentId);
	BigSerial tazId = INVALID_ID;

	if (establishment)
	{
		tazId = DataManagerSingleton::getInstance().getPostcodeTazId(establishment->getSlaAddressId());
	}

	return tazId;
}


BigSerial HM_Model::getUnitTazId(BigSerial unitId) const
{
	const Unit* unit = getUnitById(unitId);
	BigSerial tazId = INVALID_ID;

	if (unit)
	{
		tazId = DataManagerSingleton::getInstance().getPostcodeTazId(unit->getSlaAddressId());
	}

	return tazId;
}

const HM_Model::TazStats* HM_Model::getTazStats(BigSerial tazId) const
{
	StatsMap::const_iterator itr = stats.find(tazId);
	if (itr != stats.end())
	{
		return (*itr).second;
	}
	return nullptr;
}


double HM_Model::ComputeHedonicPriceLogsumFromDatabase( BigSerial taz) const
{
	LogsumMtzV2Map::const_iterator itr = logsumMtzV2ById.find(taz);

	if (itr != logsumMtzV2ById.end())
	{
		LogsumMtzV2 *tazLogsum = itr->second;

		return tazLogsum->getLogsumWeighted();
	}

	return 0;
}

double HM_Model::ComputeHedonicPriceLogsumFromMidterm(BigSerial taz)
{

    BigSerial workTaz = -1;
    int vehicleOwnership = -1;
    double logsum = 0;

    boost::unordered_map<BigSerial, double>::const_iterator itr = tazLevelLogsum.find(taz);

	if (itr != tazLevelLogsum.end())
	{
		return (*itr).second;
	}

	for(int n = 0; n < tazLogsumWeights.size(); n++)
	{
		PredayPersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( tazLogsumWeights[n]->getIndividualId(), taz, workTaz, vehicleOwnership );
		double lg = personParam.getDpbLogsum();
		double weight = tazLogsumWeights[n]->getWeight();

		Individual *individual = this->getIndividualById(tazLogsumWeights[n]->getIndividualId());
		Household  *household = this->getHouseholdById( individual->getHouseholdId() );
		float hhSize = household->getSize();

		logsum = logsum + (lg * weight / hhSize);
	}

	printTazLevelLogsum(taz, logsum);

	mtx.lock();
	tazLevelLogsum.insert( std::make_pair<BigSerial,double>( BigSerial(taz),double(logsum) ));
	mtx.unlock();

	return logsum;
}

HousingMarket* HM_Model::getMarket()
{
	return &market;
}

void HM_Model::incrementAwakeningCounter()
{
	initialHHAwakeningCounter++;
}

int HM_Model::getAwakeningCounter() const
{
	return initialHHAwakeningCounter;
}

void HM_Model::incrementLifestyle1HHs()
{
	numLifestyle1HHs++;
}

void HM_Model::incrementLifestyle2HHs()
{
	numLifestyle2HHs++;
}

void HM_Model::incrementLifestyle3HHs()
{
	numLifestyle3HHs++;
}

int HM_Model::getLifestyle1HHs() const
{
	return numLifestyle1HHs;
}

int HM_Model::getLifestyle2HHs() const
{
	return numLifestyle2HHs;
}

int HM_Model::getLifestyle3HHs() const
{
	return numLifestyle3HHs;
}

HM_Model::VehicleOwnershipCoeffList HM_Model::getVehicleOwnershipCoeffs() const
{
	return this->vehicleOwnershipCoeffs;
}

VehicleOwnershipCoefficients* HM_Model::getVehicleOwnershipCoeffsById( BigSerial id) const
{
	VehicleOwnershipCoeffMap::const_iterator itr = vehicleOwnershipCoeffsById.find(id);

		if (itr != vehicleOwnershipCoeffsById.end())
		{
			return itr->second;
		}

		return nullptr;
}

HM_Model:: TaxiAccessCoeffList HM_Model::getTaxiAccessCoeffs()const
{
	return this->taxiAccessCoeffs;
}

TaxiAccessCoefficients* HM_Model::getTaxiAccessCoeffsById( BigSerial id) const
{
		TaxiAccessCoeffMap::const_iterator itr = taxiAccessCoeffsById.find(id);

		if (itr != taxiAccessCoeffsById.end())
		{
			return itr->second;
		}

		return nullptr;
}

Taz* HM_Model::getTazById( BigSerial id) const
{
		TazMap::const_iterator itr = tazById.find(id);

		if (itr != tazById.end())
		{
			return itr->second;
		}

		return nullptr;
}

const HM_Model::TazStats* HM_Model::getTazStatsByUnitId(BigSerial unitId) const
{
	BigSerial tazId = getUnitTazId(unitId);
	if (tazId != INVALID_ID)
	{
		return getTazStats(tazId);
	}
	return nullptr;
}



HM_Model::HouseholdGroup::HouseholdGroup(BigSerial groupId, BigSerial homeTaz, double logsum):groupId(groupId),homeTaz(homeTaz),logsum(logsum){}

HM_Model::HouseholdGroup::HouseholdGroup( HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;
}

HM_Model::HouseholdGroup::HouseholdGroup(const HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;
}

HM_Model::HouseholdGroup& HM_Model::HouseholdGroup::operator=(const HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;

	return *this;
}

HM_Model::HouseholdGroup& HM_Model::HouseholdGroup::operator=( HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;

	return *this;
}

BigSerial HM_Model::HouseholdGroup::getGroupId() const
{
	return groupId;
}

BigSerial HM_Model::HouseholdGroup::getHomeTaz() const
{
	return homeTaz;
}

double HM_Model::HouseholdGroup::getLogsum() const
{
	return logsum;
}

void HM_Model::HouseholdGroup::setHomeTaz(BigSerial value)
{
	homeTaz = value;
}

void HM_Model::HouseholdGroup::setLogsum(double value)
{
	logsum = value;
}

void HM_Model::HouseholdGroup::setGroupId(BigSerial value)
{
	groupId = value;
}


void HM_Model::addUnit(Unit* unit)
{
	units.push_back(unit);
	unitsById.insert(std::pair<BigSerial,Unit*>(unit->getId(), unit));
}

std::vector<BigSerial> HM_Model::getRealEstateAgentIds()
{
	return this->realEstateAgentIds;
}

HM_Model::VehicleOwnershipLogsumList HM_Model::getVehicleOwnershipLosums()const
{
	return this->vehicleOwnershipLogsums;
}

LogSumVehicleOwnership* HM_Model::getVehicleOwnershipLogsumsById( BigSerial id) const
{
	VehicleOwnershipLogsumMap::const_iterator itr = vehicleOwnershipLogsumById.find(id);

		if (itr != vehicleOwnershipLogsumById.end())
		{
			return itr->second;
		}

		return nullptr;
}

HM_Model::DistMRTList HM_Model::getDistanceMRT()const
{
	return this->mrtDistances;
}

DistanceMRT* HM_Model::getDistanceMRTById( BigSerial id) const
{
	DistMRTMap::const_iterator itr = mrtDistancesById.find(id);

	if (itr != mrtDistancesById.end())
		{
			return itr->second;
		}

	return nullptr;
}

void HM_Model::getScreeningProbabilities(std::string hitsId, vector<double> &householdScreeningProbabilities )
{
	for( int n = 0; n < hits2008ScreeningProb.size(); n++ )
	{
		if( hits2008ScreeningProb[n]->getH1HhId() == hitsId )
		{
			hits2008ScreeningProb[n]->getProbabilities(householdScreeningProbabilities);
			break;
		}
	}
}

std::vector<Alternative*> HM_Model::getAlternatives()
{
	return alternative;
}

Alternative* HM_Model::getAlternativeById(int id)
{
	AlternativeMap::const_iterator itr = alternativeById.find(id);

	if (itr != alternativeById.end())
		return itr->second;
	else
		return nullptr;
}

PlanningArea* HM_Model::getPlanningAreaById( int id )
{
	PlanningAreaMap::const_iterator itr = planningAreaById.find(id);

	if( itr != planningAreaById.end())
		return itr->second;
	else
		return nullptr;
}

std::vector<PlanningSubzone*> HM_Model::getPlanningSubZoneByPlanningAreaId(int id)
{
	std::vector<PlanningSubzone*> vecPlanningSubzone;
	for(int n = 0; n < planningSubzone.size(); n++ )
	{
		if(planningSubzone[n]->getPlanningAreaId() == id )
			vecPlanningSubzone.push_back( planningSubzone[n]);
	}

	return vecPlanningSubzone;
}

std::vector<Mtz*> HM_Model::getMtzBySubzoneVec( std::vector<PlanningSubzone*> vecPlanningSubzone )
{
	std::vector<Mtz*> vecMtz;
	for(int n = 0; n < mtz.size(); n++ )
	{
		for(int m =0; m < vecPlanningSubzone.size(); m++ )
		{
			if(mtz[n]->getPlanningSubzoneId() == vecPlanningSubzone[m]->getId() )
				vecMtz.push_back( mtz[n]);
		}
	}

	return vecMtz;
}

int HM_Model::getMtzIdByTazId(int tazId)
{
	for( int n = 0; n < mtzTaz.size(); n++)
	{
		if( mtzTaz[n]->getTazId() == tazId )
			return mtzTaz[n]->getMtzId();
	}

	return 0;
}

PlanningSubzone* HM_Model::getPlanningSubzoneById(int id)
{
	PlanningSubzoneMap::const_iterator itr = planningSubzoneById.find(id);

	if (itr != planningSubzoneById.end())
	{
		return itr->second;
	}

	return nullptr;
}

Mtz* HM_Model::getMtzById( int id)
{
	MtzMap::const_iterator itr = mtzById.find(id);

	if (itr != mtzById.end())
	{
		return itr->second;
	}

	return nullptr;
}

std::vector<BigSerial> HM_Model::getTazByMtzVec( std::vector<Mtz*> vecMtz )
{
	std::vector<BigSerial> vecTaz;
	for(int n = 0; n < mtzTaz.size(); n++ )
	{
		for( int m = 0; m < vecMtz.size(); m++ )
		{
			if( mtzTaz[n]->getMtzId() == vecMtz[m]->getId() )
			{
				vecTaz.push_back( mtzTaz[n]->getTazId());
			}
		}
	}

	return vecTaz;
}

ZonalLanduseVariableValues* HM_Model::getZonalLandUseByAlternativeId(int id)const
{
	ZonalLanduseVariableValuesMap::const_iterator itr = zonalLanduseVariableValuesById.find(id);

	if (itr != zonalLanduseVariableValuesById.end())
	{
		return itr->second;
	}

	return nullptr;
}

Alternative* HM_Model::getAlternativeByPlanningAreaId(int id) const
{
	for( int n = 0; n < alternative.size(); n++ )
	{
		if(alternative[n]->getPlanAreaId() == id)
			return alternative[n];
	}

	return nullptr;
}

HM_Model::HouseHoldHitsSampleList HM_Model::getHouseHoldHits()const
{
	return this->houseHoldHits;
}

HouseHoldHitsSample* HM_Model::getHouseHoldHitsById( BigSerial id) const
{
	HouseHoldHitsSampleMap::const_iterator itr = houseHoldHitsById.find(id);

	if (itr != houseHoldHitsById.end())
	{
		return itr->second;
	}

	return nullptr;
}

HM_Model::HouseholdGroup* HM_Model::getHouseholdGroupByGroupId(BigSerial id)const
{
	boost::unordered_map<BigSerial, HouseholdGroup*>::const_iterator itr = vehicleOwnerhipHHGroupByGroupId.find(id);

		if (itr != vehicleOwnerhipHHGroupByGroupId.end())
		{
			return itr->second;
		}

		return nullptr;
}


std::vector<PopulationPerPlanningArea*> HM_Model::getPopulationByPlanningAreaId(BigSerial id) const
{
	std::vector<PopulationPerPlanningArea*> populationPPAvector;

	for( int n = 0; n < populationPerPlanningArea.size(); n++ )
	{
		if( populationPerPlanningArea[n]->getPlanningAreaId() == id )
			populationPPAvector.push_back(populationPerPlanningArea[n]);
	}


	return populationPPAvector;
}


HM_Model::HitsIndividualLogsumList HM_Model::getHitsIndividualLogsumVec() const
{
	return hitsIndividualLogsum;
}

void HM_Model::setStartDay(int day)
{
	startDay = day;
}

int HM_Model::getStartDay() const
{
	return this->startDay;
}

void HM_Model::addHouseholdGroupByGroupId(HouseholdGroup* hhGroup)
{
	mtx2.lock();
	this->vehicleOwnerhipHHGroupByGroupId.insert(std::make_pair(hhGroup->getGroupId(),hhGroup));
	mtx2.unlock();
}

void HM_Model::setTaxiAccess(const Household *household)
{
	double valueTaxiAccess = getTaxiAccessCoeffsById(INTERCEPT)->getCoefficientEstimate();
	//finds out whether the household is an HDB or not
	int unitTypeId = 0;

	if(getUnitById(household->getUnitId()) != nullptr)
	{
		unitTypeId = getUnitById(household->getUnitId())->getUnitType();
	}

	if( (unitTypeId>0) && (unitTypeId<=6))
	{

		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(HDB1)->getCoefficientEstimate();
	}

	std::vector<BigSerial> individuals = household->getIndividuals();
	int numIndividualsInAge5064 = 0;
	int numIndividualsInAge65Up = 0;
	int numIndividualsAge1019 = 0;
	int numSelfEmployedIndividuals = 0;
	int numRetiredIndividuals = 0;
	int numServiceIndividuals = 0;
	int numProfIndividuals = 0;
	int numLabourIndividuals = 0;
	int numManagerIndividuals = 0;
	int numOperatorIndividuals = 0;

	std::vector<BigSerial>::iterator individualsItr;
	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		int ageCategoryId = getIndividualById((*individualsItr))->getAgeCategoryId();
		//IndividualsAge1019
		if((ageCategoryId==2) || (ageCategoryId==3))
		{
			numIndividualsAge1019++;
		}
		//IndividualsInAge5064
		if((ageCategoryId >= 10)&& (ageCategoryId <= 12))
		{
			numIndividualsInAge5064++;
		}
		//IndividualsInAge65Up
		if((ageCategoryId >= 13) && (ageCategoryId <= 17))
		{
			numIndividualsInAge65Up++;
		}
		//SelfEmployedIndividuals
		if(getIndividualById((*individualsItr))->getEmploymentStatusId()==3)
		{
			numSelfEmployedIndividuals++;
		}
		//RetiredIndividuals
		if(getIndividualById((*individualsItr))->getEmploymentStatusId()==6)
		{
			numRetiredIndividuals++;
		}
		//individuals in service sector
		if(getIndividualById((*individualsItr))->getOccupationId() == 5)
		{
			numServiceIndividuals++;
		}
		//Professional Individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 2)
		{
			numProfIndividuals++;
		}
		//Manager individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 2)
		{
			numManagerIndividuals++;
		}
		//Operator individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 6)
		{
			numOperatorIndividuals++;
		}
		//labour individuals : occupation type = other
		if(getIndividualById((*individualsItr))->getOccupationId() == 7)
		{
			numLabourIndividuals++;
		}
	}

	if(numIndividualsInAge5064 == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE5064_1)->getCoefficientEstimate();
	}
	else if (numIndividualsInAge5064 >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE5064_2)->getCoefficientEstimate();
	}
	if(numIndividualsInAge65Up == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE65UP_1)->getCoefficientEstimate();
	}
	else if (numIndividualsInAge65Up >= 2 )
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE65UP_2)->getCoefficientEstimate();
	}
	if(numIndividualsAge1019 >=2 )
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE1019_2)->getCoefficientEstimate();
	}
	if(numSelfEmployedIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(EMPLOYED_SELF_1)->getCoefficientEstimate();
	}
	else if(numSelfEmployedIndividuals >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(EMPLOYED_SELF_2)->getCoefficientEstimate();
	}
	if(numRetiredIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(RETIRED_1)->getCoefficientEstimate();
	}
	else if (numRetiredIndividuals >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(RETIRED_2)->getCoefficientEstimate();
	}

	const double incomeLaw = 3000;
	const double incomeHigh = 10000;
	if(household->getIncome() <= incomeLaw)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INC_LOW)->getCoefficientEstimate();
	}
	else if (household->getIncome() > incomeHigh)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INC_HIGH)->getCoefficientEstimate();
	}

	if(numOperatorIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(OPERATOR_1)->getCoefficientEstimate();
	}
	else if(numOperatorIndividuals >=2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(OPERATOR_2)->getCoefficientEstimate();
	}

	if(numServiceIndividuals >=2 )
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(SERVICE_2)->getCoefficientEstimate();
	}

	if(numProfIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(PROF_1)->getCoefficientEstimate();
	}
	if(numLabourIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(LABOR_1)->getCoefficientEstimate();
	}
	if(numManagerIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(MANAGER_1)->getCoefficientEstimate();
	}
	//Indian
	if(household->getEthnicityId() == 3)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INDIAN_TAXI_ACCESS)->getCoefficientEstimate();
	}
	//Malay
	if(household->getEthnicityId() == 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(MALAY_TAXI_ACCESS)->getCoefficientEstimate();
	}

	double expTaxiAccess = exp(valueTaxiAccess);
	double probabilityTaxiAccess = (expTaxiAccess) / (1 + expTaxiAccess);

	/*generate a random number between 0-1
	* time(0) is passed as an input to constructor in order to randomize the result
	*/
	boost::mt19937 randomNumbergenerator( time( 0 ) );
	boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
	boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >generateRandomNumbers( randomNumbergenerator, uniformDistribution );
	const double randomNum = generateRandomNumbers( );
	if(randomNum < probabilityTaxiAccess)
	{
		writeTaxiAvailabilityToFile(household->getId());
		hasTaxiAccess = true;
		AgentsLookup& lookup = AgentsLookupSingleton::getInstance();
		const HouseholdAgent* householdAgent = lookup.getHouseholdAgentById(household->getId());
		MessageBus::PostMessage(const_cast<HouseholdAgent*>(householdAgent), LTMID_HH_TAXI_AVAILABILITY, MessageBus::MessagePtr(new Message()));
	}
}

void HM_Model::startImpl()
{
	PredayLT_LogsumManager::getInstance();


	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	MetadataEntry entry;

	// Loads necessary data from database.
	DB_Config dbConfig(LT_DB_CONFIG_FILE);
	dbConfig.load();
	// Connect to database and load data for this model.
	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();

	bool resume = config.ltParams.resume;

	if (conn.isConnected())
	{
		//load individuals
		loadData<IndividualDao>(conn, individuals, individualsById,	&Individual::getId);
		PrintOutV("Initial Individuals: " << individuals.size() << std::endl);

		//Load households
		loadData<HouseholdDao>(conn, households, householdsById, &Household::getId);
		PrintOutV("Number of households: " << households.size() << ". Households used: " << households.size()  << std::endl);

		//Load units
		loadData<UnitDao>(conn, units, unitsById, &Unit::getId);
		PrintOutV("Number of units: " << units.size() << ". Units Used: " << units.size() << std::endl);

		loadData<AwakeningDao>(conn, awakening, awakeningById,	&Awakening::getId);
		PrintOutV("Awakening probability: " << awakening.size() << std::endl );

		loadData<PostcodeDao>(conn, postcodes, postcodesById,	&Postcode::getAddressId);
		PrintOutV("Number of postcodes: " << postcodes.size() << std::endl );

		loadData<VehicleOwnershipCoefficientsDao>(conn,vehicleOwnershipCoeffs,vehicleOwnershipCoeffsById, &VehicleOwnershipCoefficients::getParameterId);
		PrintOutV("Vehicle Ownership coefficients: " << vehicleOwnershipCoeffs.size() << std::endl );

		loadData<TaxiAccessCoefficientsDao>(conn,taxiAccessCoeffs,taxiAccessCoeffsById, &TaxiAccessCoefficients::getParameterId);
		PrintOutV("Taxi access coefficients: " << taxiAccessCoeffs.size() << std::endl );

		loadData<EstablishmentDao>(conn, establishments, establishmentsById, &Establishment::getId);
		PrintOutV("Number of establishments: " << establishments.size() << std::endl );

		loadData<JobDao>( conn, jobs, jobsById, &Job::getId);
		PrintOutV("Number of jobs: " << jobs.size() << std::endl );

		loadData<HousingInterestRateDao>( conn, housingInterestRates, housingInterestRatesById, &HousingInterestRate::getId);
		PrintOutV("Number of interest rate quarters: " << housingInterestRates.size() << std::endl );

		loadData<LogSumVehicleOwnershipDao>( conn, vehicleOwnershipLogsums, vehicleOwnershipLogsumById, &LogSumVehicleOwnership::getHouseholdId);
		PrintOutV("Number of vehicle ownership logsums: " << vehicleOwnershipLogsums.size() << std::endl );

		loadData<DistanceMRTDao>( conn, mrtDistances, mrtDistancesById, &DistanceMRT::getHouseholdId);
		PrintOutV("Number of mrt distances: " << mrtDistances.size() << std::endl );

		loadData<TazDao>( conn, tazs, tazById, &Taz::getId);
		PrintOutV("Number of taz: " << tazs.size() << std::endl );

		loadData<HouseHoldHitsSampleDao>( conn, houseHoldHits, houseHoldHitsById, &HouseHoldHitsSample::getHouseholdId);
		PrintOutV("Number of houseHoldHits: " << houseHoldHits.size() << std::endl );

		loadData<TazLogsumWeightDao>( conn, tazLogsumWeights, tazLogsumWeightById, &TazLogsumWeight::getGroupLogsum );
		PrintOutV("Number of tazLogsumWeights: " << tazLogsumWeights.size() << std::endl );

		loadData<LogsumMtzV2Dao>( conn, logsumMtzV2, logsumMtzV2ById, &LogsumMtzV2::getV2 );
		PrintOutV("Number of LogsumMtzV2: " << logsumMtzV2.size() << std::endl );

		loadData<PlanningAreaDao>( conn, planningArea, planningAreaById, &PlanningArea::getId );
		PrintOutV("Number of planning areas: " << planningArea.size() << std::endl );

		loadData<PlanningSubzoneDao>( conn, planningSubzone, planningSubzoneById, &PlanningSubzone::getId );
		PrintOutV("Number of planing subzones: " << planningSubzone.size() << std::endl );

		loadData<MtzDao>( conn, mtz, mtzById, &Mtz::getId );
		PrintOutV("Number of Mtz: " << mtz.size() << std::endl );

		loadData<MtzTazDao>( conn, mtzTaz, mtzTazById, &MtzTaz::getMtzId );
		PrintOutV("Number of mtz taz lookups: " << mtzTaz.size() << std::endl );

		loadData<AlternativeDao>( conn, alternative, alternativeById, &Alternative::getId );
		PrintOutV("Number of alternative region names: " << alternative.size() << std::endl );

		//only used with Hits2008 data
		//loadData<Hits2008ScreeningProbDao>( conn, hits2008ScreeningProb, hits2008ScreeningProbById, &Hits2008ScreeningProb::getId );
		//PrintOutV("Number of hits2008 screening probabilities: " << hits2008ScreeningProb.size() << std::endl );

		loadData<ZonalLanduseVariableValuesDao>( conn, zonalLanduseVariableValues, zonalLanduseVariableValuesById, &ZonalLanduseVariableValues::getAltId );
		PrintOutV("Number of zonal landuse variable values: " << zonalLanduseVariableValues.size() << std::endl );

		loadData<PopulationPerPlanningAreaDao>( conn, populationPerPlanningArea, populationPerPlanningAreaById, &PopulationPerPlanningArea::getPlanningAreaId );
		PrintOutV("Number of PopulationPerPlanningArea rows: " << populationPerPlanningArea.size() << std::endl );

		loadData<HitsIndividualLogsumDao>( conn, hitsIndividualLogsum, hitsIndividualLogsumById, &HitsIndividualLogsum::getId );
		PrintOutV("Number of Hits Individual Logsum rows: " << hitsIndividualLogsum.size() << std::endl );


		loadData<IndvidualVehicleOwnershipLogsumDao>( conn, IndvidualVehicleOwnershipLogsums, IndvidualVehicleOwnershipLogsumById, &IndvidualVehicleOwnershipLogsum::getHouseholdId );
		PrintOutV("Number of Hits Individual VehicleOwnership Logsum rows: " << IndvidualVehicleOwnershipLogsums.size() << std::endl );

		loadData<ScreeningCostTimeDao>( conn, screeningCostTime, screeningCostTimeById, &ScreeningCostTime::getId );
		PrintOutV("Number of Screening Cost Time rows: " << screeningCostTime.size() << std::endl );

		loadData<AccessibilityFixedPzidDao>( conn, accessibilityFixedPzid, accessibilityFixedPzidById, &AccessibilityFixedPzid::getId );
		PrintOutV("Number of Accessibility fixed pz id rows: " << accessibilityFixedPzid.size() << std::endl );

		loadData<TenureTransitionRateDao>( conn, tenureTransitionRate, tenureTransitionRateById, &TenureTransitionRate::getId );
		PrintOutV("Number of Tenure Transition rate rows: " << tenureTransitionRate.size() << std::endl );

		loadData<OwnerTenantMovingRateDao>( conn, ownerTenantMovingRate, ownerTenantMovingRateById, &OwnerTenantMovingRate::getId );
		PrintOutV("Number of Owner Tenant Moving Rate rows: " << ownerTenantMovingRate.size() << std::endl );

		if(resume)
		{
			std::string  outputSchema = config.ltParams.currentOutputSchema;
			SimulationStoppedPointDao simStoppedPointDao(conn);
			const std::string getAllSimStoppedPointParams = "SELECT * FROM " + outputSchema+ "."+"simulation_stopped_point;";
			simStoppedPointDao.getByQuery(getAllSimStoppedPointParams,simStoppedPointList);
			if(!simStoppedPointList.empty())
			{
				bidId = simStoppedPointList[simStoppedPointList.size()-1]->getBidId();
				unitSaleId = simStoppedPointList[simStoppedPointList.size()-1]->getUnitSaleId();
			}
			BidDao bidDao(conn);
			db::Parameters params;
		    params.push_back(lastStoppedDay);
			const std::string getResumptionBidsOnLastDay = "SELECT * FROM " + outputSchema+ "."+"bids" + " WHERE simulation_day = :v1;";
			bidDao.getByQueryId(getResumptionBidsOnLastDay,params,resumptionBids);

			const std::string getAllResumptionHouseholds = "SELECT * FROM " + outputSchema+ "."+"household;";

			HouseholdDao hhDao(conn);
			hhDao.getByQuery(getAllResumptionHouseholds,resumptionHouseholds);
			//Index all resumed households.
			for (HouseholdList::iterator it = resumptionHouseholds.begin(); it != resumptionHouseholds.end(); ++it) {
				resumptionHHById.insert(std::make_pair((*it)->getId(), *it));
			}

			const std::string getAllVehicleOwnershipChanges = "SELECT * FROM " + outputSchema+ "."+"vehicle_ownership_changes;";
			VehicleOwnershipChangesDao vehicleOwnershipChangesDao(conn);
			vehicleOwnershipChangesDao.getByQuery(getAllVehicleOwnershipChanges,vehOwnershipChangesList);
			for (VehicleOwnershipChangesList::iterator it = vehOwnershipChangesList.begin(); it != vehOwnershipChangesList.end(); ++it) {
				vehicleOwnershipChangesById.insert(std::make_pair((*it)->getHouseholdId(), *it));
			}

		}


	}


	//Create a map that concatanates origin and destination PA for faster lookup.
	for(int n = 0; n < screeningCostTime.size(); n++ )
	{
		std::string costTime = std::to_string(screeningCostTime[n]->getPlanningAreaOrigin() ) + "-" + std::to_string(screeningCostTime[n]->getPlanningAreaDestination());
		screeningCostTimeSuperMap.insert({costTime, screeningCostTime[n]->getId()});
	}

	unitsFiltering();

	workGroup.assignAWorker(&market);
	int numWorkers = workGroup.getNumberOfWorkers();

	//
	//Create freelance seller agents to sell vacant units.
	//
	std::vector<HouseholdAgent*> freelanceAgents;
	for (int i = 0; i < numWorkers ; i++)
	{
		HouseholdAgent* freelanceAgent = new HouseholdAgent((FAKE_IDS_START + i),this, nullptr, &market, true, startDay, config.ltParams.housingModel.householdBiddingWindow);
		AgentsLookupSingleton::getInstance().addHouseholdAgent(freelanceAgent);
		agents.push_back(freelanceAgent);
		workGroup.assignAWorker(freelanceAgent);
		freelanceAgents.push_back(freelanceAgent);
	}


	//
	//Create real-estate agents. Their tasks are to sell units from the developer model.
	//
	std::vector<RealEstateAgent*> realEstateAgents;
	for( int i = 0; i < numWorkers ; i++ )
	{
		BigSerial id = FAKE_IDS_START + numWorkers + i;
		realEstateAgentIds.push_back(id);
		RealEstateAgent* realEstateAgent = new RealEstateAgent(id, this, nullptr, &market, true,startDay);
		AgentsLookupSingleton::getInstance().addRealEstateAgent(realEstateAgent);
		agents.push_back(realEstateAgent);
		workGroup.assignAWorker(realEstateAgent);
		realEstateAgents.push_back(realEstateAgent);
	}



	int homelessHousehold = 0;

	//
	// 1. Create Household Agents.
	// 2. Assign households to the units.
	//
	for (HouseholdList::iterator it = households.begin();	it != households.end(); it++)
	{
		Household* household = *it;
		Household *resumptionHH = getResumptionHouseholdById(household->getId());
		if(resume)
		{
			if ((resumptionHH != nullptr) && (resumptionHH->getHasMoved()))//update the unit id of the households moved to new units.
			{
				household->setUnitId(getResumptionHouseholdById(household->getId())->getUnitId());
			}

			if(getVehicleOwnershipChangesByHHId(household->getId()) != nullptr) //update the vehicle ownership option of the households that change vehicles.
			{
				household->setVehicleOwnershipOptionId(getVehicleOwnershipChangesByHHId(household->getId())->getNewVehicleOwnershipOptionId());
			}
		}
		HouseholdAgent* hhAgent = new HouseholdAgent(household->getId(), this,	household, &market, false, startDay, config.ltParams.housingModel.householdBiddingWindow);
		if (resumptionHH != nullptr)
		{
			if(resumptionHH->getIsBidder())
			{
				hhAgent->getBidder()->setActive(true);
				if(!resumptionHH->getHasMoved())
				{
					hhAgent->getBidder()->setMovInWaitingTimeInDays(resumptionHH->getMoveInDate().tm_mday - startDay);
				}
			}
			else if(resumptionHH->getIsSeller())
			{
				hhAgent->getSeller()->setActive(true);
			}
		}
		const Unit* unit = getUnitById(household->getUnitId());

		if (unit)
		{
			hhAgent->addUnitId(unit->getId());
			assignedUnits.insert(std::make_pair(unit->getId(), unit->getId()));
		}
		else
		{
			homelessHousehold++;
		}

		BigSerial tazId = getUnitTazId(household->getUnitId());
		if (tazId != INVALID_ID)
		{
			const HM_Model::TazStats* tazStats = getTazStatsByUnitId( household->getUnitId());
			if (!tazStats)
			{
				tazStats = new TazStats(tazId);
				stats.insert( std::make_pair(tazId,	const_cast<HM_Model::TazStats*>(tazStats)));
			}

			const_cast<HM_Model::TazStats*>(tazStats)->updateStats(*household);
		}

		AgentsLookupSingleton::getInstance().addHouseholdAgent(hhAgent);
		agents.push_back(hhAgent);
		workGroup.assignAWorker(hhAgent);
	}


	int totalPopulation = 0;
	for ( StatsMap::iterator it = stats.begin(); it != stats.end(); ++it )
	{
		#ifdef VERBOSE
		PrintOutV("Taz: " << it->first << std::fixed << std::setprecision(2)
						  << " \tAvg Income: " << it->second->getHH_AvgIncome()
						  << " \t%Chinese: " << it->second->getChinesePercentage()
						  << " \t%Malay: " << it->second->getMalayPercentage()
						  << " \t%Indian: " << it->second->getIndianPercentage()
						  << " \tAvg HH size: " << it->second->getAvgHHSize()
						  << " \tTaz Households: " << it->second->getHH_Num()
		  	  	  	  	  << " \tTaz population: " << it->second->getIndividuals()
						  << std::endl);
		#endif

		totalPopulation += it->second->getIndividuals();
	}

	PrintOutV("total Population: " << totalPopulation << std::endl);

	PrintOutV( "There are " << homelessHousehold << " homeless households" << std::endl);

	///////////////////////////////////////////
	//Vacant Unit activation model
	//////////////////////////////////////////
	int vacancies = 0;
	int onMarket  = 0;
	int offMarket = 0;
	//assign empty units to freelance housing agents
	for (UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		(*it)->setbiddingMarketEntryDay( 365 );
		(*it)->setTimeOnMarket(config.ltParams.housingModel.timeOnMarket);
		(*it)->setTimeOffMarket(config.ltParams.housingModel.timeOffMarket);

		//this unit is a vacancy
		if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
		{
			if( (*it)->getUnitType() != NON_RESIDENTIAL_PROPERTY )
			{
				float awakeningProbability = (float)rand() / RAND_MAX;

				if( awakeningProbability < config.ltParams.housingModel.vacantUnitActivationProbability )
				{
					(*it)->setbiddingMarketEntryDay( startDay );
					(*it)->setTimeOnMarket( 1 + int((float)rand() / RAND_MAX * ( config.ltParams.housingModel.timeOnMarket )) );

					onMarket++;
				}
				else
				{
					(*it)->setbiddingMarketEntryDay( (float)rand() / RAND_MAX * ( config.ltParams.housingModel.timeOnMarket + config.ltParams.housingModel.timeOffMarket));
					offMarket++;
				}

				freelanceAgents[vacancies % numWorkers]->addUnitId((*it)->getId());
				vacancies++;
			}
		}

		{
			Unit *thisUnit = (*it);

			PostcodeMap::iterator itrPC  =  postcodesById.find((*it)->getSlaAddressId());
			int tazId = (*itrPC).second->getTazId();
			int mtzId = -1;
			int subzoneId = -1;
			int planningAreaId = -1;

			for(int n = 0; n < mtzTaz.size();n++)
			{
				if(tazId == mtzTaz[n]->getTazId() )
				{
					mtzId = mtzTaz[n]->getMtzId();
					break;
				}
			}

			for(int n = 0; n < mtz.size(); n++)
			{
				if( mtzId == mtz[n]->getId())
				{
					subzoneId = mtz[n]->getPlanningSubzoneId();
					break;
				}
			}

			for( int n = 0; n < planningSubzone.size(); n++ )
			{
				if( subzoneId == planningSubzone[n]->getId() )
				{
					planningAreaId = planningSubzone[n]->getPlanningAreaId();
					break;
				}
			}

			if( thisUnit->getUnitType()  == 1 || thisUnit->getUnitType() == 2)
			{
				thisUnit->setDwellingType(100);
			}
			else
			if( thisUnit->getUnitType() == 3)
			{
				thisUnit->setDwellingType(300);
			}
			else
			if( thisUnit->getUnitType() == 4)
			{
				thisUnit->setDwellingType(400);
			}
			else
			if( thisUnit->getUnitType() == 5)
			{
				thisUnit->setDwellingType(500);
			}
			else
			if(( thisUnit->getUnitType() >=7 && thisUnit->getUnitType() <=16 ) || ( thisUnit->getUnitType() >= 32 && thisUnit->getUnitType() <= 36 ) )
			{
				thisUnit->setDwellingType(600);
			}
			else
			if( thisUnit->getUnitType() >= 17 && thisUnit->getUnitType() <= 31 )
			{
				thisUnit->setDwellingType(700);
			}
			else
			{
				thisUnit->setDwellingType(800);
			}

			for( int n = 0; n < alternative.size(); n++)
			{
				if( thisUnit->getDwellingType() == alternative[n]->getDwellingTypeId() &&
					planningAreaId   == alternative[n]->getPlanAreaId() )
				{
					thisUnit->setZoneHousingType(alternative[n]->getId());

					//PrintOutV(" " << thisUnit->getId() << " " << alternative[n]->getPlanAreaId() << std::endl );
					unitsByZoneHousingType.insert( std::pair<BigSerial,Unit*>( alternative[n]->getId(), thisUnit ) );
					break;
				}
			}
		}
	}

	PrintOutV("Initial Vacant units: " << vacancies << " onMarket: " << onMarket << " offMarket: " << offMarket << std::endl);


	addMetadata("Initial Units", units.size());
	addMetadata("Initial Households", households.size());
	addMetadata("Initial Vacancies", vacancies);
	addMetadata("Freelance housing agents", numWorkers);

	for (size_t n = 0; n < individuals.size(); n++)
	{
		BigSerial householdId = individuals[n]->getHouseholdId();

		Household *tempHH = getHouseholdById(householdId);

		if (tempHH != nullptr)
			tempHH->setIndividual(individuals[n]->getId());
	}

	for (size_t n = 0; n < households.size(); n++)
	{
		hdbEligibilityTest(n);
		setTaxiAccess(households[n]);
		//reconsiderVehicleOwnershipOption(households[n]);

	}
	//PrintOut("taxi access available for "<<count<<" number of hh"<<std::endl);

	PrintOutV("The synthetic population contains " << household_stats.adultSingaporean_global << " adult Singaporeans." << std::endl);
	PrintOutV("Minors. Male: " << household_stats.maleChild_global << " Female: " << household_stats.femaleChild_global << std::endl);
	PrintOutV("Young adults. Male: " << household_stats.maleAdultYoung_global << " Female: " << household_stats.femaleAdultYoung_global << std::endl);
	PrintOutV("Middle-age adults. Male: " << household_stats.maleAdultMiddleAged_global << " Female: " << household_stats.femaleAdultMiddleAged_global << std::endl);
	PrintOutV("Elderly adults. Male: " << household_stats.maleAdultElderly_global << " Female: " << household_stats.femaleAdultElderly_global << std::endl);
	PrintOutV("Household type Enumeration" << std::endl);
	PrintOutV("Couple and child " << household_stats.coupleAndChild << std::endl);
	PrintOutV("Siblings and parents " << household_stats.siblingsAndParents << std::endl );
	PrintOutV("Single parent " << household_stats.singleParent << std::endl );
	PrintOutV("Engaged couple " << household_stats.engagedCouple << std::endl );
	PrintOutV("Orphaned siblings " << household_stats.orphanSiblings << std::endl );
	PrintOutV("Multigenerational " << household_stats.multigeneration << std::endl );

}

std::multimap<BigSerial, Unit*> HM_Model::getUnitsByZoneHousingType()
{
	return unitsByZoneHousingType;
}

void HM_Model::getLogsumOfIndividuals(BigSerial id)
{
	Household *currentHousehold = getHouseholdById( id );

	BigSerial tazId = getUnitTazId( currentHousehold->getUnitId());
	Taz *tazObj = getTazById( tazId );

	std::string tazStr;
	if( tazObj != NULL )
		tazStr = tazObj->getName();

	BigSerial taz = std::atoi( tazStr.c_str() );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		PredayPersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n], taz, -1, 1 );
		double logsum =  personParam.getDpbLogsum();

		printIndividualHitsLogsum( householdIndividualIds[n], logsum );
	}
}


void HM_Model::getLogsumOfHouseholdVO(BigSerial householdId2)
{
	BigSerial householdId = 0;
	HouseHoldHitsSample *hitsSample = nullptr;

	{
		boost::mutex::scoped_lock lock( mtx3 );

		householdId = householdLogsumCounter++;

		hitsSample = this->getHouseHoldHitsById( householdId );

		if( !hitsSample )
			return;

		std::string householdHitsIdStr = hitsSample->getHouseholdHitsId();

		if( processedHouseholdHitsLogsum.find( householdHitsIdStr ) != processedHouseholdHitsLogsum.end() )
			return;
		else
			processedHouseholdHitsLogsum.insert( householdHitsIdStr );
	}

	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);

		vector<double> logsum;
		vector<double> travelProbability;
		vector<double> tripsExpected;

		int tazIdW = -1;
		int tazIdH = -1;
		int paxId  = -1;

		int p = 0;
		for(p = 0; p < hitsIndividualLogsum.size(); p++ )
		{
			if (  hitsIndividualLogsum[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
			{
				tazIdW = hitsIndividualLogsum[p]->getWorkTaz();
				tazIdH = hitsIndividualLogsum[p]->getHomeTaz();
				paxId  = hitsIndividualLogsum[p]->getPaxId();
				break;
			}
		}

		Taz *tazObjW = getTazById( tazIdW );
	    std::string tazStrW;
		if( tazObjW != NULL )
			tazStrW = tazObjW->getName();
		BigSerial tazW = std::atoi( tazStrW.c_str() );

		Taz *tazObjH = getTazById( tazIdH );
	    std::string tazStrH;
		if( tazObjH != NULL )
			tazStrH = tazObjH->getName();
		BigSerial tazH = std::atoi( tazStrH.c_str() );

		PredayPersonParams personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 );
		double logsumNoVehicle	= personParams1.getDpbLogsum();
		double travelProbNV		= personParams1.getTravelProbability();
		double tripsExpectedNV 	= personParams1.getTripsExpected();
		logsum.push_back(logsumNoVehicle);
		travelProbability.push_back(travelProbNV);
		tripsExpected.push_back(tripsExpectedNV);

		PredayPersonParams personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 );
		double logsumVehicle	= personParams2.getDpbLogsum();
		double travelProbV		= personParams2.getTravelProbability();
		double tripsExpectedV 	= personParams2.getTripsExpected();
		logsum.push_back(logsumVehicle);
		travelProbability.push_back(travelProbV);
		tripsExpected.push_back(tripsExpectedV);

		simulationStopCounter++;

		printHouseholdHitsLogsumFVO( hitsSample->getHouseholdHitsId(), paxId, householdId, householdIndividualIds[n], thisIndividual->getMemberId(), tazH, tazW, logsum, travelProbability, tripsExpected );
		PrintOutV( simulationStopCounter << ". " << hitsIndividualLogsum[p]->getHitsId() << ", " << paxId << ", " << hitsSample->getHouseholdHitsId() << ", " << householdId << ", " << thisIndividual->getMemberId()
										 << ", " << householdIndividualIds[n] << ", " << tazH << ", " << tazW << ", "
										 << std::setprecision(5)	<< logsum[0]  << ", " << logsum[1] << ", " << tripsExpected[0] << ", " << tripsExpected[1]
										 << ", " << travelProbability[0] << ", " << travelProbability[1] <<std::endl );

	}
}

void HM_Model::getLogsumOfHousehold(BigSerial householdId2)
{
	BigSerial householdId = 0;
	HouseHoldHitsSample *hitsSample = nullptr;

	{
		boost::mutex::scoped_lock lock( mtx3 );

		householdId = householdLogsumCounter++;

		hitsSample = this->getHouseHoldHitsById( householdId );

		if( !hitsSample )
			return;

		std::string householdHitsIdStr = hitsSample->getHouseholdHitsId();

		if( processedHouseholdHitsLogsum.find( householdHitsIdStr ) != processedHouseholdHitsLogsum.end() )
			return;
		else
			processedHouseholdHitsLogsum.insert( householdHitsIdStr );
	}


	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);

		int vehicleOwnership = 0;

		if( thisIndividual->getVehicleCategoryId() > 0)
			vehicleOwnership = 1;


		vector<double> logsum;
		vector<double> travelProbability;
		vector<double> tripsExpected;

		int tazId = -1;
		int paxId  = -1;
		int p = 0;
		for(p = 0; p < hitsIndividualLogsum.size(); p++ )
		{
			if (  hitsIndividualLogsum[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
			{
				tazId = hitsIndividualLogsum[p]->getHomeTaz();
				paxId  = hitsIndividualLogsum[p]->getPaxId();

				break;
			}
		}

		Taz *tazObj = getTazById( tazId );
	    std::string tazStr;
		if( tazObj != NULL )
			tazStr = tazObj->getName();
		BigSerial tazHome = std::atoi( tazStr.c_str() );

		for( int m = 0; m < this->tazs.size(); m++)
		{
			Taz *tazObjList = getTazById( m );
		    std::string tazStrList;
			if( tazObjList != NULL )
				tazStrList = tazObjList->getName();
			BigSerial tazList = std::atoi( tazStrList.c_str() );

			PredayPersonParams personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazHome, tazList, vehicleOwnership );

			double logsumD 				= personParams.getDpbLogsum();
 			double travelProbabilityD	= personParams.getTravelProbability();
			double tripsExpectedD		= personParams.getTripsExpected();

			logsum.push_back(logsumD);
			travelProbability.push_back(travelProbabilityD);
			tripsExpected.push_back(tripsExpectedD);
		}

		simulationStopCounter++;

		printHouseholdHitsLogsum( "logsum", hitsSample->getHouseholdHitsId() , householdId, householdIndividualIds[n], paxId, logsum );
		printHouseholdHitsLogsum( "travelProbability", hitsSample->getHouseholdHitsId() , householdId, householdIndividualIds[n], paxId, travelProbability );
		printHouseholdHitsLogsum( "tripsExpected", hitsSample->getHouseholdHitsId() , householdId, householdIndividualIds[n], paxId, tripsExpected );
		PrintOutV( simulationStopCounter << ". " << hitsIndividualLogsum[p]->getHitsId() << ", " << hitsSample->getHouseholdHitsId() << ", " << householdId << ", " << paxId << ", " << thisIndividual->getMemberId() << ", " << householdIndividualIds[n] << ", " << std::setprecision(5)
														<< logsum[0]  << ", " << logsum[1]  << ", " << logsum[2]  << ", " << logsum[3]  << ", " << logsum[4]  << ", " << logsum[5]  << ", " << logsum[6]  << ", " << logsum[7]  << ", " << logsum[8]  << ", " << logsum[9]  << ", " << logsum[10]  << ", " << logsum[11]  << ", " << logsum[12]  << ", " << logsum[13]
														<< ", " << logsum[14]  << ", " << logsum[15]  << ", " << logsum[16]  << ", " << logsum[17]  << ", " << logsum[18]  << ", " << logsum[19]  << ", " << logsum[20]  << ", " << logsum[21]  << ", " << logsum[22]  << ", " << logsum[23]  << ", " << logsum[24]  << ", " << logsum[25]  << ", " << logsum[26]  << ", " << logsum[27]
														<< ", " << logsum[28]  << ", " << logsum[29]  << ", " << logsum[30]  << ", " << logsum[31]  << ", " << logsum[32]  << ", " << logsum[33]  << ", " << logsum[34]  << ", " << logsum[35]  << ", " << logsum[36]  << ", " << logsum[37]  << ", " << logsum[38]  << ", " << logsum[39]  << ", " << logsum[40]  << ", " << logsum[41]
														<< ", " << logsum[42]  << ", " << logsum[43]  << ", " << logsum[44]  << ", " << logsum[45]  << ", " << logsum[46]  << ", " << logsum[47]  << ", " << logsum[48]  << ", " << logsum[49]  << ", " << logsum[50]  << ", " << logsum[51]  << ", " << logsum[52]  << ", " << logsum[53]  << ", " << logsum[54]  << ", " << logsum[55]
														<< ", " << logsum[56]  << ", " << logsum[57]  << ", " << logsum[58]  << ", " << logsum[59]  << ", " << logsum[60]  << ", " << logsum[61]  << ", " << logsum[62]  << ", " << logsum[63]  << ", " << logsum[64]  << ", " << logsum[65]  << ", " << logsum[66]  << ", " << logsum[67]  << ", " << logsum[68]  << ", " << logsum[69]
														<< ", " << logsum[70]  << ", " << logsum[71]  << ", " << logsum[72]  << ", " << logsum[73]  << ", " << logsum[74]  << ", " << logsum[75]  << ", " << logsum[76]  << ", " << logsum[77]  << ", " << logsum[78]  << ", " << logsum[79]  << ", " << logsum[80]  << ", " << logsum[81]  << ", " << logsum[82]  << ", " << logsum[83]
														<< ", " << logsum[84]  << ", " << logsum[85]  << ", " << logsum[86]  << ", " << logsum[87]  << ", " << logsum[88]  << ", " << logsum[89]  << ", " << logsum[90]  << ", " << logsum[91]  << ", " << logsum[92]  << ", " << logsum[93]  << ", " << logsum[94]  << ", " << logsum[95]  << ", " << logsum[96]  << ", " << logsum[97]
														<< ", " << logsum[98]  << ", " << logsum[99]  << ", " << logsum[100]  << ", " << logsum[101]  << ", " << logsum[102]  << ", " << logsum[103]  << ", " << logsum[104]  << ", " << logsum[105]  << ", " << logsum[106]  << ", " << logsum[107]  << ", " << logsum[108]  << ", " << logsum[109]  << ", " << logsum[110]
														<< ", " << logsum[111]  << ", " << logsum[112]  << ", " << logsum[113]  << ", " << logsum[114]  << ", " << logsum[115]  << ", " << logsum[116]  << ", " << logsum[117]  << ", " << logsum[118]  << ", " << logsum[119]  << ", " << logsum[120]  << ", " << logsum[121]  << ", " << logsum[122]  << ", " << logsum[123]
														<< ", " << logsum[124]  << ", " << logsum[125]  << ", " << logsum[126]  << ", " << logsum[127]  << ", " << logsum[128]  << ", " << logsum[129]  << ", " << logsum[130]  << ", " << logsum[131]  << ", " << logsum[132]  << ", " << logsum[133]  << ", " << logsum[134]  << ", " << logsum[135]  << ", " << logsum[136]
														<< ", " << logsum[137]  << ", " << logsum[138]  << ", " << logsum[139]  << ", " << logsum[140]  << ", " << logsum[141]  << ", " << logsum[142]  << ", " << logsum[143]  << ", " << logsum[144]  << ", " << logsum[145]  << ", " << logsum[146]  << ", " << logsum[147]  << ", " << logsum[148]  << ", " << logsum[149]
														<< ", " << logsum[150]  << ", " << logsum[151]  << ", " << logsum[152]  << ", " << logsum[153]  << ", " << logsum[154]  << ", " << logsum[155]  << ", " << logsum[156]  << ", " << logsum[157]  << ", " << logsum[158]  << ", " << logsum[159]  << ", " << logsum[160]  << ", " << logsum[161]  << ", " << logsum[162]
														<< ", " << logsum[163]  << ", " << logsum[164]  << ", " << logsum[165]  << ", " << logsum[166]  << ", " << logsum[167]  << ", " << logsum[168]  << ", " << logsum[169]  << ", " << logsum[170]  << ", " << logsum[171]  << ", " << logsum[172]  << ", " << logsum[173]  << ", " << logsum[174]  << ", " << logsum[175]
														<< ", " << logsum[176]  << ", " << logsum[177]  << ", " << logsum[178]  << ", " << logsum[179]  << ", " << logsum[180]  << ", " << logsum[181]  << ", " << logsum[182]  << ", " << logsum[183]  << ", " << logsum[184]  << ", " << logsum[185]  << ", " << logsum[186]  << ", " << logsum[187]  << ", " << logsum[188]
														<< ", " << logsum[189]  << ", " << logsum[190]  << ", " << logsum[191]  << ", " << logsum[192]  << ", " << logsum[193]  << ", " << logsum[194]  << ", " << logsum[195]  << ", " << logsum[196]  << ", " << logsum[197]  << ", " << logsum[198]  << ", " << logsum[199]  << ", " << logsum[200]  << ", " << logsum[201]
														<< ", " << logsum[202]  << ", " << logsum[203]  << ", " << logsum[204]  << ", " << logsum[205]  << ", " << logsum[206]  << ", " << logsum[207]  << ", " << logsum[208]  << ", " << logsum[209]  << ", " << logsum[210]  << ", " << logsum[211]  << ", " << logsum[212]  << ", " << logsum[213]  << ", " << logsum[214]
														<< ", " << logsum[215]  << ", " << logsum[216]  << ", " << logsum[217]  << ", " << logsum[218]  << ", " << logsum[219]  << ", " << logsum[220]  << ", " << logsum[221]  << ", " << logsum[222]  << ", " << logsum[223]  << ", " << logsum[224]  << ", " << logsum[225]  << ", " << logsum[226]  << ", " << logsum[227]
														<< ", " << logsum[228]  << ", " << logsum[229]  << ", " << logsum[230]  << ", " << logsum[231]  << ", " << logsum[232]  << ", " << logsum[233]  << ", " << logsum[234]  << ", " << logsum[235]  << ", " << logsum[236]  << ", " << logsum[237]  << ", " << logsum[238]  << ", " << logsum[239]  << ", " << logsum[240]
														<< ", " << logsum[241]  << ", " << logsum[242]  << ", " << logsum[243]  << ", " << logsum[244]  << ", " << logsum[245]  << ", " << logsum[246]  << ", " << logsum[247]  << ", " << logsum[248]  << ", " << logsum[249]  << ", " << logsum[250]  << ", " << logsum[251]  << ", " << logsum[252]  << ", " << logsum[253]
														<< ", " << logsum[254]  << ", " << logsum[255]  << ", " << logsum[256]  << ", " << logsum[257]  << ", " << logsum[258]  << ", " << logsum[259]  << ", " << logsum[260]  << ", " << logsum[261]  << ", " << logsum[262]  << ", " << logsum[263]  << ", " << logsum[264]  << ", " << logsum[265]  << ", " << logsum[266]
														<< ", " << logsum[267]  << ", " << logsum[268]  << ", " << logsum[269]  << ", " << logsum[270]  << ", " << logsum[271]  << ", " << logsum[272]  << ", " << logsum[273]  << ", " << logsum[274]  << ", " << logsum[275]  << ", " << logsum[276]  << ", " << logsum[277]  << ", " << logsum[278]  << ", " << logsum[279]
														<< ", " << logsum[280]  << ", " << logsum[281]  << ", " << logsum[282]  << ", " << logsum[283]  << ", " << logsum[284]  << ", " << logsum[285]  << ", " << logsum[286]  << ", " << logsum[287]  << ", " << logsum[288]  << ", " << logsum[289]  << ", " << logsum[290]  << ", " << logsum[291]  << ", " << logsum[292]
														<< ", " << logsum[293]  << ", " << logsum[294]  << ", " << logsum[295]  << ", " << logsum[296]  << ", " << logsum[297]  << ", " << logsum[298]  << ", " << logsum[299]  << ", " << logsum[300]  << ", " << logsum[301]  << ", " << logsum[302]  << ", " << logsum[303]  << ", " << logsum[304]  << ", " << logsum[305]
														<< ", " << logsum[306]  << ", " << logsum[307]  << ", " << logsum[308]  << ", " << logsum[309]  << ", " << logsum[310]  << ", " << logsum[311]  << ", " << logsum[312]  << ", " << logsum[313]  << ", " << logsum[314]  << ", " << logsum[315]  << ", " << logsum[316]  << ", " << logsum[317]  << ", " << logsum[318]
														<< ", " << logsum[319]  << ", " << logsum[320]  << ", " << logsum[321]  << ", " << logsum[322]  << ", " << logsum[323]  << ", " << logsum[324]  << ", " << logsum[325]  << ", " << logsum[326]  << ", " << logsum[327]  << ", " << logsum[328]  << ", " << logsum[329]  << ", " << logsum[330]  << ", " << logsum[331]
														<< ", " << logsum[332]  << ", " << logsum[333]  << ", " << logsum[334]  << ", " << logsum[335]  << ", " << logsum[336]  << ", " << logsum[337]  << ", " << logsum[338]  << ", " << logsum[339]  << ", " << logsum[340]  << ", " << logsum[341]  << ", " << logsum[342]  << ", " << logsum[343]  << ", " << logsum[344]
														<< ", " << logsum[345]  << ", " << logsum[346]  << ", " << logsum[347]  << ", " << logsum[348]  << ", " << logsum[349]  << ", " << logsum[350]  << ", " << logsum[351]  << ", " << logsum[352]  << ", " << logsum[353]  << ", " << logsum[354]  << ", " << logsum[355]  << ", " << logsum[356]  << ", " << logsum[357]
														<< ", " << logsum[358]  << ", " << logsum[359]  << ", " << logsum[360]  << ", " << logsum[361]  << ", " << logsum[362]  << ", " << logsum[363]  << ", " << logsum[364]  << ", " << logsum[365]  << ", " << logsum[366]  << ", " << logsum[367]  << ", " << logsum[368]  << ", " << logsum[369]  << ", " << logsum[370]
														<< ", " << logsum[371]  << ", " << logsum[372]  << ", " << logsum[373]  << ", " << logsum[374]  << ", " << logsum[375]  << ", " << logsum[376]  << ", " << logsum[377]  << ", " << logsum[378]  << ", " << logsum[379]  << ", " << logsum[380]  << ", " << logsum[381]  << ", " << logsum[382]  << ", " << logsum[383]
														<< ", " << logsum[384]  << ", " << logsum[385]  << ", " << logsum[386]  << ", " << logsum[387]  << ", " << logsum[388]  << ", " << logsum[389]  << ", " << logsum[390]  << ", " << logsum[391]  << ", " << logsum[392]  << ", " << logsum[393]  << ", " << logsum[394]  << ", " << logsum[395]  << ", " << logsum[396]
														<< ", " << logsum[397]  << ", " << logsum[398]  << ", " << logsum[399]  << ", " << logsum[400]  << ", " << logsum[401]  << ", " << logsum[402]  << ", " << logsum[403]  << ", " << logsum[404]  << ", " << logsum[405]  << ", " << logsum[406]  << ", " << logsum[407]  << ", " << logsum[408]  << ", " << logsum[409]
														<< ", " << logsum[410]  << ", " << logsum[411]  << ", " << logsum[412]  << ", " << logsum[413]  << ", " << logsum[414]  << ", " << logsum[415]  << ", " << logsum[416]  << ", " << logsum[417]  << ", " << logsum[418]  << ", " << logsum[419]  << ", " << logsum[420]  << ", " << logsum[421]  << ", " << logsum[422]
														<< ", " << logsum[423]  << ", " << logsum[424]  << ", " << logsum[425]  << ", " << logsum[426]  << ", " << logsum[427]  << ", " << logsum[428]  << ", " << logsum[429]  << ", " << logsum[430]  << ", " << logsum[431]  << ", " << logsum[432]  << ", " << logsum[433]  << ", " << logsum[434]  << ", " << logsum[435]
														<< ", " << logsum[436]  << ", " << logsum[437]  << ", " << logsum[438]  << ", " << logsum[439]  << ", " << logsum[440]  << ", " << logsum[441]  << ", " << logsum[442]  << ", " << logsum[443]  << ", " << logsum[444]  << ", " << logsum[445]  << ", " << logsum[446]  << ", " << logsum[447]  << ", " << logsum[448]
														<< ", " << logsum[449]  << ", " << logsum[450]  << ", " << logsum[451]  << ", " << logsum[452]  << ", " << logsum[453]  << ", " << logsum[454]  << ", " << logsum[455]  << ", " << logsum[456]  << ", " << logsum[457]  << ", " << logsum[458]  << ", " << logsum[459]  << ", " << logsum[460]  << ", " << logsum[461]
														<< ", " << logsum[462]  << ", " << logsum[463]  << ", " << logsum[464]  << ", " << logsum[465]  << ", " << logsum[466]  << ", " << logsum[467]  << ", " << logsum[468]  << ", " << logsum[469]  << ", " << logsum[470]  << ", " << logsum[471]  << ", " << logsum[472]  << ", " << logsum[473]  << ", " << logsum[474]
														<< ", " << logsum[475]  << ", " << logsum[476]  << ", " << logsum[477]  << ", " << logsum[478]  << ", " << logsum[479]  << ", " << logsum[480]  << ", " << logsum[481]  << ", " << logsum[482]  << ", " << logsum[483]  << ", " << logsum[484]  << ", " << logsum[485]  << ", " << logsum[486]  << ", " << logsum[487]
														<< ", " << logsum[488]  << ", " << logsum[489]  << ", " << logsum[490]  << ", " << logsum[491]  << ", " << logsum[492]  << ", " << logsum[493]  << ", " << logsum[494]  << ", " << logsum[495]  << ", " << logsum[496]  << ", " << logsum[497]  << ", " << logsum[498]  << ", " << logsum[499]  << ", " << logsum[500]
														<< ", " << logsum[501]  << ", " << logsum[502]  << ", " << logsum[503]  << ", " << logsum[504]  << ", " << logsum[505]  << ", " << logsum[506]  << ", " << logsum[507]  << ", " << logsum[508]  << ", " << logsum[509]  << ", " << logsum[510]  << ", " << logsum[511]  << ", " << logsum[512]  << ", " << logsum[513]
														<< ", " << logsum[514]  << ", " << logsum[515]  << ", " << logsum[516]  << ", " << logsum[517]  << ", " << logsum[518]  << ", " << logsum[519]  << ", " << logsum[520]  << ", " << logsum[521]  << ", " << logsum[522]  << ", " << logsum[523]  << ", " << logsum[524]  << ", " << logsum[525]  << ", " << logsum[526]
														<< ", " << logsum[527]  << ", " << logsum[528]  << ", " << logsum[529]  << ", " << logsum[530]  << ", " << logsum[531]  << ", " << logsum[532]  << ", " << logsum[533]  << ", " << logsum[534]  << ", " << logsum[535]  << ", " << logsum[536]  << ", " << logsum[537]  << ", " << logsum[538]  << ", " << logsum[539]
														<< ", " << logsum[540]  << ", " << logsum[541]  << ", " << logsum[542]  << ", " << logsum[543]  << ", " << logsum[544]  << ", " << logsum[545]  << ", " << logsum[546]  << ", " << logsum[547]  << ", " << logsum[548]  << ", " << logsum[549]  << ", " << logsum[550]  << ", " << logsum[551]  << ", " << logsum[552]
														<< ", " << logsum[553]  << ", " << logsum[554]  << ", " << logsum[555]  << ", " << logsum[556]  << ", " << logsum[557]  << ", " << logsum[558]  << ", " << logsum[559]  << ", " << logsum[560]  << ", " << logsum[561]  << ", " << logsum[562]  << ", " << logsum[563]  << ", " << logsum[564]  << ", " << logsum[565]
														<< ", " << logsum[566]  << ", " << logsum[567]  << ", " << logsum[568]  << ", " << logsum[569]  << ", " << logsum[570]  << ", " << logsum[571]  << ", " << logsum[572]  << ", " << logsum[573]  << ", " << logsum[574]  << ", " << logsum[575]  << ", " << logsum[576]  << ", " << logsum[577]  << ", " << logsum[578]
														<< ", " << logsum[579]  << ", " << logsum[580]  << ", " << logsum[581]  << ", " << logsum[582]  << ", " << logsum[583]  << ", " << logsum[584]  << ", " << logsum[585]  << ", " << logsum[586]  << ", " << logsum[587]  << ", " << logsum[588]  << ", " << logsum[589]  << ", " << logsum[590]  << ", " << logsum[591]
														<< ", " << logsum[592]  << ", " << logsum[593]  << ", " << logsum[594]  << ", " << logsum[595]  << ", " << logsum[596]  << ", " << logsum[597]  << ", " << logsum[598]  << ", " << logsum[599]  << ", " << logsum[600]  << ", " << logsum[601]  << ", " << logsum[602]  << ", " << logsum[603]  << ", " << logsum[604]
														<< ", " << logsum[605]  << ", " << logsum[606]  << ", " << logsum[607]  << ", " << logsum[608]  << ", " << logsum[609]  << ", " << logsum[610]  << ", " << logsum[611]  << ", " << logsum[612]  << ", " << logsum[613]  << ", " << logsum[614]  << ", " << logsum[615]  << ", " << logsum[616]  << ", " << logsum[617]
														<< ", " << logsum[618]  << ", " << logsum[619]  << ", " << logsum[620]  << ", " << logsum[621]  << ", " << logsum[622]  << ", " << logsum[623]  << ", " << logsum[624]  << ", " << logsum[625]  << ", " << logsum[626]  << ", " << logsum[627]  << ", " << logsum[628]  << ", " << logsum[629]  << ", " << logsum[630]
														<< ", " << logsum[631]  << ", " << logsum[632]  << ", " << logsum[633]  << ", " << logsum[634]  << ", " << logsum[635]  << ", " << logsum[636]  << ", " << logsum[637]  << ", " << logsum[638]  << ", " << logsum[639]  << ", " << logsum[640]  << ", " << logsum[641]  << ", " << logsum[642]  << ", " << logsum[643]
														<< ", " << logsum[644]  << ", " << logsum[645]  << ", " << logsum[646]  << ", " << logsum[647]  << ", " << logsum[648]  << ", " << logsum[649]  << ", " << logsum[650]  << ", " << logsum[651]  << ", " << logsum[652]  << ", " << logsum[653]  << ", " << logsum[654]  << ", " << logsum[655]  << ", " << logsum[656]
														<< ", " << logsum[657]  << ", " << logsum[658]  << ", " << logsum[659]  << ", " << logsum[660]  << ", " << logsum[661]  << ", " << logsum[662]  << ", " << logsum[663]  << ", " << logsum[664]  << ", " << logsum[665]  << ", " << logsum[666]  << ", " << logsum[667]  << ", " << logsum[668]  << ", " << logsum[669]
														<< ", " << logsum[670]  << ", " << logsum[671]  << ", " << logsum[672]  << ", " << logsum[673]  << ", " << logsum[674]  << ", " << logsum[675]  << ", " << logsum[676]  << ", " << logsum[677]  << ", " << logsum[678]  << ", " << logsum[679]  << ", " << logsum[680]  << ", " << logsum[681]  << ", " << logsum[682]
														<< ", " << logsum[683]  << ", " << logsum[684]  << ", " << logsum[685]  << ", " << logsum[686]  << ", " << logsum[687]  << ", " << logsum[688]  << ", " << logsum[689]  << ", " << logsum[690]  << ", " << logsum[691]  << ", " << logsum[692]  << ", " << logsum[693]  << ", " << logsum[694]  << ", " << logsum[695]
														<< ", " << logsum[696]  << ", " << logsum[697]  << ", " << logsum[698]  << ", " << logsum[699]  << ", " << logsum[700]  << ", " << logsum[701]  << ", " << logsum[702]  << ", " << logsum[703]  << ", " << logsum[704]  << ", " << logsum[705]  << ", " << logsum[706]  << ", " << logsum[707]  << ", " << logsum[708]
														<< ", " << logsum[709]  << ", " << logsum[710]  << ", " << logsum[711]  << ", " << logsum[712]  << ", " << logsum[713]  << ", " << logsum[714]  << ", " << logsum[715]  << ", " << logsum[716]  << ", " << logsum[717]  << ", " << logsum[718]  << ", " << logsum[719]  << ", " << logsum[720]  << ", " << logsum[721]
														<< ", " << logsum[722]  << ", " << logsum[723]  << ", " << logsum[724]  << ", " << logsum[725]  << ", " << logsum[726]  << ", " << logsum[727]  << ", " << logsum[728]  << ", " << logsum[729]  << ", " << logsum[730]  << ", " << logsum[731]  << ", " << logsum[732]  << ", " << logsum[733]  << ", " << logsum[734]
														<< ", " << logsum[735]  << ", " << logsum[736]  << ", " << logsum[737]  << ", " << logsum[738]  << ", " << logsum[739]  << ", " << logsum[740]  << ", " << logsum[741]  << ", " << logsum[742]  << ", " << logsum[743]  << ", " << logsum[744]  << ", " << logsum[745]  << ", " << logsum[746]  << ", " << logsum[747]
														<< ", " << logsum[748]  << ", " << logsum[749]  << ", " << logsum[750]  << ", " << logsum[751]  << ", " << logsum[752]  << ", " << logsum[753]  << ", " << logsum[754]  << ", " << logsum[755]  << ", " << logsum[756]  << ", " << logsum[757]  << ", " << logsum[758]  << ", " << logsum[759]  << ", " << logsum[760]
														<< ", " << logsum[761]  << ", " << logsum[762]  << ", " << logsum[763]  << ", " << logsum[764]  << ", " << logsum[765]  << ", " << logsum[766]  << ", " << logsum[767]  << ", " << logsum[768]  << ", " << logsum[769]  << ", " << logsum[770]  << ", " << logsum[771]  << ", " << logsum[772]  << ", " << logsum[773]
														<< ", " << logsum[774]  << ", " << logsum[775]  << ", " << logsum[776]  << ", " << logsum[777]  << ", " << logsum[778]  << ", " << logsum[779]  << ", " << logsum[780]  << ", " << logsum[781]  << ", " << logsum[782]  << ", " << logsum[783]  << ", " << logsum[784]  << ", " << logsum[785]  << ", " << logsum[786]
														<< ", " << logsum[787]  << ", " << logsum[788]  << ", " << logsum[789]  << ", " << logsum[790]  << ", " << logsum[791]  << ", " << logsum[792]  << ", " << logsum[793]  << ", " << logsum[794]  << ", " << logsum[795]  << ", " << logsum[796]  << ", " << logsum[797]  << ", " << logsum[798]  << ", " << logsum[799]
														<< ", " << logsum[800]  << ", " << logsum[801]  << ", " << logsum[802]  << ", " << logsum[803]  << ", " << logsum[804]  << ", " << logsum[805]  << ", " << logsum[806]  << ", " << logsum[807]  << ", " << logsum[808]  << ", " << logsum[809]  << ", " << logsum[810]  << ", " << logsum[811]  << ", " << logsum[812]
														<< ", " << logsum[813]  << ", " << logsum[814]  << ", " << logsum[815]  << ", " << logsum[816]  << ", " << logsum[817]  << ", " << logsum[818]  << ", " << logsum[819]  << ", " << logsum[820]  << ", " << logsum[821]  << ", " << logsum[822]  << ", " << logsum[823]  << ", " << logsum[824]  << ", " << logsum[825]
														<< ", " << logsum[826]  << ", " << logsum[827]  << ", " << logsum[828]  << ", " << logsum[829]  << ", " << logsum[830]  << ", " << logsum[831]  << ", " << logsum[832]  << ", " << logsum[833]  << ", " << logsum[834]  << ", " << logsum[835]  << ", " << logsum[836]  << ", " << logsum[837]  << ", " << logsum[838]
														<< ", " << logsum[839]  << ", " << logsum[840]  << ", " << logsum[841]  << ", " << logsum[842]  << ", " << logsum[843]  << ", " << logsum[844]  << ", " << logsum[845]  << ", " << logsum[846]  << ", " << logsum[847]  << ", " << logsum[848]  << ", " << logsum[849]  << ", " << logsum[850]  << ", " << logsum[851]
														<< ", " << logsum[852]  << ", " << logsum[853]  << ", " << logsum[854]  << ", " << logsum[855]  << ", " << logsum[856]  << ", " << logsum[857]  << ", " << logsum[858]  << ", " << logsum[859]  << ", " << logsum[860]  << ", " << logsum[861]  << ", " << logsum[862]  << ", " << logsum[863]  << ", " << logsum[864]
														<< ", " << logsum[865]  << ", " << logsum[866]  << ", " << logsum[867]  << ", " << logsum[868]  << ", " << logsum[869]  << ", " << logsum[870]  << ", " << logsum[871]  << ", " << logsum[872]  << ", " << logsum[873]  << ", " << logsum[874]  << ", " << logsum[875]  << ", " << logsum[876]  << ", " << logsum[877]
														<< ", " << logsum[878]  << ", " << logsum[879]  << ", " << logsum[880]  << ", " << logsum[881]  << ", " << logsum[882]  << ", " << logsum[883]  << ", " << logsum[884]  << ", " << logsum[885]  << ", " << logsum[886]  << ", " << logsum[887]  << ", " << logsum[888]  << ", " << logsum[889]  << ", " << logsum[890]
														<< ", " << logsum[891]  << ", " << logsum[892]  << ", " << logsum[893]  << ", " << logsum[894]  << ", " << logsum[895]  << ", " << logsum[896]  << ", " << logsum[897]  << ", " << logsum[898]  << ", " << logsum[899]  << ", " << logsum[900]  << ", " << logsum[901]  << ", " << logsum[902]  << ", " << logsum[903]
														<< ", " << logsum[904]  << ", " << logsum[905]  << ", " << logsum[906]  << ", " << logsum[907]  << ", " << logsum[908]  << ", " << logsum[909]  << ", " << logsum[910]  << ", " << logsum[911]  << ", " << logsum[912]  << ", " << logsum[913]  << ", " << logsum[914]  << ", " << logsum[915]  << ", " << logsum[916]
														<< ", " << logsum[917]  << ", " << logsum[918]  << ", " << logsum[919]  << ", " << logsum[920]  << ", " << logsum[921]  << ", " << logsum[922]  << ", " << logsum[923]  << ", " << logsum[924]  << ", " << logsum[925]  << ", " << logsum[926]  << ", " << logsum[927]  << ", " << logsum[928]  << ", " << logsum[929]
														<< ", " << logsum[930]  << ", " << logsum[931]  << ", " << logsum[932]  << ", " << logsum[933]  << ", " << logsum[934]  << ", " << logsum[935]  << ", " << logsum[936]  << ", " << logsum[937]  << ", " << logsum[938]  << ", " << logsum[939]  << ", " << logsum[940]  << ", " << logsum[941]  << ", " << logsum[942]
														<< ", " << logsum[943]  << ", " << logsum[944]  << ", " << logsum[945]  << ", " << logsum[946]  << ", " << logsum[947]  << ", " << logsum[948]  << ", " << logsum[949]  << ", " << logsum[950]  << ", " << logsum[951]  << ", " << logsum[952]  << ", " << logsum[953]  << ", " << logsum[954]  << ", " << logsum[955]
														<< ", " << logsum[956]  << ", " << logsum[957]  << ", " << logsum[958]  << ", " << logsum[959]  << ", " << logsum[960]  << ", " << logsum[961]  << ", " << logsum[962]  << ", " << logsum[963]  << ", " << logsum[964]  << ", " << logsum[965]  << ", " << logsum[966]  << ", " << logsum[967]  << ", " << logsum[968]
														<< ", " << logsum[969]  << ", " << logsum[970]  << ", " << logsum[971]  << ", " << logsum[972]  << ", " << logsum[973]  << ", " << logsum[974]  << ", " << logsum[975]  << ", " << logsum[976]  << ", " << logsum[977]  << ", " << logsum[978]  << ", " << logsum[979]  << ", " << logsum[980]  << ", " << logsum[981]
														<< ", " << logsum[982]  << ", " << logsum[983]  << ", " << logsum[984]  << ", " << logsum[985]  << ", " << logsum[986]  << ", " << logsum[987]  << ", " << logsum[988]  << ", " << logsum[989]  << ", " << logsum[990]  << ", " << logsum[991]  << ", " << logsum[992]  << ", " << logsum[993]  << ", " << logsum[994]
														<< ", " << logsum[995]  << ", " << logsum[996]  << ", " << logsum[997]  << ", " << logsum[998]  << ", " << logsum[999]  << ", " << logsum[1000]  << ", " << logsum[1001]  << ", " << logsum[1002]  << ", " << logsum[1003]  << ", " << logsum[1004]  << ", " << logsum[1005]  << ", " << logsum[1006]
														<< ", " << logsum[1007]  << ", " << logsum[1008]  << ", " << logsum[1009]  << ", " << logsum[1010]  << ", " << logsum[1011]  << ", " << logsum[1012]  << ", " << logsum[1013]  << ", " << logsum[1014]  << ", " << logsum[1015]  << ", " << logsum[1016]  << ", " << logsum[1017]  << ", " << logsum[1018]
														<< ", " << logsum[1019]  << ", " << logsum[1020]  << ", " << logsum[1021]  << ", " << logsum[1022]  << ", " << logsum[1023]  << ", " << logsum[1024]  << ", " << logsum[1025]  << ", " << logsum[1026]  << ", " << logsum[1027]  << ", " << logsum[1028]  << ", " << logsum[1029]  << ", " << logsum[1030]
														<< ", " << logsum[1031]  << ", " << logsum[1032]  << ", " << logsum[1033]  << ", " << logsum[1034]  << ", " << logsum[1035]  << ", " << logsum[1036]  << ", " << logsum[1037]  << ", " << logsum[1038]  << ", " << logsum[1039]  << ", " << logsum[1040]  << ", " << logsum[1041]  << ", " << logsum[1042]
														<< ", " << logsum[1043]  << ", " << logsum[1044]  << ", " << logsum[1045]  << ", " << logsum[1046]  << ", " << logsum[1047]  << ", " << logsum[1048]  << ", " << logsum[1049]  << ", " << logsum[1050]  << ", " << logsum[1051]  << ", " << logsum[1052]  << ", " << logsum[1053]  << ", " << logsum[1054]
														<< ", " << logsum[1055]  << ", " << logsum[1056]  << ", " << logsum[1057]  << ", " << logsum[1058]  << ", " << logsum[1059]  << ", " << logsum[1060]  << ", " << logsum[1061]  << ", " << logsum[1062]  << ", " << logsum[1063]  << ", " << logsum[1064]  << ", " << logsum[1065]  << ", " << logsum[1066]
														<< ", " << logsum[1067]  << ", " << logsum[1068]  << ", " << logsum[1069]  << ", " << logsum[1070]  << ", " << logsum[1071]  << ", " << logsum[1072]  << ", " << logsum[1073]  << ", " << logsum[1074]  << ", " << logsum[1075]  << ", " << logsum[1076]  << ", " << logsum[1077]  << ", " << logsum[1078]
														<< ", " << logsum[1079]  << ", " << logsum[1080]  << ", " << logsum[1081]  << ", " << logsum[1082]  << ", " << logsum[1083]  << ", " << logsum[1084]  << ", " << logsum[1085]  << ", " << logsum[1086]  << ", " << logsum[1087]  << ", " << logsum[1088]  << ", " << logsum[1089]  << ", " << logsum[1090]
														<< ", " << logsum[1091]  << ", " << logsum[1092]  << ", " << logsum[1093]  << ", " << logsum[1094]  << ", " << logsum[1095]  << ", " << logsum[1096]  << ", " << logsum[1097]  << ", " << logsum[1098]  << ", " << logsum[1099]  << ", " << logsum[1100]  << ", " << logsum[1101]  << ", " << logsum[1102]
														<< ", " << logsum[1103]  << ", " << logsum[1104]  << ", " << logsum[1105]  << ", " << logsum[1106]  << ", " << logsum[1107]  << ", " << logsum[1108]  << ", " << logsum[1109]  << ", " << logsum[1110]  << ", " << logsum[1111]  << ", " << logsum[1112]  << ", " << logsum[1113]  << ", " << logsum[1114]
														<< ", " << logsum[1115]  << ", " << logsum[1116]  << ", " << logsum[1117]  << ", " << logsum[1118]  << ", " << logsum[1119]  << ", " << logsum[1120]  << ", " << logsum[1121]  << ", " << logsum[1122]  << ", " << logsum[1123]  << ", " << logsum[1124]  << ", " << logsum[1125]  << ", " << logsum[1126]
														<< ", " << logsum[1127]  << ", " << logsum[1128]  << ", " << logsum[1129]  << ", " << logsum[1130]  << ", " << logsum[1131]  << ", " << logsum[1132]  << ", " << logsum[1133]  << ", " << logsum[1134]  << ", " << logsum[1135]  << ", " << logsum[1136]  << ", " << logsum[1137]  << ", " << logsum[1138]
														<< ", " << logsum[1139]  << ", " << logsum[1140]  << ", " << logsum[1141]  << ", " << logsum[1142]  << ", " << logsum[1143]  << ", " << logsum[1144]  << ", " << logsum[1145]  << ", " << logsum[1146]  << ", " << logsum[1147]  << ", " << logsum[1148]  << ", " << logsum[1149]  << ", " << logsum[1150]
														<< ", " << logsum[1151]  << ", " << logsum[1152]  << ", " << logsum[1153]  << ", " << logsum[1154]  << ", " << logsum[1155]  << ", " << logsum[1156]  << ", " << logsum[1157]  << ", " << logsum[1158]  << ", " << logsum[1159]  << ", " << logsum[1160]  << ", " << logsum[1161]  << ", " << logsum[1162]
														<< ", " << logsum[1163]  << ", " << logsum[1164]  << ", " << logsum[1165]  << ", " << logsum[1166]  << ", " << logsum[1167]  << ", " << logsum[1168]  << ", " << logsum[1169]  << ", " << logsum[1170]  << ", " << logsum[1171]  << ", " << logsum[1172]  << ", " << logsum[1173]  << ", " << logsum[1174]
														<< ", " << logsum[1175]  << ", " << logsum[1176]  << ", " << logsum[1177]  << ", " << logsum[1178]  << ", " << logsum[1179]  << ", " << logsum[1180]  << ", " << logsum[1181]  << ", " << logsum[1182]  << ", " << logsum[1183]  << ", " << logsum[1184]  << ", " << logsum[1185]  << ", " << logsum[1186]
														<< ", " << logsum[1187]  << ", " << logsum[1188]  << ", " << logsum[1189]  << ", " << logsum[1190]  << ", " << logsum[1191]  << ", " << logsum[1192]  << ", " << logsum[1193]  << ", " << logsum[1194]  << ", " << logsum[1195]  << ", " << logsum[1196]  << ", " << logsum[1197]  << ", " << logsum[1198]
														<< ", " << logsum[1199]  << ", " << logsum[1200]  << ", " << logsum[1201]  << ", " << logsum[1202]  << ", " << logsum[1203]  << ", " << logsum[1204]  << ", " << logsum[1205]  << ", " << logsum[1206]  << ", " << logsum[1207]  << ", " << logsum[1208]  << ", " << logsum[1209]  << ", " << logsum[1210]
														<< ", " << logsum[1211]  << ", " << logsum[1212]  << ", " << logsum[1213]  << ", " << logsum[1214]  << ", " << logsum[1215]  << ", " << logsum[1216]  << ", " << logsum[1217]  << ", " << logsum[1218]  << ", " << logsum[1219]  << ", " << logsum[1220]  << ", " << logsum[1221]  << ", " << logsum[1222]
														<< ", " << logsum[1223]  << ", " << logsum[1224]  << ", " << logsum[1225]  << ", " << logsum[1226]  << ", " << logsum[1227]  << ", " << logsum[1228]  << ", " << logsum[1229]  << ", " << logsum[1230]  << ", " << logsum[1231]  << ", " << logsum[1232]  << ", " << logsum[1233]  << ", " << logsum[1234]
														<< ", " << logsum[1235]  << ", " << logsum[1236]  << ", " << logsum[1237]  << ", " << logsum[1238]  << ", " << logsum[1239]  << ", " << logsum[1240]  << ", " << logsum[1241]  << ", " << logsum[1242]  << ", " << logsum[1243]  << ", " << logsum[1244]  << ", " << logsum[1245]  << ", " << logsum[1246]
														<< ", " << logsum[1247]  << ", " << logsum[1248]  << ", " << logsum[1249]  << ", " << logsum[1250]  << ", " << logsum[1251]  << ", " << logsum[1252]  << ", " << logsum[1253]  << ", " << logsum[1254]  << ", " << logsum[1255]  << ", " << logsum[1256]  << ", " << logsum[1257]  << ", " << logsum[1258]
														<< ", " << logsum[1259]  << ", " << logsum[1260]  << ", " << logsum[1261]  << ", " << logsum[1262]  << ", " << logsum[1263]  << ", " << logsum[1264]  << ", " << logsum[1265]  << ", " << logsum[1266]  << std::endl );
	}
}

void HM_Model::unitsFiltering()
{
	int numOfHDB = 0;
	int numOfCondo = 0;

	for (UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		int unitType = (*it)->getUnitType();

		//1:Studio HDB 2:2room HDB 3:3room HDB 4:4room HDB" 5:5room HDB 6:EC HDB
		if( unitType < LS70_APT )
		{
			numOfHDB++;
		}
		else
		if( unitType < LG379_RC )
		{
			numOfCondo++;
		}
	}


	//we need to filter out 10% of unoccupied apartments, condos and 5% of HDBs.
	int targetNumOfHDB   = 0.05 * numOfHDB;
	int targetNumOfCondo = 0.10 * numOfCondo;

	PrintOutV( "[Prefilter] Total number of HDB: " << numOfHDB  << std::endl );
	PrintOutV( "[Prefilter] Total number of Condos: " << numOfCondo << std::endl );
	PrintOutV( "Total units " << units.size() << std::endl );

	srand(time(0));
	for( int n = 0;  n < targetNumOfHDB; )
	{
		int random =  (double)rand() / RAND_MAX * units.size();

		if( units[random]->getUnitType() < LS70_APT )
		{
			units.erase( units.begin() + random );
			n++;
		}
	}

	for( int n = 0;  n < targetNumOfCondo; )
	{
		int random =  (double)rand() / RAND_MAX * units.size();

		if( units[random]->getUnitType() >= LS70_APT && units[random]->getUnitType() < LG379_RC )
		{
			units.erase( units.begin() + random );
			n++;
		}
	}

	PrintOutV( "[Postfilter] Total number of HDB: " << numOfHDB - targetNumOfHDB  << std::endl );
	PrintOutV( "[Postfilter] Total number of Condos: " << numOfCondo - targetNumOfCondo << std::endl );
	PrintOutV( "Total units " << units.size() << std::endl );
}

void HM_Model::update(int day)
{
	//PrintOut("HM_Model update" << std::endl);
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	for(UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		//this unit is a vacancy
		if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
		{
			//If a unit is off the market and unoccupied, we should put it back on the market after its timeOffMarket value is exceeded.
			if( (*it)->getbiddingMarketEntryDay() + (*it)->getTimeOnMarket() + (*it)->getTimeOffMarket() < day )
			{
				//PrintOutV("A unit is being re-awakened" << std::endl);
				(*it)->setbiddingMarketEntryDay(day + 1);
				(*it)->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
			}
		}
	}
}


void HM_Model::hdbEligibilityTest(int index)
{
	int familyType = 0;

	household_stats.ResetLocal();

	for (int n = 0; n < households[index]->getIndividuals().size(); n++)
	{
		const Individual* hhIndividual = getIndividualById(	households[index]->getIndividuals()[n]);

		time_t now = time(0);
		tm ltm = *(localtime(&now));
		std::tm birthday = hhIndividual->getDateOfBirth();


		boost::gregorian::date date1(birthday.tm_year + 1900, birthday.tm_mon + 1, birthday.tm_mday);
		boost::gregorian::date date2(ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday);

		int years = (date2 - date1).days() / YEAR;

		if (years < MINOR)
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleChild++;
				household_stats.maleChild_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleChild++;
				household_stats.femaleChild_global++;
			}
		}
		else if (years < YOUNG_ADULT)
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleAdultYoung++;
				household_stats.maleAdultYoung_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleAdultYoung++;
				household_stats.femaleAdultYoung_global++;
			}
		}
		else if (years < MIDDLE_AGED_ADULT)
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleAdultMiddleAged++;
				household_stats.maleAdultMiddleAged_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleAdultMiddleAged++;
				household_stats.femaleAdultMiddleAged_global++;
			}
		}
		else
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleAdultElderly++;
				household_stats.maleAdultElderly_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleAdultElderly++;
				household_stats.femaleAdultElderly_global++;
			}
		}

		if (years >= MINOR && hhIndividual->getResidentialStatusId() == RESIDENT)
		{
			household_stats.adultSingaporean++;
			household_stats.adultSingaporean_global++;
		}
	}

	if (((household_stats.maleAdultYoung == 1 	   && household_stats.femaleAdultYoung == 1)		&& ((household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))) ||
		((household_stats.maleAdultMiddleAged == 1 && household_stats.femaleAdultMiddleAged == 1)	&& (household_stats.maleAdultElderly > 0 	 || household_stats.femaleAdultElderly > 0))   ||
		((household_stats.maleAdultYoung == 1 	   && household_stats.femaleAdultYoung == 1)		&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0) || (household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))) ||
		((household_stats.maleAdultMiddleAged == 1 || household_stats.femaleAdultMiddleAged == 1) 	&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))))
	{
		familyType = Household::MULTIGENERATION;
		household_stats.multigeneration++;
	}
	else
	if((household_stats.maleAdultYoung > 0 && household_stats.femaleAdultYoung > 0	&& (household_stats.maleChild > 0 || household_stats.femaleChild > 0)) ||
	   (household_stats.maleAdultMiddleAged > 0 && household_stats.femaleAdultMiddleAged > 0  && (household_stats.maleChild > 0 || household_stats.femaleChild > 0 || household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0)))
	{
		familyType = Household::COUPLEANDCHILD;
		household_stats.coupleAndChild++;
	}
	else
	if((household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0) &&
	  ((household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0)))
	{
		familyType = Household::SIBLINGSANDPARENTS;
		household_stats.siblingsAndParents++;
	}
	else
	if (household_stats.maleAdultYoung == 1 && household_stats.femaleAdultYoung == 1)
	{
		familyType = Household::ENGAGEDCOUPLE;
		household_stats.engagedCouple++;
	}
	else
	if ((household_stats.maleAdultYoung > 1 || household_stats.femaleAdultYoung > 1) ||
		(household_stats.maleAdultMiddleAged > 1 || household_stats.femaleAdultMiddleAged > 1))
	{
		familyType = Household::ORPHANSIBLINGS;
		household_stats.orphanSiblings++;
	}
	else
	if(((household_stats.maleAdultYoung == 1 || household_stats.femaleAdultYoung == 1)		&& (household_stats.maleChild > 0 || household_stats.femaleChild > 0))			||
	   ((household_stats.maleAdultMiddleAged == 1 || household_stats.femaleAdultMiddleAged == 1)	&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0)	||
	    (household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0))))
	{
		familyType = Household::SINGLEPARENT;
		household_stats.singleParent++;
	}


	households[index]->setFamilyType(familyType);

	if( household_stats.adultSingaporean > 0 )
	{
		bool familyTypeGeneral = false;

		if( familyType == Household::COUPLEANDCHILD ||
			familyType == Household::SIBLINGSANDPARENTS ||
			familyType == Household::SINGLEPARENT  ||
			familyType == Household::ENGAGEDCOUPLE ||
			familyType == Household::ORPHANSIBLINGS )
		{
			familyTypeGeneral = true;
		}

		if (households[index]->getIncome() < TWOBEDROOM	&& familyTypeGeneral == true)
		{
			households[index]->setTwoRoomHdbEligibility(true);
		}
		else if (households[index]->getIncome() < THREEBEDROOM && familyTypeGeneral == true)
		{
			households[index]->setTwoRoomHdbEligibility(true);
			households[index]->setThreeRoomHdbEligibility(true);
		}
		else if (households[index]->getIncome() < THREEBEDROOMMATURE && familyTypeGeneral == true && familyType == Household::MULTIGENERATION)
		{
			households[index]->setTwoRoomHdbEligibility(true);
			households[index]->setThreeRoomHdbEligibility(true);
			households[index]->setFourRoomHdbEligibility(true);
		}
	}
}

void HM_Model::addNewBids(boost::shared_ptr<Bid> &newBid)
{
	DBLock.lock();
	newBids.push_back(newBid);
	DBLock.unlock();
}

void HM_Model::addUnitSales(boost::shared_ptr<UnitSale> &unitSale)
{
	DBLock.lock();
	unitSales.push_back(unitSale);
	DBLock.unlock();
}

std::vector<boost::shared_ptr<UnitSale> > HM_Model::getUnitSales()
{
	return this->unitSales;
}

std::vector<boost::shared_ptr<Bid> > HM_Model::getNewBids()
{
	return this->newBids;
}

BigSerial HM_Model::getBidId()
{
	{
		boost::mutex::scoped_lock lock(idLock);

		return ++bidId;
	}
}

BigSerial HM_Model::getUnitSaleId()
{
	{
		boost::mutex::scoped_lock lock(idLock);

		return ++unitSaleId;
	}
}

void HM_Model::addHouseholdsTo_OPSchema(boost::shared_ptr<Household> &houseHold)
{
	DBLock.lock();
	hhVector.push_back(houseHold);
	DBLock.unlock();
}

std::vector<boost::shared_ptr<Household> > HM_Model::getHouseholdsWithBids()
{
	return this->hhVector;
}

void HM_Model::addVehicleOwnershipChanges(boost::shared_ptr<VehicleOwnershipChanges> &vehicleOwnershipChange)
{
	DBLock.lock();
	vehicleOwnershipChangesVector.push_back(vehicleOwnershipChange);
	DBLock.unlock();
}

std::vector<boost::shared_ptr<VehicleOwnershipChanges> > HM_Model::getVehicleOwnershipChanges()
{
	return vehicleOwnershipChangesVector;
}


IndvidualVehicleOwnershipLogsum* HM_Model::getIndvidualVehicleOwnershipLogsumsByHHId(BigSerial householdId) const
{
	IndvidualVehicleOwnershipLogsumMap::const_iterator itr = IndvidualVehicleOwnershipLogsumById.find(householdId);

	if (itr != IndvidualVehicleOwnershipLogsumById.end())
	{
		return itr->second;
	}
	return nullptr;
}

std::vector<ScreeningCostTime*>  HM_Model::getScreeningCostTime()
{
	return  screeningCostTime;
}

ScreeningCostTime* HM_Model::getScreeningCostTimeInst(std::string key)
{
	ScreeningCostTimeSuperMap::const_iterator itr = screeningCostTimeSuperMap.find(key);

	int size = screeningCostTimeSuperMap.size();

	if (itr != screeningCostTimeSuperMap.end())
	{
		ScreeningCostTimeMap::const_iterator itr2 = screeningCostTimeById.find((*itr).second);

		if (itr2 != screeningCostTimeById.end())
			return (*itr2).second;
	}

	return nullptr;
}


HM_Model::IndvidualVehicleOwnershipLogsumList HM_Model::getIndvidualVehicleOwnershipLogsums() const
{
	return IndvidualVehicleOwnershipLogsums;
}

std::vector<AccessibilityFixedPzid*> HM_Model::getAccessibilityFixedPzid()
{
	return accessibilityFixedPzid;

}

std::vector<Bid*> HM_Model::getResumptionBids()
{
	return this->resumptionBids;
}

Household* HM_Model::getResumptionHouseholdById(BigSerial id) const
{
	HouseholdMap::const_iterator itr = resumptionHHById.find(id);

	if (itr != resumptionHHById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

VehicleOwnershipChanges* HM_Model::getVehicleOwnershipChangesByHHId(BigSerial houseHoldId) const
{
	VehicleOwnershipChangesMap::const_iterator itr = vehicleOwnershipChangesById.find(houseHoldId);

	if (itr != vehicleOwnershipChangesById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

void HM_Model::setLastStoppedDay(int stopDay)
{
	lastStoppedDay = stopDay;
}

void HM_Model::stopImpl()
{
	deleteAll(stats);
	clear_delete_vector(households);
	clear_delete_vector(units);
	householdsById.clear();
	unitsById.clear();
}
