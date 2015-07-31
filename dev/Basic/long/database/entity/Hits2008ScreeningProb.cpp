//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Hits2008ScreeningProb.cpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/Hits2008ScreeningProb.hpp>

namespace sim_mob
{
	namespace long_term
	{

		Hits2008ScreeningProb::Hits2008ScreeningProb(std::string h1_hhid,
													 double p1, double p2, double p3, double p4, double p5, double p6, double p7, double p8, double p9, double p10, double p11, double p12, double p13,
													 double p14, double p15, double p16, double p17, double p18, double p19, double p20, double p21, double p22, double p23, double p24, double p25,
													 double p26, double p27, double p28, double p29, double p30, double p31, double p32, double p33, double p34, double p35, double p36, double p37,
													 double p38, double p39, double p40, double p41, double p42, double p43, double p44, double p45, double p46, double p47, double p48, double p49,
													 double p50, double p51, double p52, double p53, double p54, double p55, double p56, double p57, double p58, double p59, double p60, double p61,
													 double p62, double p63, double p64, double p65, double p66, double p67, double p68, double p69, double p70, double p71, double p72, double p73,
													 double p74, double p75, double p76, double p77, double p78, double p79, double p80, double p81, double p82, double p83, double p84, double p85,
													 double p86, double p87, double p88, double p89, double p90, double p91, double p92, double p93, double p94, double p95, double p96, double p97,
													 double p98, double p99, double p100, double p101, double p102, double p103, double p104, double p105, double p106, double p107, double p108,
													 double p109, double p110, double p111, double p112, double p113, double p114, double p115, double p116, double p117, double p118, double p119,
													 double p120, double p121, double p122, double p123, double p124, double p125, double p126, double p127, double p128, double p129, double p130,
													 double p131, double p132, double p133, double p134, double p135, double p136, double p137, double p138, double p139, double p140, double p141,
													 double p142, double p143, double p144, double p145, double p146, double p147, double p148, double p149, double p150, double p151, double p152,
													 double p153, double p154, double p155, double p156, double p157, double p158, double p159, double p160, double p161, double p162, double p163,
													 double p164, double p165, double p166, double p167, double p168, double p169, double p170, double p171, double p172, double p173, double p174,
													 double p175, double p176, double p177, double p178, double p179, double p180, double p181, double p182, double p183, double p184, double p185,
													 double p186, double p187, double p188, double p189, double p190, double p191, double p192, double p193, double p194, double p195, double p196,
													 double p197, double p198, double p199, double p200, double p201, double p202, double p203, double p204, double p205, double p206, double p207,
													 double p208, double p209, double p210, double p211, double p212, double p213, double p214, double p215)
												 	 :h1_hhid(h1_hhid),
													 p1(p1),p2(p2),p3(p3),p4(p4),p5(p5),p6(p6),p7(p7),p8(p8),p9(p9),p10(p10),p11(p11),p12(p12),p13(p13),p14(p14),p15(p15),p16(p16),p17(p17),p18(p18),p19(p19),
													 p20(p20),p21(p21),p22(p22),p23(p23),p24(p24),p25(p25),p26(p26),p27(p27),p28(p28),p29(p29),p30(p30),p31(p31),p32(p32),p33(p33),p34(p34),p35(p35),p36(p36),
													 p37(p37),p38(p38),p39(p39),p40(p40),p41(p41),p42(p42),p43(p43),p44(p44),p45(p45),p46(p46),p47(p47),p48(p48),p49(p49),p50(p50),p51(p51),p52(p52),p53(p53),
													 p54(p54),p55(p55),p56(p56),p57(p57),p58(p58),p59(p59),p60(p60),p61(p61),p62(p62),p63(p63),p64(p64),p65(p65),p66(p66),p67(p67),p68(p68),p69(p69),p70(p70),
													 p71(p71),p72(p72),p73(p73),p74(p74),p75(p75),p76(p76),p77(p77),p78(p78),p79(p79),p80(p80),p81(p81),p82(p82),p83(p83),p84(p84),p85(p85),p86(p86),p87(p87),
													 p88(p88),p89(p89),p90(p90),p91(p91),p92(p92),p93(p93),p94(p94),p95(p95),p96(p96),p97(p97),p98(p98),p99(p99),p100(p100),p101(p101),p102(p102),p103(p103),
													 p104(p104),p105(p105),p106(p106),p107(p107),p108(p108),p109(p109),p110(p110),p111(p111),p112(p112),p113(p113),p114(p114),p115(p115),p116(p116),p117(p117),
													 p118(p118),p119(p119),p120(p120),p121(p121),p122(p122),p123(p123),p124(p124),p125(p125),p126(p126),p127(p127),p128(p128),p129(p129),p130(p130),p131(p131),
													 p132(p132),p133(p133),p134(p134),p135(p135),p136(p136),p137(p137),p138(p138),p139(p139),p140(p140),p141(p141),p142(p142),p143(p143),p144(p144),p145(p145),
													 p146(p146),p147(p147),p148(p148),p149(p149),p150(p150),p151(p151),p152(p152),p153(p153),p154(p154),p155(p155),p156(p156),p157(p157),p158(p158),p159(p159),
													 p160(p160),p161(p161),p162(p162),p163(p163),p164(p164),p165(p165),p166(p166),p167(p167),p168(p168),p169(p169),p170(p170),p171(p171),p172(p172),p173(p173),
													 p174(p174),p175(p175),p176(p176),p177(p177),p178(p178),p179(p179),p180(p180),p181(p181),p182(p182),p183(p183),p184(p184),p185(p185),p186(p186),p187(p187),
													 p188(p188),p189(p189),p190(p190),p191(p191),p192(p192),p193(p193),p194(p194),p195(p195),p196(p196),p197(p197),p198(p198),p199(p199),p200(p200),p201(p201),
													 p202(p202),p203(p203),p204(p204),p205(p205),p206(p206),p207(p207),p208(p208),p209(p209),p210(p210),p211(p211),p212(p212),p213(p213),p214(p214),p215(p215){}

		Hits2008ScreeningProb::~Hits2008ScreeningProb() {}

		Hits2008ScreeningProb::Hits2008ScreeningProb(const Hits2008ScreeningProb& source)
		{
			this->h1_hhid = source.h1_hhid;
			this->p1 = source.p1;
			this->p2 = source.p2;
			this->p3 = source.p3;
			this->p4 = source.p4;
			this->p5 = source.p5;
			this->p6 = source.p6;
			this->p7 = source.p7;
			this->p8 = source.p8;
			this->p9 = source.p9;
			this->p10 = source.p10;
			this->p11 = source.p11;
			this->p12 = source.p12;
			this->p13 = source.p13;
			this->p14 = source.p14;
			this->p15 = source.p15;
			this->p16 = source.p16;
			this->p17 = source.p17;
			this->p18 = source.p18;
			this->p19 = source.p19;
			this->p20 = source.p20;
			this->p21 = source.p21;
			this->p22 = source.p22;
			this->p23 = source.p23;
			this->p24 = source.p24;
			this->p25 = source.p25;
			this->p26 = source.p26;
			this->p27 = source.p27;
			this->p28 = source.p28;
			this->p29 = source.p29;
			this->p30 = source.p30;
			this->p31 = source.p31;
			this->p32 = source.p32;
			this->p33 = source.p33;
			this->p34 = source.p34;
			this->p35 = source.p35;
			this->p36 = source.p36;
			this->p37 = source.p37;
			this->p38 = source.p38;
			this->p39 = source.p39;
			this->p40 = source.p40;
			this->p41 = source.p41;
			this->p42 = source.p42;
			this->p43 = source.p43;
			this->p44 = source.p44;
			this->p45 = source.p45;
			this->p46 = source.p46;
			this->p47 = source.p47;
			this->p48 = source.p48;
			this->p49 = source.p49;
			this->p50 = source.p50;
			this->p51 = source.p51;
			this->p52 = source.p52;
			this->p53 = source.p53;
			this->p54 = source.p54;
			this->p55 = source.p55;
			this->p56 = source.p56;
			this->p57 = source.p57;
			this->p58 = source.p58;
			this->p59 = source.p59;
			this->p60 = source.p60;
			this->p61 = source.p61;
			this->p62 = source.p62;
			this->p63 = source.p63;
			this->p64 = source.p64;
			this->p65 = source.p65;
			this->p66 = source.p66;
			this->p67 = source.p67;
			this->p68 = source.p68;
			this->p69 = source.p69;
			this->p70 = source.p70;
			this->p71 = source.p71;
			this->p72 = source.p72;
			this->p73 = source.p73;
			this->p74 = source.p74;
			this->p75 = source.p75;
			this->p76 = source.p76;
			this->p77 = source.p77;
			this->p78 = source.p78;
			this->p79 = source.p79;
			this->p80 = source.p80;
			this->p81 = source.p81;
			this->p82 = source.p82;
			this->p83 = source.p83;
			this->p84 = source.p84;
			this->p85 = source.p85;
			this->p86 = source.p86;
			this->p87 = source.p87;
			this->p88 = source.p88;
			this->p89 = source.p89;
			this->p90 = source.p90;
			this->p91 = source.p91;
			this->p92 = source.p92;
			this->p93 = source.p93;
			this->p94 = source.p94;
			this->p95 = source.p95;
			this->p96 = source.p96;
			this->p97 = source.p97;
			this->p98 = source.p98;
			this->p99 = source.p99;
			this->p100 = source.p100;
			this->p101 = source.p101;
			this->p102 = source.p102;
			this->p103 = source.p103;
			this->p104 = source.p104;
			this->p105 = source.p105;
			this->p106 = source.p106;
			this->p107 = source.p107;
			this->p108 = source.p108;
			this->p109 = source.p109;
			this->p110 = source.p110;
			this->p111 = source.p111;
			this->p112 = source.p112;
			this->p113 = source.p113;
			this->p114 = source.p114;
			this->p115 = source.p115;
			this->p116 = source.p116;
			this->p117 = source.p117;
			this->p118 = source.p118;
			this->p119 = source.p119;
			this->p120 = source.p120;
			this->p121 = source.p121;
			this->p122 = source.p122;
			this->p123 = source.p123;
			this->p124 = source.p124;
			this->p125 = source.p125;
			this->p126 = source.p126;
			this->p127 = source.p127;
			this->p128 = source.p128;
			this->p129 = source.p129;
			this->p130 = source.p130;
			this->p131 = source.p131;
			this->p132 = source.p132;
			this->p133 = source.p133;
			this->p134 = source.p134;
			this->p135 = source.p135;
			this->p136 = source.p136;
			this->p137 = source.p137;
			this->p138 = source.p138;
			this->p139 = source.p139;
			this->p140 = source.p140;
			this->p141 = source.p141;
			this->p142 = source.p142;
			this->p143 = source.p143;
			this->p144 = source.p144;
			this->p145 = source.p145;
			this->p146 = source.p146;
			this->p147 = source.p147;
			this->p148 = source.p148;
			this->p149 = source.p149;
			this->p150 = source.p150;
			this->p151 = source.p151;
			this->p152 = source.p152;
			this->p153 = source.p153;
			this->p154 = source.p154;
			this->p155 = source.p155;
			this->p156 = source.p156;
			this->p157 = source.p157;
			this->p158 = source.p158;
			this->p159 = source.p159;
			this->p160 = source.p160;
			this->p161 = source.p161;
			this->p162 = source.p162;
			this->p163 = source.p163;
			this->p164 = source.p164;
			this->p165 = source.p165;
			this->p166 = source.p166;
			this->p167 = source.p167;
			this->p168 = source.p168;
			this->p169 = source.p169;
			this->p170 = source.p170;
			this->p171 = source.p171;
			this->p172 = source.p172;
			this->p173 = source.p173;
			this->p174 = source.p174;
			this->p175 = source.p175;
			this->p176 = source.p176;
			this->p177 = source.p177;
			this->p178 = source.p178;
			this->p179 = source.p179;
			this->p180 = source.p180;
			this->p181 = source.p181;
			this->p182 = source.p182;
			this->p183 = source.p183;
			this->p184 = source.p184;
			this->p185 = source.p185;
			this->p186 = source.p186;
			this->p187 = source.p187;
			this->p188 = source.p188;
			this->p189 = source.p189;
			this->p190 = source.p190;
			this->p191 = source.p191;
			this->p192 = source.p192;
			this->p193 = source.p193;
			this->p194 = source.p194;
			this->p195 = source.p195;
			this->p196 = source.p196;
			this->p197 = source.p197;
			this->p198 = source.p198;
			this->p199 = source.p199;
			this->p200 = source.p200;
			this->p201 = source.p201;
			this->p202 = source.p202;
			this->p203 = source.p203;
			this->p204 = source.p204;
			this->p205 = source.p205;
			this->p206 = source.p206;
			this->p207 = source.p207;
			this->p208 = source.p208;
			this->p209 = source.p209;
			this->p210 = source.p210;
			this->p211 = source.p211;
			this->p212 = source.p212;
			this->p213 = source.p213;
			this->p214 = source.p214;
			this->p215 = source.p215;
		}

		Hits2008ScreeningProb Hits2008ScreeningProb::operator=(const Hits2008ScreeningProb& source )
		{
			this->h1_hhid = source.h1_hhid;
			this->p1 = source.p1;
			this->p2 = source.p2;
			this->p3 = source.p3;
			this->p4 = source.p4;
			this->p5 = source.p5;
			this->p6 = source.p6;
			this->p7 = source.p7;
			this->p8 = source.p8;
			this->p9 = source.p9;
			this->p10 = source.p10;
			this->p11 = source.p11;
			this->p12 = source.p12;
			this->p13 = source.p13;
			this->p14 = source.p14;
			this->p15 = source.p15;
			this->p16 = source.p16;
			this->p17 = source.p17;
			this->p18 = source.p18;
			this->p19 = source.p19;
			this->p20 = source.p20;
			this->p21 = source.p21;
			this->p22 = source.p22;
			this->p23 = source.p23;
			this->p24 = source.p24;
			this->p25 = source.p25;
			this->p26 = source.p26;
			this->p27 = source.p27;
			this->p28 = source.p28;
			this->p29 = source.p29;
			this->p30 = source.p30;
			this->p31 = source.p31;
			this->p32 = source.p32;
			this->p33 = source.p33;
			this->p34 = source.p34;
			this->p35 = source.p35;
			this->p36 = source.p36;
			this->p37 = source.p37;
			this->p38 = source.p38;
			this->p39 = source.p39;
			this->p40 = source.p40;
			this->p41 = source.p41;
			this->p42 = source.p42;
			this->p43 = source.p43;
			this->p44 = source.p44;
			this->p45 = source.p45;
			this->p46 = source.p46;
			this->p47 = source.p47;
			this->p48 = source.p48;
			this->p49 = source.p49;
			this->p50 = source.p50;
			this->p51 = source.p51;
			this->p52 = source.p52;
			this->p53 = source.p53;
			this->p54 = source.p54;
			this->p55 = source.p55;
			this->p56 = source.p56;
			this->p57 = source.p57;
			this->p58 = source.p58;
			this->p59 = source.p59;
			this->p60 = source.p60;
			this->p61 = source.p61;
			this->p62 = source.p62;
			this->p63 = source.p63;
			this->p64 = source.p64;
			this->p65 = source.p65;
			this->p66 = source.p66;
			this->p67 = source.p67;
			this->p68 = source.p68;
			this->p69 = source.p69;
			this->p70 = source.p70;
			this->p71 = source.p71;
			this->p72 = source.p72;
			this->p73 = source.p73;
			this->p74 = source.p74;
			this->p75 = source.p75;
			this->p76 = source.p76;
			this->p77 = source.p77;
			this->p78 = source.p78;
			this->p79 = source.p79;
			this->p80 = source.p80;
			this->p81 = source.p81;
			this->p82 = source.p82;
			this->p83 = source.p83;
			this->p84 = source.p84;
			this->p85 = source.p85;
			this->p86 = source.p86;
			this->p87 = source.p87;
			this->p88 = source.p88;
			this->p89 = source.p89;
			this->p90 = source.p90;
			this->p91 = source.p91;
			this->p92 = source.p92;
			this->p93 = source.p93;
			this->p94 = source.p94;
			this->p95 = source.p95;
			this->p96 = source.p96;
			this->p97 = source.p97;
			this->p98 = source.p98;
			this->p99 = source.p99;
			this->p100 = source.p100;
			this->p101 = source.p101;
			this->p102 = source.p102;
			this->p103 = source.p103;
			this->p104 = source.p104;
			this->p105 = source.p105;
			this->p106 = source.p106;
			this->p107 = source.p107;
			this->p108 = source.p108;
			this->p109 = source.p109;
			this->p110 = source.p110;
			this->p111 = source.p111;
			this->p112 = source.p112;
			this->p113 = source.p113;
			this->p114 = source.p114;
			this->p115 = source.p115;
			this->p116 = source.p116;
			this->p117 = source.p117;
			this->p118 = source.p118;
			this->p119 = source.p119;
			this->p120 = source.p120;
			this->p121 = source.p121;
			this->p122 = source.p122;
			this->p123 = source.p123;
			this->p124 = source.p124;
			this->p125 = source.p125;
			this->p126 = source.p126;
			this->p127 = source.p127;
			this->p128 = source.p128;
			this->p129 = source.p129;
			this->p130 = source.p130;
			this->p131 = source.p131;
			this->p132 = source.p132;
			this->p133 = source.p133;
			this->p134 = source.p134;
			this->p135 = source.p135;
			this->p136 = source.p136;
			this->p137 = source.p137;
			this->p138 = source.p138;
			this->p139 = source.p139;
			this->p140 = source.p140;
			this->p141 = source.p141;
			this->p142 = source.p142;
			this->p143 = source.p143;
			this->p144 = source.p144;
			this->p145 = source.p145;
			this->p146 = source.p146;
			this->p147 = source.p147;
			this->p148 = source.p148;
			this->p149 = source.p149;
			this->p150 = source.p150;
			this->p151 = source.p151;
			this->p152 = source.p152;
			this->p153 = source.p153;
			this->p154 = source.p154;
			this->p155 = source.p155;
			this->p156 = source.p156;
			this->p157 = source.p157;
			this->p158 = source.p158;
			this->p159 = source.p159;
			this->p160 = source.p160;
			this->p161 = source.p161;
			this->p162 = source.p162;
			this->p163 = source.p163;
			this->p164 = source.p164;
			this->p165 = source.p165;
			this->p166 = source.p166;
			this->p167 = source.p167;
			this->p168 = source.p168;
			this->p169 = source.p169;
			this->p170 = source.p170;
			this->p171 = source.p171;
			this->p172 = source.p172;
			this->p173 = source.p173;
			this->p174 = source.p174;
			this->p175 = source.p175;
			this->p176 = source.p176;
			this->p177 = source.p177;
			this->p178 = source.p178;
			this->p179 = source.p179;
			this->p180 = source.p180;
			this->p181 = source.p181;
			this->p182 = source.p182;
			this->p183 = source.p183;
			this->p184 = source.p184;
			this->p185 = source.p185;
			this->p186 = source.p186;
			this->p187 = source.p187;
			this->p188 = source.p188;
			this->p189 = source.p189;
			this->p190 = source.p190;
			this->p191 = source.p191;
			this->p192 = source.p192;
			this->p193 = source.p193;
			this->p194 = source.p194;
			this->p195 = source.p195;
			this->p196 = source.p196;
			this->p197 = source.p197;
			this->p198 = source.p198;
			this->p199 = source.p199;
			this->p200 = source.p200;
			this->p201 = source.p201;
			this->p202 = source.p202;
			this->p203 = source.p203;
			this->p204 = source.p204;
			this->p205 = source.p205;
			this->p206 = source.p206;
			this->p207 = source.p207;
			this->p208 = source.p208;
			this->p209 = source.p209;
			this->p210 = source.p210;
			this->p211 = source.p211;
			this->p212 = source.p212;
			this->p213 = source.p213;
			this->p214 = source.p214;
			this->p215 = source.p215;

			return *this;
		}

		std::string Hits2008ScreeningProb::getId() const
		{
			return h1_hhid;
		}

		double Hits2008ScreeningProb::getP1() const
		{
			return p1;
		}


		double Hits2008ScreeningProb::getP2() const
		{
			return p2;
		}


		double Hits2008ScreeningProb::getP3() const
		{
			return p3;
		}


		double Hits2008ScreeningProb::getP4() const
		{
			return p4;
		}


		double Hits2008ScreeningProb::getP5() const
		{
			return p5;
		}


		double Hits2008ScreeningProb::getP6() const
		{
			return p6;
		}


		double Hits2008ScreeningProb::getP7() const
		{
			return p7;
		}


		double Hits2008ScreeningProb::getP8() const
		{
			return p8;
		}


		double Hits2008ScreeningProb::getP9() const
		{
			return p9;
		}


		double Hits2008ScreeningProb::getP10() const
		{
			return p10;
		}


		double Hits2008ScreeningProb::getP11() const
		{
			return p11;
		}


		double Hits2008ScreeningProb::getP12() const
		{
			return p12;
		}


		double Hits2008ScreeningProb::getP13() const
		{
			return p13;
		}


		double Hits2008ScreeningProb::getP14() const
		{
			return p14;
		}


		double Hits2008ScreeningProb::getP15() const
		{
			return p15;
		}


		double Hits2008ScreeningProb::getP16() const
		{
			return p16;
		}


		double Hits2008ScreeningProb::getP17() const
		{
			return p17;
		}


		double Hits2008ScreeningProb::getP18() const
		{
			return p18;
		}


		double Hits2008ScreeningProb::getP19() const
		{
			return p19;
		}


		double Hits2008ScreeningProb::getP20() const
		{
			return p20;
		}


		double Hits2008ScreeningProb::getP21() const
		{
			return p21;
		}


		double Hits2008ScreeningProb::getP22() const
		{
			return p22;
		}


		double Hits2008ScreeningProb::getP23() const
		{
			return p23;
		}


		double Hits2008ScreeningProb::getP24() const
		{
			return p24;
		}


		double Hits2008ScreeningProb::getP25() const
		{
			return p25;
		}


		double Hits2008ScreeningProb::getP26() const
		{
			return p26;
		}


		double Hits2008ScreeningProb::getP27() const
		{
			return p27;
		}


		double Hits2008ScreeningProb::getP28() const
		{
			return p28;
		}


		double Hits2008ScreeningProb::getP29() const
		{
			return p29;
		}


		double Hits2008ScreeningProb::getP30() const
		{
			return p30;
		}


		double Hits2008ScreeningProb::getP31() const
		{
			return p31;
		}


		double Hits2008ScreeningProb::getP32() const
		{
			return p32;
		}


		double Hits2008ScreeningProb::getP33() const
		{
			return p33;
		}


		double Hits2008ScreeningProb::getP34() const
		{
			return p34;
		}


		double Hits2008ScreeningProb::getP35() const
		{
			return p35;
		}


		double Hits2008ScreeningProb::getP36() const
		{
			return p36;
		}


		double Hits2008ScreeningProb::getP37() const
		{
			return p37;
		}


		double Hits2008ScreeningProb::getP38() const
		{
			return p38;
		}


		double Hits2008ScreeningProb::getP39() const
		{
			return p39;
		}


		double Hits2008ScreeningProb::getP40() const
		{
			return p40;
		}


		double Hits2008ScreeningProb::getP41() const
		{
			return p41;
		}


		double Hits2008ScreeningProb::getP42() const
		{
			return p42;
		}


		double Hits2008ScreeningProb::getP43() const
		{
			return p43;
		}


		double Hits2008ScreeningProb::getP44() const
		{
			return p44;
		}


		double Hits2008ScreeningProb::getP45() const
		{
			return p45;
		}


		double Hits2008ScreeningProb::getP46() const
		{
			return p46;
		}


		double Hits2008ScreeningProb::getP47() const
		{
			return p47;
		}


		double Hits2008ScreeningProb::getP48() const
		{
			return p48;
		}


		double Hits2008ScreeningProb::getP49() const
		{
			return p49;
		}


		double Hits2008ScreeningProb::getP50() const
		{
			return p50;
		}


		double Hits2008ScreeningProb::getP51() const
		{
			return p51;
		}


		double Hits2008ScreeningProb::getP52() const
		{
			return p52;
		}


		double Hits2008ScreeningProb::getP53() const
		{
			return p53;
		}


		double Hits2008ScreeningProb::getP54() const
		{
			return p54;
		}


		double Hits2008ScreeningProb::getP55() const
		{
			return p55;
		}


		double Hits2008ScreeningProb::getP56() const
		{
			return p56;
		}


		double Hits2008ScreeningProb::getP57() const
		{
			return p57;
		}


		double Hits2008ScreeningProb::getP58() const
		{
			return p58;
		}


		double Hits2008ScreeningProb::getP59() const
		{
			return p59;
		}


		double Hits2008ScreeningProb::getP60() const
		{
			return p60;
		}


		double Hits2008ScreeningProb::getP61() const
		{
			return p61;
		}


		double Hits2008ScreeningProb::getP62() const
		{
			return p62;
		}


		double Hits2008ScreeningProb::getP63() const
		{
			return p63;
		}


		double Hits2008ScreeningProb::getP64() const
		{
			return p64;
		}


		double Hits2008ScreeningProb::getP65() const
		{
			return p65;
		}


		double Hits2008ScreeningProb::getP66() const
		{
			return p66;
		}


		double Hits2008ScreeningProb::getP67() const
		{
			return p67;
		}


		double Hits2008ScreeningProb::getP68() const
		{
			return p68;
		}


		double Hits2008ScreeningProb::getP69() const
		{
			return p69;
		}


		double Hits2008ScreeningProb::getP70() const
		{
			return p70;
		}


		double Hits2008ScreeningProb::getP71() const
		{
			return p71;
		}


		double Hits2008ScreeningProb::getP72() const
		{
			return p72;
		}


		double Hits2008ScreeningProb::getP73() const
		{
			return p73;
		}


		double Hits2008ScreeningProb::getP74() const
		{
			return p74;
		}


		double Hits2008ScreeningProb::getP75() const
		{
			return p75;
		}


		double Hits2008ScreeningProb::getP76() const
		{
			return p76;
		}


		double Hits2008ScreeningProb::getP77() const
		{
			return p77;
		}


		double Hits2008ScreeningProb::getP78() const
		{
			return p78;
		}


		double Hits2008ScreeningProb::getP79() const
		{
			return p79;
		}


		double Hits2008ScreeningProb::getP80() const
		{
			return p80;
		}


		double Hits2008ScreeningProb::getP81() const
		{
			return p81;
		}


		double Hits2008ScreeningProb::getP82() const
		{
			return p82;
		}


		double Hits2008ScreeningProb::getP83() const
		{
			return p83;
		}


		double Hits2008ScreeningProb::getP84() const
		{
			return p84;
		}


		double Hits2008ScreeningProb::getP85() const
		{
			return p85;
		}


		double Hits2008ScreeningProb::getP86() const
		{
			return p86;
		}


		double Hits2008ScreeningProb::getP87() const
		{
			return p87;
		}


		double Hits2008ScreeningProb::getP88() const
		{
			return p88;
		}


		double Hits2008ScreeningProb::getP89() const
		{
			return p89;
		}


		double Hits2008ScreeningProb::getP90() const
		{
			return p90;
		}


		double Hits2008ScreeningProb::getP91() const
		{
			return p91;
		}


		double Hits2008ScreeningProb::getP92() const
		{
			return p92;
		}


		double Hits2008ScreeningProb::getP93() const
		{
			return p93;
		}


		double Hits2008ScreeningProb::getP94() const
		{
			return p94;
		}


		double Hits2008ScreeningProb::getP95() const
		{
			return p95;
		}


		double Hits2008ScreeningProb::getP96() const
		{
			return p96;
		}


		double Hits2008ScreeningProb::getP97() const
		{
			return p97;
		}


		double Hits2008ScreeningProb::getP98() const
		{
			return p98;
		}


		double Hits2008ScreeningProb::getP99() const
		{
			return p99;
		}


		double Hits2008ScreeningProb::getP100() const
		{
			return p100;
		}


		double Hits2008ScreeningProb::getP101() const
		{
			return p101;
		}


		double Hits2008ScreeningProb::getP102() const
		{
			return p102;
		}


		double Hits2008ScreeningProb::getP103() const
		{
			return p103;
		}


		double Hits2008ScreeningProb::getP104() const
		{
			return p104;
		}


		double Hits2008ScreeningProb::getP105() const
		{
			return p105;
		}


		double Hits2008ScreeningProb::getP106() const
		{
			return p106;
		}


		double Hits2008ScreeningProb::getP107() const
		{
			return p107;
		}


		double Hits2008ScreeningProb::getP108() const
		{
			return p108;
		}


		double Hits2008ScreeningProb::getP109() const
		{
			return p109;
		}


		double Hits2008ScreeningProb::getP110() const
		{
			return p110;
		}


		double Hits2008ScreeningProb::getP111() const
		{
			return p111;
		}


		double Hits2008ScreeningProb::getP112() const
		{
			return p112;
		}


		double Hits2008ScreeningProb::getP113() const
		{
			return p113;
		}


		double Hits2008ScreeningProb::getP114() const
		{
			return p114;
		}


		double Hits2008ScreeningProb::getP115() const
		{
			return p115;
		}


		double Hits2008ScreeningProb::getP116() const
		{
			return p116;
		}


		double Hits2008ScreeningProb::getP117() const
		{
			return p117;
		}


		double Hits2008ScreeningProb::getP118() const
		{
			return p118;
		}


		double Hits2008ScreeningProb::getP119() const
		{
			return p119;
		}


		double Hits2008ScreeningProb::getP120() const
		{
			return p120;
		}


		double Hits2008ScreeningProb::getP121() const
		{
			return p121;
		}


		double Hits2008ScreeningProb::getP122() const
		{
			return p122;
		}


		double Hits2008ScreeningProb::getP123() const
		{
			return p123;
		}


		double Hits2008ScreeningProb::getP124() const
		{
			return p124;
		}


		double Hits2008ScreeningProb::getP125() const
		{
			return p125;
		}


		double Hits2008ScreeningProb::getP126() const
		{
			return p126;
		}


		double Hits2008ScreeningProb::getP127() const
		{
			return p127;
		}


		double Hits2008ScreeningProb::getP128() const
		{
			return p128;
		}


		double Hits2008ScreeningProb::getP129() const
		{
			return p129;
		}


		double Hits2008ScreeningProb::getP130() const
		{
			return p130;
		}


		double Hits2008ScreeningProb::getP131() const
		{
			return p131;
		}


		double Hits2008ScreeningProb::getP132() const
		{
			return p132;
		}


		double Hits2008ScreeningProb::getP133() const
		{
			return p133;
		}


		double Hits2008ScreeningProb::getP134() const
		{
			return p134;
		}


		double Hits2008ScreeningProb::getP135() const
		{
			return p135;
		}


		double Hits2008ScreeningProb::getP136() const
		{
			return p136;
		}


		double Hits2008ScreeningProb::getP137() const
		{
			return p137;
		}


		double Hits2008ScreeningProb::getP138() const
		{
			return p138;
		}


		double Hits2008ScreeningProb::getP139() const
		{
			return p139;
		}


		double Hits2008ScreeningProb::getP140() const
		{
			return p140;
		}


		double Hits2008ScreeningProb::getP141() const
		{
			return p141;
		}


		double Hits2008ScreeningProb::getP142() const
		{
			return p142;
		}


		double Hits2008ScreeningProb::getP143() const
		{
			return p143;
		}


		double Hits2008ScreeningProb::getP144() const
		{
			return p144;
		}


		double Hits2008ScreeningProb::getP145() const
		{
			return p145;
		}


		double Hits2008ScreeningProb::getP146() const
		{
			return p146;
		}


		double Hits2008ScreeningProb::getP147() const
		{
			return p147;
		}


		double Hits2008ScreeningProb::getP148() const
		{
			return p148;
		}


		double Hits2008ScreeningProb::getP149() const
		{
			return p149;
		}


		double Hits2008ScreeningProb::getP150() const
		{
			return p150;
		}


		double Hits2008ScreeningProb::getP151() const
		{
			return p151;
		}


		double Hits2008ScreeningProb::getP152() const
		{
			return p152;
		}


		double Hits2008ScreeningProb::getP153() const
		{
			return p153;
		}


		double Hits2008ScreeningProb::getP154() const
		{
			return p154;
		}


		double Hits2008ScreeningProb::getP155() const
		{
			return p155;
		}


		double Hits2008ScreeningProb::getP156() const
		{
			return p156;
		}


		double Hits2008ScreeningProb::getP157() const
		{
			return p157;
		}


		double Hits2008ScreeningProb::getP158() const
		{
			return p158;
		}


		double Hits2008ScreeningProb::getP159() const
		{
			return p159;
		}


		double Hits2008ScreeningProb::getP160() const
		{
			return p160;
		}


		double Hits2008ScreeningProb::getP161() const
		{
			return p161;
		}


		double Hits2008ScreeningProb::getP162() const
		{
			return p162;
		}


		double Hits2008ScreeningProb::getP163() const
		{
			return p163;
		}


		double Hits2008ScreeningProb::getP164() const
		{
			return p164;
		}


		double Hits2008ScreeningProb::getP165() const
		{
			return p165;
		}


		double Hits2008ScreeningProb::getP166() const
		{
			return p166;
		}


		double Hits2008ScreeningProb::getP167() const
		{
			return p167;
		}


		double Hits2008ScreeningProb::getP168() const
		{
			return p168;
		}


		double Hits2008ScreeningProb::getP169() const
		{
			return p169;
		}


		double Hits2008ScreeningProb::getP170() const
		{
			return p170;
		}


		double Hits2008ScreeningProb::getP171() const
		{
			return p171;
		}


		double Hits2008ScreeningProb::getP172() const
		{
			return p172;
		}


		double Hits2008ScreeningProb::getP173() const
		{
			return p173;
		}


		double Hits2008ScreeningProb::getP174() const
		{
			return p174;
		}


		double Hits2008ScreeningProb::getP175() const
		{
			return p175;
		}


		double Hits2008ScreeningProb::getP176() const
		{
			return p176;
		}


		double Hits2008ScreeningProb::getP177() const
		{
			return p177;
		}


		double Hits2008ScreeningProb::getP178() const
		{
			return p178;
		}


		double Hits2008ScreeningProb::getP179() const
		{
			return p179;
		}


		double Hits2008ScreeningProb::getP180() const
		{
			return p180;
		}


		double Hits2008ScreeningProb::getP181() const
		{
			return p181;
		}


		double Hits2008ScreeningProb::getP182() const
		{
			return p182;
		}


		double Hits2008ScreeningProb::getP183() const
		{
			return p183;
		}


		double Hits2008ScreeningProb::getP184() const
		{
			return p184;
		}


		double Hits2008ScreeningProb::getP185() const
		{
			return p185;
		}


		double Hits2008ScreeningProb::getP186() const
		{
			return p186;
		}


		double Hits2008ScreeningProb::getP187() const
		{
			return p187;
		}


		double Hits2008ScreeningProb::getP188() const
		{
			return p188;
		}


		double Hits2008ScreeningProb::getP189() const
		{
			return p189;
		}


		double Hits2008ScreeningProb::getP190() const
		{
			return p190;
		}


		double Hits2008ScreeningProb::getP191() const
		{
			return p191;
		}


		double Hits2008ScreeningProb::getP192() const
		{
			return p192;
		}


		double Hits2008ScreeningProb::getP193() const
		{
			return p193;
		}


		double Hits2008ScreeningProb::getP194() const
		{
			return p194;
		}


		double Hits2008ScreeningProb::getP195() const
		{
			return p195;
		}


		double Hits2008ScreeningProb::getP196() const
		{
			return p196;
		}


		double Hits2008ScreeningProb::getP197() const
		{
			return p197;
		}


		double Hits2008ScreeningProb::getP198() const
		{
			return p198;
		}


		double Hits2008ScreeningProb::getP199() const
		{
			return p199;
		}


		double Hits2008ScreeningProb::getP200() const
		{
			return p200;
		}


		double Hits2008ScreeningProb::getP201() const
		{
			return p201;
		}


		double Hits2008ScreeningProb::getP202() const
		{
			return p202;
		}


		double Hits2008ScreeningProb::getP203() const
		{
			return p203;
		}


		double Hits2008ScreeningProb::getP204() const
		{
			return p204;
		}


		double Hits2008ScreeningProb::getP205() const
		{
			return p205;
		}


		double Hits2008ScreeningProb::getP206() const
		{
			return p206;
		}


		double Hits2008ScreeningProb::getP207() const
		{
			return p207;
		}


		double Hits2008ScreeningProb::getP208() const
		{
			return p208;
		}


		double Hits2008ScreeningProb::getP209() const
		{
			return p209;
		}


		double Hits2008ScreeningProb::getP210() const
		{
			return p210;
		}


		double Hits2008ScreeningProb::getP211() const
		{
			return p211;
		}


		double Hits2008ScreeningProb::getP212() const
		{
			return p212;
		}


		double Hits2008ScreeningProb::getP213() const
		{
			return p213;
		}


		double Hits2008ScreeningProb::getP214() const
		{
			return p214;
		}


		double Hits2008ScreeningProb::getP215() const
		{
			return p215;
		}


        std::ostream& operator<<(std::ostream& strm, const Hits2008ScreeningProb& data)
        {
            return strm << "{"
						<< "\"id \":\"" 		<< data.h1_hhid	<< "\","
						<< "\"p1 \":\"" 		<< data.p1 		<< "\","
						<< "\"p2 \":\"" 		<< data.p2 		<< "\","
						<< "\"p3 \":\"" 		<< data.p3 		<< "\","
						<< "\"p4 \":\"" 		<< data.p4 		<< "\","
						<< "\"p5 \":\"" 		<< data.p5 		<< "\","
						<< "\"p6 \":\"" 		<< data.p6 		<< "\","
						<< "\"p7 \":\"" 		<< data.p7 		<< "\","
						<< "\"p8 \":\"" 		<< data.p8 		<< "\","
						<< "\"p9 \":\"" 		<< data.p9 		<< "\","
						<< "\"p10 \":\"" 		<< data.p10 		<< "\","
						<< "\"p11 \":\"" 		<< data.p11 		<< "\","
						<< "\"p12 \":\"" 		<< data.p12 		<< "\","
						<< "\"p13 \":\"" 		<< data.p13 		<< "\","
						<< "\"p14 \":\"" 		<< data.p14 		<< "\","
						<< "\"p15 \":\"" 		<< data.p15 		<< "\","
						<< "\"p16 \":\"" 		<< data.p16 		<< "\","
						<< "\"p17 \":\"" 		<< data.p17 		<< "\","
						<< "\"p18 \":\"" 		<< data.p18 		<< "\","
						<< "\"p19 \":\"" 		<< data.p19 		<< "\","
						<< "\"p20 \":\"" 		<< data.p20 		<< "\","
						<< "\"p21 \":\"" 		<< data.p21 		<< "\","
						<< "\"p22 \":\"" 		<< data.p22 		<< "\","
						<< "\"p23 \":\"" 		<< data.p23 		<< "\","
						<< "\"p24 \":\"" 		<< data.p24 		<< "\","
						<< "\"p25 \":\"" 		<< data.p25 		<< "\","
						<< "\"p26 \":\"" 		<< data.p26 		<< "\","
						<< "\"p27 \":\"" 		<< data.p27 		<< "\","
						<< "\"p28 \":\"" 		<< data.p28 		<< "\","
						<< "\"p29 \":\"" 		<< data.p29 		<< "\","
						<< "\"p30 \":\"" 		<< data.p30 		<< "\","
						<< "\"p31 \":\"" 		<< data.p31 		<< "\","
						<< "\"p32 \":\"" 		<< data.p32 		<< "\","
						<< "\"p33 \":\"" 		<< data.p33 		<< "\","
						<< "\"p34 \":\"" 		<< data.p34 		<< "\","
						<< "\"p35 \":\"" 		<< data.p35 		<< "\","
						<< "\"p36 \":\"" 		<< data.p36 		<< "\","
						<< "\"p37 \":\"" 		<< data.p37 		<< "\","
						<< "\"p38 \":\"" 		<< data.p38 		<< "\","
						<< "\"p39 \":\"" 		<< data.p39 		<< "\","
						<< "\"p40 \":\"" 		<< data.p40 		<< "\","
						<< "\"p41 \":\"" 		<< data.p41 		<< "\","
						<< "\"p42 \":\"" 		<< data.p42 		<< "\","
						<< "\"p43 \":\"" 		<< data.p43 		<< "\","
						<< "\"p44 \":\"" 		<< data.p44 		<< "\","
						<< "\"p45 \":\"" 		<< data.p45 		<< "\","
						<< "\"p46 \":\"" 		<< data.p46 		<< "\","
						<< "\"p47 \":\"" 		<< data.p47 		<< "\","
						<< "\"p48 \":\"" 		<< data.p48 		<< "\","
						<< "\"p49 \":\"" 		<< data.p49 		<< "\","
						<< "\"p50 \":\"" 		<< data.p50 		<< "\","
						<< "\"p51 \":\"" 		<< data.p51 		<< "\","
						<< "\"p52 \":\"" 		<< data.p52 		<< "\","
						<< "\"p53 \":\"" 		<< data.p53 		<< "\","
						<< "\"p54 \":\"" 		<< data.p54 		<< "\","
						<< "\"p55 \":\"" 		<< data.p55 		<< "\","
						<< "\"p56 \":\"" 		<< data.p56 		<< "\","
						<< "\"p57 \":\"" 		<< data.p57 		<< "\","
						<< "\"p58 \":\"" 		<< data.p58 		<< "\","
						<< "\"p59 \":\"" 		<< data.p59 		<< "\","
						<< "\"p60 \":\"" 		<< data.p60 		<< "\","
						<< "\"p61 \":\"" 		<< data.p61 		<< "\","
						<< "\"p62 \":\"" 		<< data.p62 		<< "\","
						<< "\"p63 \":\"" 		<< data.p63 		<< "\","
						<< "\"p64 \":\"" 		<< data.p64 		<< "\","
						<< "\"p65 \":\"" 		<< data.p65 		<< "\","
						<< "\"p66 \":\"" 		<< data.p66 		<< "\","
						<< "\"p67 \":\"" 		<< data.p67 		<< "\","
						<< "\"p68 \":\"" 		<< data.p68 		<< "\","
						<< "\"p69 \":\"" 		<< data.p69 		<< "\","
						<< "\"p70 \":\"" 		<< data.p70 		<< "\","
						<< "\"p71 \":\"" 		<< data.p71 		<< "\","
						<< "\"p72 \":\"" 		<< data.p72 		<< "\","
						<< "\"p73 \":\"" 		<< data.p73 		<< "\","
						<< "\"p74 \":\"" 		<< data.p74 		<< "\","
						<< "\"p75 \":\"" 		<< data.p75 		<< "\","
						<< "\"p76 \":\"" 		<< data.p76 		<< "\","
						<< "\"p77 \":\"" 		<< data.p77 		<< "\","
						<< "\"p78 \":\"" 		<< data.p78 		<< "\","
						<< "\"p79 \":\"" 		<< data.p79 		<< "\","
						<< "\"p80 \":\"" 		<< data.p80 		<< "\","
						<< "\"p81 \":\"" 		<< data.p81 		<< "\","
						<< "\"p82 \":\"" 		<< data.p82 		<< "\","
						<< "\"p83 \":\"" 		<< data.p83 		<< "\","
						<< "\"p84 \":\"" 		<< data.p84 		<< "\","
						<< "\"p85 \":\"" 		<< data.p85 		<< "\","
						<< "\"p86 \":\"" 		<< data.p86 		<< "\","
						<< "\"p87 \":\"" 		<< data.p87 		<< "\","
						<< "\"p88 \":\"" 		<< data.p88 		<< "\","
						<< "\"p89 \":\"" 		<< data.p89 		<< "\","
						<< "\"p90 \":\"" 		<< data.p90 		<< "\","
						<< "\"p91 \":\"" 		<< data.p91 		<< "\","
						<< "\"p92 \":\"" 		<< data.p92 		<< "\","
						<< "\"p93 \":\"" 		<< data.p93 		<< "\","
						<< "\"p94 \":\"" 		<< data.p94 		<< "\","
						<< "\"p95 \":\"" 		<< data.p95 		<< "\","
						<< "\"p96 \":\"" 		<< data.p96 		<< "\","
						<< "\"p97 \":\"" 		<< data.p97 		<< "\","
						<< "\"p98 \":\"" 		<< data.p98 		<< "\","
						<< "\"p99 \":\"" 		<< data.p99 		<< "\","
						<< "\"p100 \":\"" 		<< data.p100 		<< "\","
						<< "\"p101 \":\"" 		<< data.p101 		<< "\","
						<< "\"p102 \":\"" 		<< data.p102 		<< "\","
						<< "\"p103 \":\"" 		<< data.p103 		<< "\","
						<< "\"p104 \":\"" 		<< data.p104 		<< "\","
						<< "\"p105 \":\"" 		<< data.p105 		<< "\","
						<< "\"p106 \":\"" 		<< data.p106 		<< "\","
						<< "\"p107 \":\"" 		<< data.p107 		<< "\","
						<< "\"p108 \":\"" 		<< data.p108 		<< "\","
						<< "\"p109 \":\"" 		<< data.p109 		<< "\","
						<< "\"p110 \":\"" 		<< data.p110 		<< "\","
						<< "\"p111 \":\"" 		<< data.p111 		<< "\","
						<< "\"p112 \":\"" 		<< data.p112 		<< "\","
						<< "\"p113 \":\"" 		<< data.p113 		<< "\","
						<< "\"p114 \":\"" 		<< data.p114 		<< "\","
						<< "\"p115 \":\"" 		<< data.p115 		<< "\","
						<< "\"p116 \":\"" 		<< data.p116 		<< "\","
						<< "\"p117 \":\"" 		<< data.p117 		<< "\","
						<< "\"p118 \":\"" 		<< data.p118 		<< "\","
						<< "\"p119 \":\"" 		<< data.p119 		<< "\","
						<< "\"p120 \":\"" 		<< data.p120 		<< "\","
						<< "\"p121 \":\"" 		<< data.p121 		<< "\","
						<< "\"p122 \":\"" 		<< data.p122 		<< "\","
						<< "\"p123 \":\"" 		<< data.p123 		<< "\","
						<< "\"p124 \":\"" 		<< data.p124 		<< "\","
						<< "\"p125 \":\"" 		<< data.p125 		<< "\","
						<< "\"p126 \":\"" 		<< data.p126 		<< "\","
						<< "\"p127 \":\"" 		<< data.p127 		<< "\","
						<< "\"p128 \":\"" 		<< data.p128 		<< "\","
						<< "\"p129 \":\"" 		<< data.p129 		<< "\","
						<< "\"p130 \":\"" 		<< data.p130 		<< "\","
						<< "\"p131 \":\"" 		<< data.p131 		<< "\","
						<< "\"p132 \":\"" 		<< data.p132 		<< "\","
						<< "\"p133 \":\"" 		<< data.p133 		<< "\","
						<< "\"p134 \":\"" 		<< data.p134 		<< "\","
						<< "\"p135 \":\"" 		<< data.p135 		<< "\","
						<< "\"p136 \":\"" 		<< data.p136 		<< "\","
						<< "\"p137 \":\"" 		<< data.p137 		<< "\","
						<< "\"p138 \":\"" 		<< data.p138 		<< "\","
						<< "\"p139 \":\"" 		<< data.p139 		<< "\","
						<< "\"p140 \":\"" 		<< data.p140 		<< "\","
						<< "\"p141 \":\"" 		<< data.p141 		<< "\","
						<< "\"p142 \":\"" 		<< data.p142 		<< "\","
						<< "\"p143 \":\"" 		<< data.p143 		<< "\","
						<< "\"p144 \":\"" 		<< data.p144 		<< "\","
						<< "\"p145 \":\"" 		<< data.p145 		<< "\","
						<< "\"p146 \":\"" 		<< data.p146 		<< "\","
						<< "\"p147 \":\"" 		<< data.p147 		<< "\","
						<< "\"p148 \":\"" 		<< data.p148 		<< "\","
						<< "\"p149 \":\"" 		<< data.p149 		<< "\","
						<< "\"p150 \":\"" 		<< data.p150 		<< "\","
						<< "\"p151 \":\"" 		<< data.p151 		<< "\","
						<< "\"p152 \":\"" 		<< data.p152 		<< "\","
						<< "\"p153 \":\"" 		<< data.p153 		<< "\","
						<< "\"p154 \":\"" 		<< data.p154 		<< "\","
						<< "\"p155 \":\"" 		<< data.p155 		<< "\","
						<< "\"p156 \":\"" 		<< data.p156 		<< "\","
						<< "\"p157 \":\"" 		<< data.p157 		<< "\","
						<< "\"p158 \":\"" 		<< data.p158 		<< "\","
						<< "\"p159 \":\"" 		<< data.p159 		<< "\","
						<< "\"p160 \":\"" 		<< data.p160 		<< "\","
						<< "\"p161 \":\"" 		<< data.p161 		<< "\","
						<< "\"p162 \":\"" 		<< data.p162 		<< "\","
						<< "\"p163 \":\"" 		<< data.p163 		<< "\","
						<< "\"p164 \":\"" 		<< data.p164 		<< "\","
						<< "\"p165 \":\"" 		<< data.p165 		<< "\","
						<< "\"p166 \":\"" 		<< data.p166 		<< "\","
						<< "\"p167 \":\"" 		<< data.p167 		<< "\","
						<< "\"p168 \":\"" 		<< data.p168 		<< "\","
						<< "\"p169 \":\"" 		<< data.p169 		<< "\","
						<< "\"p170 \":\"" 		<< data.p170 		<< "\","
						<< "\"p171 \":\"" 		<< data.p171 		<< "\","
						<< "\"p172 \":\"" 		<< data.p172 		<< "\","
						<< "\"p173 \":\"" 		<< data.p173 		<< "\","
						<< "\"p174 \":\"" 		<< data.p174 		<< "\","
						<< "\"p175 \":\"" 		<< data.p175 		<< "\","
						<< "\"p176 \":\"" 		<< data.p176 		<< "\","
						<< "\"p177 \":\"" 		<< data.p177 		<< "\","
						<< "\"p178 \":\"" 		<< data.p178 		<< "\","
						<< "\"p179 \":\"" 		<< data.p179 		<< "\","
						<< "\"p180 \":\"" 		<< data.p180 		<< "\","
						<< "\"p181 \":\"" 		<< data.p181 		<< "\","
						<< "\"p182 \":\"" 		<< data.p182 		<< "\","
						<< "\"p183 \":\"" 		<< data.p183 		<< "\","
						<< "\"p184 \":\"" 		<< data.p184 		<< "\","
						<< "\"p185 \":\"" 		<< data.p185 		<< "\","
						<< "\"p186 \":\"" 		<< data.p186 		<< "\","
						<< "\"p187 \":\"" 		<< data.p187 		<< "\","
						<< "\"p188 \":\"" 		<< data.p188 		<< "\","
						<< "\"p189 \":\"" 		<< data.p189 		<< "\","
						<< "\"p190 \":\"" 		<< data.p190 		<< "\","
						<< "\"p191 \":\"" 		<< data.p191 		<< "\","
						<< "\"p192 \":\"" 		<< data.p192 		<< "\","
						<< "\"p193 \":\"" 		<< data.p193 		<< "\","
						<< "\"p194 \":\"" 		<< data.p194 		<< "\","
						<< "\"p195 \":\"" 		<< data.p195 		<< "\","
						<< "\"p196 \":\"" 		<< data.p196 		<< "\","
						<< "\"p197 \":\"" 		<< data.p197 		<< "\","
						<< "\"p198 \":\"" 		<< data.p198 		<< "\","
						<< "\"p199 \":\"" 		<< data.p199 		<< "\","
						<< "\"p200 \":\"" 		<< data.p200 		<< "\","
						<< "\"p201 \":\"" 		<< data.p201 		<< "\","
						<< "\"p202 \":\"" 		<< data.p202 		<< "\","
						<< "\"p203 \":\"" 		<< data.p203 		<< "\","
						<< "\"p204 \":\"" 		<< data.p204 		<< "\","
						<< "\"p205 \":\"" 		<< data.p205 		<< "\","
						<< "\"p206 \":\"" 		<< data.p206 		<< "\","
						<< "\"p207 \":\"" 		<< data.p207 		<< "\","
						<< "\"p208 \":\"" 		<< data.p208 		<< "\","
						<< "\"p209 \":\"" 		<< data.p209 		<< "\","
						<< "\"p210 \":\"" 		<< data.p210 		<< "\","
						<< "\"p211 \":\"" 		<< data.p211 		<< "\","
						<< "\"p212 \":\"" 		<< data.p212 		<< "\","
						<< "\"p213 \":\"" 		<< data.p213 		<< "\","
						<< "\"p214 \":\"" 		<< data.p214 		<< "\","
						<< "\"p215 \":\"" 		<< data.p215 		<< "\""
						<< "}";
        }
	}
} /* namespace sim_mob */
