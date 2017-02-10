//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   PrintLog.hpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on Feb 23, 2016, 9:52 AM
 */

#include "core/AgentsLookup.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>

namespace sim_mob
{
	namespace long_term
	{

	inline void printHouseholdEligibility(Household *household)
	{
		static bool printHeader = true;

		 if(printHeader)
		 {
			printHeader = false;
			AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLD_STATISTICS,"householdId, TwoRoomHdbEligibility, ThreeRoomHdbEligibility, FourRoomHdbEligibility, FamilyType, hhSize, adultSingaporean, coupleAndChild, engagedCouple, femaleAdultElderly, femaleAdultMiddleAged, femaleAdultYoung, femaleChild, maleAdultElderly, maleAdultMiddleAged, maleAdultYoung, maleChild, multigeneration, orphanSiblings, siblingsAndParents, singleParent");
		 }

	     boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, "
	    		 	 	 	 	 	 	 	"%11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21% ")
	     	 	 	 	 	 	 	 	 	 	 	 	 	 % household->getId()
															 % household->getTwoRoomHdbEligibility()
															 % household->getThreeRoomHdbEligibility()
															 % household->getFourRoomHdbEligibility()
															 % household->getFamilyType()
															 % household->getSize()
															 % household->getHouseholdStats().adultSingaporean
															 % household->getHouseholdStats().coupleAndChild
															 % household->getHouseholdStats().engagedCouple
															 % household->getHouseholdStats().femaleAdultElderly
															 % household->getHouseholdStats().femaleAdultMiddleAged
															 % household->getHouseholdStats().femaleAdultYoung
															 % household->getHouseholdStats().femaleChild
															 % household->getHouseholdStats().maleAdultElderly
															 % household->getHouseholdStats().maleAdultMiddleAged
															 % household->getHouseholdStats().maleAdultYoung
															 % household->getHouseholdStats().maleChild
															 % household->getHouseholdStats().multigeneration
															 % household->getHouseholdStats().orphanSiblings
															 % household->getHouseholdStats().siblingsAndParents
															 % household->getHouseholdStats().singleParent;

	     AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLD_STATISTICS, fmtr.str());
	}

	 /**
	  * Print the current expectation on the unit.
	  * @param the current day
	  * @param the day on which the bid was made
	  * @param the unit id
	  * @param agent to received the bid
	  * @param struct containing the hedonic, asking and target price.
	  *
	  */

	 inline void printExpectation(int day, int dayToApply, BigSerial unitId, BigSerial agentId, const ExpectationEntry& exp)
	 {
		 //static boost::mutex mtx_expectation;

		 static bool printHeader = true;

		 if(printHeader)
		 {
			//boost::mutex::scoped_lock(mtx_expectation);
			printHeader = false;
			AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::EXPECTATIONS,"bid_timestamp, day_to_apply, seller_id, unit_id, hedonic_price, asking_price, target_price");

		 }

	     boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%, %7%") 	% day
																					% dayToApply
																					% agentId
																					% unitId
																					% exp.hedonicPrice
																					% exp.askingPrice
																					% exp.targetPrice;

	     AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::EXPECTATIONS, fmtr.str());
	     //PrintOut(fmtr.str() << endl);
	 }


	inline void printAwakening(int day, Household *household)
	{
        static bool printHeader = true;

        if(printHeader)
        {
        	printHeader = false;
        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HH_AWAKENING,"awakening_day, householdId, TimeOnMarket");
        }

		//day household_id timeOnMarket
		boost::format fmtr = boost::format("%1%, %2%, %3%") % (day + 1) % household->getId() % household->getTimeOnMarket();
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HH_AWAKENING, fmtr.str());

	}


	inline void writeTaxiAvailabilityToFile(BigSerial hhId,double probabilityTaxiAccess,double randomNum)
	{
        static bool printHeader = true;

        if(printHeader)
        {
        	printHeader = false;
        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_TAXI_AVAILABILITY,"household_id, probability_taxiAccess, randonNum");
        }

		boost::format fmtr = boost::format("%1%, %2%, %3%") % hhId % probabilityTaxiAccess % randomNum;
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_TAXI_AVAILABILITY,fmtr.str());

	}

	inline void printTazLevelLogsum(int taz, double logsum)
	{
        static bool printHeader = true;

        if(printHeader)
        {
        	printHeader = false;
        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_TAZ_LEVEL_LOGSUM,"taz, logsum");
        }

		boost::format fmtr = boost::format("%1%, %2%") 	% taz % logsum;

		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_TAZ_LEVEL_LOGSUM, fmtr.str());
	}


	inline void printIndividualHitsLogsum( BigSerial individualId, double logsum )
	{
        static bool printHeader = true;

        if(printHeader)
        {
        	printHeader = false;
        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_HITS_LOGSUM,"individualId, logsum");
        }

		boost::format fmtr = boost::format("%1%, %2%") % individualId % logsum;

		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_HITS_LOGSUM, fmtr.str());

	}

	inline void printHouseholdHitsLogsum( std::string title, std::string hitsId, std::string householdId, std::string individualId, std::string paxId, vector<double> logsum )
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
											"%1148%, %1149%, %1150%, %1151%, %1152%, %1153%, %1154%, %1155%, %1156%, %1157%, %1158%, %1159%, %1160%, %1161%, %1162%, %1163%, %1164%, %1165%, %1166%, %1167%, %1168%, %1169%, %1170%, %1171%, %1172%, %1173%, %1174%, %1175% "
											/*"%1176%, %1177%, %1178%, %1179%, %1180%, %1181%, %1182%, %1183%, %1184%, %1185%, %1186%, %1187%, %1188%, %1189%, %1190%, %1191%, %1192%, %1193%, %1194%, %1195%, %1196%, %1197%, %1198%, %1199%, %1200%, %1201%, %1202%, %1203%, "
											"%1204%, %1205%, %1206%, %1207%, %1208%, %1209%, %1210%, %1211%, %1212%, %1213%, %1214%, %1215%, %1216%, %1217%, %1218%, %1219%, %1220%, %1221%, %1222%, %1223%, %1224%, %1225%, %1226%, %1227%, %1228%, %1229%, %1230%, %1231%, "
											"%1232%, %1233%, %1234%, %1235%, %1236%, %1237%, %1238%, %1239%, %1240%, %1241%, %1242%, %1243%, %1244%, %1245%, %1246%, %1247%, %1248%, %1249%, %1250%, %1251%, %1252%, %1253%, %1254%, %1255%, %1256%, %1257%, %1258%, %1259%, "
											"%1260%, %1261%, %1262%, %1263%, %1264%, %1265%, %1266%, %1267%, %1268%, %1269%, %1270%, %1271%, %1272%"*/) % title % hitsId % householdId % individualId % paxId
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
											% logsum[1163]  % logsum[1164]  % logsum[1165]  % logsum[1166]  % logsum[1167]  % logsum[1168]  % logsum[1169]  /*% logsum[1170]  % logsum[1171]  % logsum[1172]  % logsum[1173]  % logsum[1174]
											% logsum[1175]  % logsum[1176]  % logsum[1177]  % logsum[1178]  % logsum[1179]  % logsum[1180]  % logsum[1181]  % logsum[1182]  % logsum[1183]  % logsum[1184]  % logsum[1185]  % logsum[1186]
											% logsum[1187]  % logsum[1188]  % logsum[1189]  % logsum[1190]  % logsum[1191]  % logsum[1192]  % logsum[1193]  % logsum[1194]  % logsum[1195]  % logsum[1196]  % logsum[1197]  % logsum[1198]
											% logsum[1199]  % logsum[1200]  % logsum[1201]  % logsum[1202]  % logsum[1203]  % logsum[1204]  % logsum[1205]  % logsum[1206]  % logsum[1207]  % logsum[1208]  % logsum[1209]  % logsum[1210]
											% logsum[1211]  % logsum[1212]  % logsum[1213]  % logsum[1214]  % logsum[1215]  % logsum[1216]  % logsum[1217]  % logsum[1218]  % logsum[1219]  % logsum[1220]  % logsum[1221]  % logsum[1222]
											% logsum[1223]  % logsum[1224]  % logsum[1225]  % logsum[1226]  % logsum[1227]  % logsum[1228]  % logsum[1229]  % logsum[1230]  % logsum[1231]  % logsum[1232]  % logsum[1233]  % logsum[1234]
											% logsum[1235]  % logsum[1236]  % logsum[1237]  % logsum[1238]  % logsum[1239]  % logsum[1240]  % logsum[1241]  % logsum[1242]  % logsum[1243]  % logsum[1244]  % logsum[1245]  % logsum[1246]
											% logsum[1247]  % logsum[1248]  % logsum[1249]  % logsum[1250]  % logsum[1251]  % logsum[1252]  % logsum[1253]  % logsum[1254]  % logsum[1255]  % logsum[1256]  % logsum[1257]  % logsum[1258]
											% logsum[1259]  % logsum[1260]  % logsum[1261]  % logsum[1262]  % logsum[1263]  % logsum[1264]  % logsum[1265]  % logsum[1266]*/;



		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_HITS_LOGSUM, fmtr.str());

		std::cout << fmtr.str() << std::endl;
	}

	//hitsId , paxId , householdId , individualId , memberId , tazH , tazW , logsum[0] , logsum[1] ,logsum[2] , logsum[3] ,logsum[4] , logsum[5] ,travelProbability[0] , travelProbability[1] , travelProbability[2] , travelProbability[3] ,travelProbability[4] , travelProbability[5] ,tripsExpected[0] , tripsExpected[1], tripsExpected[2] , tripsExpected[3], tripsExpected[4] , tripsExpected[5]
	inline void printHouseholdHitsLogsumFVO( std::string hitsId, int paxId, BigSerial householdId, BigSerial individualId, int memberId, int tazH, int tazW, vector<double> logsum, vector<double> travelProbability, vector<double> tripsExpected )
	{
        static bool printHeader = true;

        if(printHeader)
        {
        	printHeader = false;
        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_LOGSUM_VO,"hitsId , paxId , householdId , individualId , memberId , tazH , tazW , logsum[0] , logsum[1] ,logsum[2] , logsum[3] ,logsum[4] , logsum[5] ,travelProbability[0] , travelProbability[1] , travelProbability[2] , travelProbability[3] ,travelProbability[4] , travelProbability[5] ,tripsExpected[0] , tripsExpected[1], tripsExpected[2] , tripsExpected[3], tripsExpected[4] , tripsExpected[5]");
        }

		boost::format fmtr = boost::format( "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25% ")
											 % hitsId % paxId % householdId % individualId % memberId % tazH % tazW
											 % logsum[0] % logsum[1] % logsum[2] % logsum[3] % logsum[4] % logsum[5]
											 % travelProbability[0] % travelProbability[1] % travelProbability[2] % travelProbability[3] % travelProbability[4] % travelProbability[5]
											 % tripsExpected[0] % tripsExpected[1] % tripsExpected[2] % tripsExpected[3] % tripsExpected[4] % tripsExpected[5];
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_INDIVIDUAL_LOGSUM_VO, fmtr.str());
	}

	inline void printProbabilityList( BigSerial householdId, std::vector<double>probabilities )
	{
		static bool printHeader = true;

		if(printHeader)
		{
			printHeader = false;
			AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCREENINGPROBABILITIES,"householdId , probabilities[0], probabilities[1], probabilities[2], probabilities[3], probabilities[4], probabilities[5], probabilities[6], probabilities[7], probabilities[8], probabilities[9], probabilities[10], probabilities[11], probabilities[12], probabilities[13], probabilities[14], probabilities[15], probabilities[16], probabilities[17], probabilities[18], probabilities[19], probabilities[20], probabilities[21], probabilities[22], probabilities[23], probabilities[24], probabilities[25], probabilities[26], probabilities[27], probabilities[28], probabilities[29], probabilities[30], probabilities[31], probabilities[32], probabilities[33], probabilities[34], probabilities[35], probabilities[36], probabilities[37], probabilities[38], probabilities[39], probabilities[40], probabilities[41], probabilities[42], probabilities[43], probabilities[44], probabilities[45], probabilities[46], probabilities[47], probabilities[48], probabilities[49], probabilities[50], probabilities[51], probabilities[52], probabilities[53], probabilities[54], probabilities[55], probabilities[56], probabilities[57], probabilities[58], probabilities[59], probabilities[60], probabilities[61], probabilities[62], probabilities[63], probabilities[64], probabilities[65], probabilities[66], probabilities[67], probabilities[68], probabilities[69], probabilities[70], probabilities[71], probabilities[72], probabilities[73], probabilities[74], probabilities[75], probabilities[76], probabilities[77], probabilities[78], probabilities[79], probabilities[80], probabilities[81], probabilities[82], probabilities[83], probabilities[84], probabilities[85], probabilities[86], probabilities[87], probabilities[88], probabilities[89], probabilities[90], probabilities[91], probabilities[92], probabilities[93], probabilities[94], probabilities[95], probabilities[96], probabilities[97], probabilities[98], probabilities[99], probabilities[100], probabilities[101], probabilities[102], probabilities[103], probabilities[104], probabilities[105], probabilities[106], probabilities[107], probabilities[108], probabilities[109], probabilities[110], probabilities[111], probabilities[112], probabilities[113], probabilities[114], probabilities[115], probabilities[116], probabilities[117], probabilities[118], probabilities[119], probabilities[120], probabilities[121], probabilities[122], probabilities[123], probabilities[124], probabilities[125], probabilities[126], probabilities[127], probabilities[128], probabilities[129], probabilities[130], probabilities[131], probabilities[132], probabilities[133], probabilities[134], probabilities[135], probabilities[136], probabilities[137], probabilities[138], probabilities[139], probabilities[140], probabilities[141], probabilities[142], probabilities[143], probabilities[144], probabilities[145], probabilities[146], probabilities[147], probabilities[148], probabilities[149], probabilities[150], probabilities[151], probabilities[152], probabilities[153], probabilities[154], probabilities[155], probabilities[156], probabilities[157], probabilities[158], probabilities[159], probabilities[160], probabilities[161], probabilities[162], probabilities[163], probabilities[164], probabilities[165], probabilities[166], probabilities[167], probabilities[168], probabilities[169], probabilities[170], probabilities[171], probabilities[172], probabilities[173], probabilities[174], probabilities[175], probabilities[176], probabilities[177], probabilities[178], probabilities[179], probabilities[180], probabilities[181], probabilities[182], probabilities[183], probabilities[184], probabilities[185], probabilities[186], probabilities[187], probabilities[188], probabilities[189], probabilities[190], probabilities[191], probabilities[192], probabilities[193], probabilities[194], probabilities[195], probabilities[196], probabilities[197], probabilities[198], probabilities[199], probabilities[200], probabilities[201], probabilities[202], probabilities[203], probabilities[204], probabilities[205], probabilities[206], probabilities[207], probabilities[208], probabilities[209], probabilities[210], probabilities[211], probabilities[212], probabilities[213], probabilities[214], probabilities[215]");
		}

		boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25%, %26%, %27%, %28%, %29%, %30%, %31%, %32%, %33%, %34%, %35%, %36%, %37%, %38%, %39%, %40%, %41%, %42%, "
											"%43%, %44%, %45%, %46%, %47%, %48%, %49%, %50%, %51%, %52%, %53%, %54%, %55%, %56%, %57%, %58%, %59%, %60%, %61%, %62%, %63%, %64%, %65%, %66%, %67%, %68%, %69%, %70%, %71%, %72%, %73%, %74%, %75%, %76%, %77%, %78%, %79%, %80%, %81%, %82%, "
											"%83%, %84%, %85%, %86%, %87%, %88%, %89%, %90%, %91%, %92%, %93%, %94%, %95%, %96%, %97%, %98%, %99%, %100%, %101%, %102%, %103%, %104%, %105%, %106%, %107%, %108%, %109%, %110%, %111%, %112%, %113%, %114%, %115%, %116%, %117%, %118%, "
											"%119%, %120%, %121%, %122%, %123%, %124%, %125%, %126%, %127%, %128%, %129%, %130%, %131%, %132%, %133%, %134%, %135%, %136%, %137%, %138%, %139%, %140%, %141%, %142%, %143%, %144%, %145%, %146%, %147%, %148%, %149%, %150%, %151%, "
											"%152%, %153%, %154%, %155%, %156%, %157%, %158%, %159%, %160%, %161%, %162%, %163%, %164%, %165%, %166%, %167%, %168%, %169%, %170%, %171%, %172%, %173%, %174%, %175%, %176%, %177%, %178%, %179%, %180%, %181%, %182%, %183%, %184%, "
											"%185%, %186%, %187%, %188%, %189%, %190%, %191%, %192%, %193%, %194%, %195%, %196%, %197%, %198%, %199%, %200%, %201%, %202%, %203%, %204%, %205%, %206%, %207%, %208%, %209%, %210%, %211%, %212%, %213%, %214%, %215%, %216%, %217%"
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
											% probabilities[189]  % probabilities[190]  % probabilities[191]  % probabilities[192]  % probabilities[193]  % probabilities[194]  % probabilities[195]  % probabilities[196]  % probabilities[197]  % probabilities[198]  % probabilities[199]  % probabilities[200]  % probabilities[201]
											% probabilities[202]  % probabilities[203]  % probabilities[204]  % probabilities[205]  % probabilities[206]  % probabilities[207]  % probabilities[208]  % probabilities[209]  % probabilities[210]  % probabilities[211]  % probabilities[212]  % probabilities[213]  % probabilities[214]  % probabilities[215];

		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCREENINGPROBABILITIES,fmtr.str());
	}


	    inline void writeVehicleOwnershipToFile(BigSerial hhId,int VehiclOwnershiOptionId)
	    {
	        static bool printHeader = true;

	        if(printHeader)
	        {
	        	printHeader = false;
	        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_VEHICLE_OWNERSIP,"householdId, vehicleOwnershipOption");
	        }

	    	boost::format fmtr = boost::format("%1%, %2%") % hhId % VehiclOwnershiOptionId;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_VEHICLE_OWNERSIP,fmtr.str());

	    }
	    									//day, householdId, unitId, willingnessToPay, AskingPrice, Affordability, BidAmount, Surplus, currentPostcode, unitPostcode
	    inline void printHouseholdBiddingList(  int day, BigSerial householdId, BigSerial unitId, std::string postcodeCurrent, std::string postcodeNew,
	    										double wp, double askingPrice, double affordability, double currentBid, double currentSurplus)
	    {
	        static bool printHeader = true;

	        if(printHeader)
	        {
	        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDBIDLIST,"day, householdId, unitId, willingnessToPay, AskingPrice, Affordability, BidAmount, Surplus, currentPostcode, unitPostcode");
	        	printHeader = false;
	        }

	    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%") % day % householdId % unitId % wp % askingPrice
	    																							% affordability % currentBid % currentSurplus % postcodeCurrent % postcodeNew;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDBIDLIST,fmtr.str());
	    }


	    inline void printChoiceset( int day, BigSerial householdId, std::string choiceset)
	    {
	        static bool printHeader = true;

	        if(printHeader)
	        {
	        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HHCHOICESET,"day, householdId, unitId1, unitId2, unitId3, unitId4, unitId5, unitId6, unitId7, unitId8, unitId9, unitId10, unitId11, unitId12, unitId13, unitId14, unitId15, unitId16, unitId17, unitId18, unitId19, unitId20, unitId21, unitId22, unitId23, unitId24, unitId25, unitId26, unitId27, unitId28, unitId29, unitId30, unitId31, unitId32, unitId33, unitId34, unitId35, unitId36, unitId37, unitId38, unitId39, unitId40, unitId41, unitId42, unitId43, unitId44, unitId45, unitId46, unitId47, unitId48, unitId49, unitId50, unitId51, unitId52, unitId53, unitId54, unitId55, unitId56, unitId57, unitId58, unitId59, unitId60, unitId61, unitId62, unitId63, unitId64, unitId65, unitId66, unitId67, unitId68, unitId69, unitId70");
	        	printHeader = false;
	        }

	    	boost::format fmtr = boost::format("%1%, %2%, %3%")% day % householdId % choiceset;

	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HHCHOICESET,fmtr.str());
	    }

	    inline void PrintExit(int day, const Household *household, int result)
	   	{
	        static bool printHeader = true;

	        if(printHeader)
	        {
	        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HH_EXIT,"day, household, exit_status");
	        	printHeader = false;
	        }

	    	//day household_id timeOnMarket
	    	boost::format fmtr = boost::format("%1%, %2%, %3%") % (day + 1) % household->getId() % result;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HH_EXIT, fmtr.str());
	    }


	    //bid_timestamp, seller_id, bidder_id, unit_id, bidder wtp, bidder wp+wp_error, wp_error, affordability, currentUnitHP,target_price, hedonicprice, lagCoefficient, asking_price, bid_value, bids_counter (daily), bid_status, logsum, floor_area, type_id, HHPC, UPC,sale_from_date,occupancy_from_date
	    const std::string LOG_BID = "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%";

	    inline void printBidGeneric(HM_Model* model, int id, const Bid& bid, const ExpectationEntry& entry, unsigned int bidsCounter, bool accepted)
	    {

	        static bool printHeader = true;

	        if(printHeader)
	        {
	        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::BIDS, "bid_timestamp, seller_id, bidder_id, unit_id, bidder wtp, bidder wp+wp_error, wp_error, affordability, currentUnitHP,target_price, hedonicprice, lagCoefficient, asking_price, bid_value, bids_counter (daily), bid_status, logsum, floor_area, type_id, HHPC, UPC,sale_from_date,occupancy_from_date" );
	        	printHeader = false;
	        }

	    	const Unit* unit  = model->getUnitById(bid.getNewUnitId());
	        double floor_area = unit->getFloorArea();
	        BigSerial type_id = unit->getUnitType();
	        int UnitslaId = unit->getSlaAddressId();
	        Postcode *unitPostcode = model->getPostcodeById(UnitslaId);


	        Household *thisBidder = model->getHouseholdById(bid.getBidderId());
	        const Unit* currentHHUnit = model->getUnitById(thisBidder->getUnitId());
	        Postcode* thisPostcode = model->getPostcodeById( currentHHUnit->getSlaAddressId() );

	        boost::gregorian::date saleFromDate 	 = boost::gregorian::date_from_tm(unit->getSaleFromDate());
	        boost::gregorian::date occupancyFromDate = boost::gregorian::date_from_tm(unit->getOccupancyFromDate());

	        boost::format fmtr = boost::format(LOG_BID) % bid.getSimulationDay()
														% id
														% bid.getBidderId()
														% bid.getNewUnitId()
														% (bid.getWillingnessToPay() - bid.getWtpErrorTerm())
														% bid.getWillingnessToPay()
														% bid.getWtpErrorTerm()
														% thisBidder->getAffordabilityAmount()
														% thisBidder->getCurrentUnitPrice()
														% entry.targetPrice
														% entry.hedonicPrice
														% unit->getLagCoefficient()
														% entry.askingPrice
														% bid.getBidValue()
														% bidsCounter
														% ((accepted) ? 1 : 0)
														% thisBidder->getLogsum()
														% floor_area
														% type_id
														% thisPostcode->getSlaPostcode()
														% unitPostcode->getSlaPostcode()
														% boost::gregorian::to_iso_extended_string(saleFromDate)
														% boost::gregorian::to_iso_extended_string(occupancyFromDate);

	        AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::BIDS, fmtr.str());
	    }



	    /**
	     * Print the current bid on the unit.
	     * @param agent to received the bid
	     * @param bid to send.
	     * @param struct containing the hedonic, asking and target price.
	     * @param number of bids for this unit
	     * @param boolean indicating if the bid was successful
	     *
	     */
	    inline void printBid(const RealEstateAgent& agent, const Bid& bid, const ExpectationEntry& entry, unsigned int bidsCounter, bool accepted)
	    {
	    	HM_Model* model = agent.getModel();

	    	int id = agent.getId();

	    	printBidGeneric( model,  id, bid, entry,bidsCounter,  accepted);
	    }


	    /**
	     * Print the current bid on the unit.
	     * @param agent to received the bid
	     * @param bid to send.
	     * @param struct containing the hedonic, asking and target price.
	     * @param number of bids for this unit
	     * @param boolean indicating if the bid was successful
	     *
	     */
	    inline void printBid(const HouseholdAgent& agent, const Bid& bid, const ExpectationEntry& entry, unsigned int bidsCounter, bool accepted)
	    {

	    	HM_Model* model = agent.getModel();
	    	int id = agent.getId();

	    	printBidGeneric( model,  id, bid, entry,bidsCounter,  accepted);
	    }

	    inline void writePreSchoolAssignmentsToFile(BigSerial hhId,BigSerial individualId,BigSerial schoolId)
	    {
	        static bool printHeader = true;

	        if(printHeader)
	        {
	        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_PRE_SCHOOL_ASSIGNMENT,"hhid, individual_id, school_id");
	        	printHeader = false;
	        }

	    	boost::format fmtr = boost::format("%1%, %2%, %3%") % hhId % individualId % schoolId;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_PRE_SCHOOL_ASSIGNMENT,fmtr.str());

	    }

	    inline void writeSchoolAssignmentsToFile(BigSerial individualId,BigSerial priSchoolId)
	    {
	        static bool printHeader = true;

	        if(printHeader)
	        {
	        	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCHOOL_ASSIGNMENT,"individualId, primarySchoolId");
	        	printHeader = false;
	        }

	    	boost::format fmtr = boost::format("%1%, %2%") % individualId % priSchoolId;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCHOOL_ASSIGNMENT,fmtr.str());

	    }


	    inline void writeRandomNumsToFile(int counter,BigSerial hhId, float montecarlo)
	    {
	    	boost::format fmtr = boost::format("%1%, %2%, %3%") % counter % hhId % montecarlo;

	    	static bool printHeader = true;

	    	if(printHeader)
	    	{
	    		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_RANDOM_NUMS,"randomNumber");
	    		printHeader = false;
	    	}
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_RANDOM_NUMS,fmtr.str());

	    }


	    inline void writeROIDataToFile(const Parcel &parcel, int newDevelopment, double profit, int devType, float threshold_roi, float roi)
	    {
	    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%") % parcel.getId() % newDevelopment % profit % devType % threshold_roi % roi;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_DEV_ROI,fmtr.str());
	    }

	    /**
	     * Write the data of units to a csv.
	     * @param unit to be written.
	     *
	     */
	    inline void printNewUnitsInMarket(BigSerial sellerId, BigSerial unitId, int entryday, int timeOnMarket, int timeOffMarket)
	    {
	    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%") % sellerId % unitId % entryday % timeOnMarket % timeOffMarket;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::UNITS_IN_MARKET,fmtr.str());
	    }

	    inline void writeNonEligibleParcelsToFile(BigSerial parcelId, std::string reason)
	    {
	    	boost::format fmtr = boost::format("%1%, %2%") % parcelId % reason;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_NON_ELIGIBLE_PARCELS,fmtr.str());
	    }

	    inline void writeEligibleParcelsToFile(BigSerial parcelId, int newDevelopment)
	    {
	    	boost::format fmtr = boost::format("%1%, %2%") % parcelId % newDevelopment;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ELIGIBLE_PARCELS,fmtr.str());
	    }

	    inline void writeGPRToFile(double parcelId, double parcelGPR, double actualGPR, double gap)
	    {
	    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%") % parcelId % parcelGPR % actualGPR % gap;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_GPR,fmtr.str());
	    }

	}
}
