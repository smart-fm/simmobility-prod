//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Hits2008ScreeningProb.hpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class Hits2008ScreeningProb
		{
		public:

			Hits2008ScreeningProb(	BigSerial id = 0, std::string h1_hhid="",
									double p1=0, double p2=0, double p3=0, double p4=0, double p5=0, double p6=0, double p7=0, double p8=0, double p9=0, double p10=0, double p11=0, double p12=0,
									double p13=0, double p14=0, double p15=0, double p16=0, double p17=0, double p18=0, double p19=0, double p20=0, double p21=0, double p22=0, double p23=0,
									double p24=0, double p25=0, double p26=0, double p27=0, double p28=0, double p29=0, double p30=0, double p31=0, double p32=0, double p33=0, double p34=0, double p35=0,
									double p36=0, double p37=0, double p38=0, double p39=0, double p40=0, double p41=0, double p42=0, double p43=0, double p44=0, double p45=0, double p46=0, double p47=0,
									double p48=0, double p49=0, double p50=0, double p51=0, double p52=0, double p53=0, double p54=0, double p55=0, double p56=0, double p57=0, double p58=0, double p59=0,
									double p60=0, double p61=0, double p62=0, double p63=0, double p64=0, double p65=0, double p66=0, double p67=0, double p68=0, double p69=0, double p70=0, double p71=0,
									double p72=0, double p73=0, double p74=0, double p75=0, double p76=0, double p77=0, double p78=0, double p79=0, double p80=0, double p81=0, double p82=0, double p83=0,
									double p84=0, double p85=0, double p86=0, double p87=0, double p88=0, double p89=0, double p90=0, double p91=0, double p92=0, double p93=0, double p94=0, double p95=0,
									double p96=0, double p97=0, double p98=0, double p99=0, double p100=0, double p101=0, double p102=0, double p103=0, double p104=0, double p105=0, double p106=0,
									double p107=0, double p108=0, double p109=0, double p110=0, double p111=0, double p112=0, double p113=0, double p114=0, double p115=0, double p116=0, double p117=0,
									double p118=0, double p119=0, double p120=0, double p121=0, double p122=0, double p123=0, double p124=0, double p125=0, double p126=0, double p127=0, double p128=0,
									double p129=0, double p130=0, double p131=0, double p132=0, double p133=0, double p134=0, double p135=0, double p136=0, double p137=0, double p138=0, double p139=0,
									double p140=0, double p141=0, double p142=0, double p143=0, double p144=0, double p145=0, double p146=0, double p147=0, double p148=0, double p149=0, double p150=0,
									double p151=0, double p152=0, double p153=0, double p154=0, double p155=0, double p156=0, double p157=0, double p158=0, double p159=0, double p160=0, double p161=0,
									double p162=0, double p163=0, double p164=0, double p165=0, double p166=0, double p167=0, double p168=0, double p169=0, double p170=0, double p171=0, double p172=0,
									double p173=0, double p174=0, double p175=0, double p176=0, double p177=0, double p178=0, double p179=0, double p180=0, double p181=0, double p182=0, double p183=0,
									double p184=0, double p185=0, double p186=0, double p187=0, double p188=0, double p189=0, double p190=0, double p191=0, double p192=0, double p193=0, double p194=0,
									double p195=0, double p196=0, double p197=0, double p198=0, double p199=0, double p200=0, double p201=0, double p202=0, double p203=0, double p204=0, double p205=0,
									double p206=0, double p207=0, double p208=0, double p209=0, double p210=0, double p211=0, double p212=0, double p213=0, double p214=0, double p215=0);


			virtual ~Hits2008ScreeningProb();

			Hits2008ScreeningProb(const Hits2008ScreeningProb& source);

			Hits2008ScreeningProb operator=(const Hits2008ScreeningProb& source );

			BigSerial getId() const;
			std::string getH1HhId() const;

			friend std::ostream& operator<<(std::ostream& strm, const Hits2008ScreeningProb& data);

			void getProbabilities(std::vector<double> &screeningProbabilities ) const;
			double getP1() const;
			double getP2() const;
			double getP3() const;
			double getP4() const;
			double getP5() const;
			double getP6() const;
			double getP7() const;
			double getP8() const;
			double getP9() const;
			double getP10() const;
			double getP11() const;
			double getP12() const;
			double getP13() const;
			double getP14() const;
			double getP15() const;
			double getP16() const;
			double getP17() const;
			double getP18() const;
			double getP19() const;
			double getP20() const;
			double getP21() const;
			double getP22() const;
			double getP23() const;
			double getP24() const;
			double getP25() const;
			double getP26() const;
			double getP27() const;
			double getP28() const;
			double getP29() const;
			double getP30() const;
			double getP31() const;
			double getP32() const;
			double getP33() const;
			double getP34() const;
			double getP35() const;
			double getP36() const;
			double getP37() const;
			double getP38() const;
			double getP39() const;
			double getP40() const;
			double getP41() const;
			double getP42() const;
			double getP43() const;
			double getP44() const;
			double getP45() const;
			double getP46() const;
			double getP47() const;
			double getP48() const;
			double getP49() const;
			double getP50() const;
			double getP51() const;
			double getP52() const;
			double getP53() const;
			double getP54() const;
			double getP55() const;
			double getP56() const;
			double getP57() const;
			double getP58() const;
			double getP59() const;
			double getP60() const;
			double getP61() const;
			double getP62() const;
			double getP63() const;
			double getP64() const;
			double getP65() const;
			double getP66() const;
			double getP67() const;
			double getP68() const;
			double getP69() const;
			double getP70() const;
			double getP71() const;
			double getP72() const;
			double getP73() const;
			double getP74() const;
			double getP75() const;
			double getP76() const;
			double getP77() const;
			double getP78() const;
			double getP79() const;
			double getP80() const;
			double getP81() const;
			double getP82() const;
			double getP83() const;
			double getP84() const;
			double getP85() const;
			double getP86() const;
			double getP87() const;
			double getP88() const;
			double getP89() const;
			double getP90() const;
			double getP91() const;
			double getP92() const;
			double getP93() const;
			double getP94() const;
			double getP95() const;
			double getP96() const;
			double getP97() const;
			double getP98() const;
			double getP99() const;
			double getP100() const;
			double getP101() const;
			double getP102() const;
			double getP103() const;
			double getP104() const;
			double getP105() const;
			double getP106() const;
			double getP107() const;
			double getP108() const;
			double getP109() const;
			double getP110() const;
			double getP111() const;
			double getP112() const;
			double getP113() const;
			double getP114() const;
			double getP115() const;
			double getP116() const;
			double getP117() const;
			double getP118() const;
			double getP119() const;
			double getP120() const;
			double getP121() const;
			double getP122() const;
			double getP123() const;
			double getP124() const;
			double getP125() const;
			double getP126() const;
			double getP127() const;
			double getP128() const;
			double getP129() const;
			double getP130() const;
			double getP131() const;
			double getP132() const;
			double getP133() const;
			double getP134() const;
			double getP135() const;
			double getP136() const;
			double getP137() const;
			double getP138() const;
			double getP139() const;
			double getP140() const;
			double getP141() const;
			double getP142() const;
			double getP143() const;
			double getP144() const;
			double getP145() const;
			double getP146() const;
			double getP147() const;
			double getP148() const;
			double getP149() const;
			double getP150() const;
			double getP151() const;
			double getP152() const;
			double getP153() const;
			double getP154() const;
			double getP155() const;
			double getP156() const;
			double getP157() const;
			double getP158() const;
			double getP159() const;
			double getP160() const;
			double getP161() const;
			double getP162() const;
			double getP163() const;
			double getP164() const;
			double getP165() const;
			double getP166() const;
			double getP167() const;
			double getP168() const;
			double getP169() const;
			double getP170() const;
			double getP171() const;
			double getP172() const;
			double getP173() const;
			double getP174() const;
			double getP175() const;
			double getP176() const;
			double getP177() const;
			double getP178() const;
			double getP179() const;
			double getP180() const;
			double getP181() const;
			double getP182() const;
			double getP183() const;
			double getP184() const;
			double getP185() const;
			double getP186() const;
			double getP187() const;
			double getP188() const;
			double getP189() const;
			double getP190() const;
			double getP191() const;
			double getP192() const;
			double getP193() const;
			double getP194() const;
			double getP195() const;
			double getP196() const;
			double getP197() const;
			double getP198() const;
			double getP199() const;
			double getP200() const;
			double getP201() const;
			double getP202() const;
			double getP203() const;
			double getP204() const;
			double getP205() const;
			double getP206() const;
			double getP207() const;
			double getP208() const;
			double getP209() const;
			double getP210() const;
			double getP211() const;
			double getP212() const;
			double getP213() const;
			double getP214() const;
			double getP215() const;

		private:
			friend class Hits2008ScreeningProbDao;

			BigSerial id;
			std::string h1_hhid;

			double p1;
			double p2;
			double p3;
			double p4;
			double p5;
			double p6;
			double p7;
			double p8;
			double p9;
			double p10;
			double p11;
			double p12;
			double p13;
			double p14;
			double p15;
			double p16;
			double p17;
			double p18;
			double p19;
			double p20;
			double p21;
			double p22;
			double p23;
			double p24;
			double p25;
			double p26;
			double p27;
			double p28;
			double p29;
			double p30;
			double p31;
			double p32;
			double p33;
			double p34;
			double p35;
			double p36;
			double p37;
			double p38;
			double p39;
			double p40;
			double p41;
			double p42;
			double p43;
			double p44;
			double p45;
			double p46;
			double p47;
			double p48;
			double p49;
			double p50;
			double p51;
			double p52;
			double p53;
			double p54;
			double p55;
			double p56;
			double p57;
			double p58;
			double p59;
			double p60;
			double p61;
			double p62;
			double p63;
			double p64;
			double p65;
			double p66;
			double p67;
			double p68;
			double p69;
			double p70;
			double p71;
			double p72;
			double p73;
			double p74;
			double p75;
			double p76;
			double p77;
			double p78;
			double p79;
			double p80;
			double p81;
			double p82;
			double p83;
			double p84;
			double p85;
			double p86;
			double p87;
			double p88;
			double p89;
			double p90;
			double p91;
			double p92;
			double p93;
			double p94;
			double p95;
			double p96;
			double p97;
			double p98;
			double p99;
			double p100;
			double p101;
			double p102;
			double p103;
			double p104;
			double p105;
			double p106;
			double p107;
			double p108;
			double p109;
			double p110;
			double p111;
			double p112;
			double p113;
			double p114;
			double p115;
			double p116;
			double p117;
			double p118;
			double p119;
			double p120;
			double p121;
			double p122;
			double p123;
			double p124;
			double p125;
			double p126;
			double p127;
			double p128;
			double p129;
			double p130;
			double p131;
			double p132;
			double p133;
			double p134;
			double p135;
			double p136;
			double p137;
			double p138;
			double p139;
			double p140;
			double p141;
			double p142;
			double p143;
			double p144;
			double p145;
			double p146;
			double p147;
			double p148;
			double p149;
			double p150;
			double p151;
			double p152;
			double p153;
			double p154;
			double p155;
			double p156;
			double p157;
			double p158;
			double p159;
			double p160;
			double p161;
			double p162;
			double p163;
			double p164;
			double p165;
			double p166;
			double p167;
			double p168;
			double p169;
			double p170;
			double p171;
			double p172;
			double p173;
			double p174;
			double p175;
			double p176;
			double p177;
			double p178;
			double p179;
			double p180;
			double p181;
			double p182;
			double p183;
			double p184;
			double p185;
			double p186;
			double p187;
			double p188;
			double p189;
			double p190;
			double p191;
			double p192;
			double p193;
			double p194;
			double p195;
			double p196;
			double p197;
			double p198;
			double p199;
			double p200;
			double p201;
			double p202;
			double p203;
			double p204;
			double p205;
			double p206;
			double p207;
			double p208;
			double p209;
			double p210;
			double p211;
			double p212;
			double p213;
			double p214;
			double p215;

		};
	}

} /* namespace sim_mob */
