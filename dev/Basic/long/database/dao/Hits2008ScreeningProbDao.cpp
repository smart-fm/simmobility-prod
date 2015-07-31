//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Hits2008ScreeningProbDao.cpp
 *
 * Created on: July 31, 2015
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/Hits2008ScreeningProbDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

Hits2008ScreeningProbDao::Hits2008ScreeningProbDao(DB_Connection& connection): SqlAbstractDao<Hits2008ScreeningProb>(connection, DB_TABLE_HITS2008SCREENINGPROB,"", "", "",DB_GETALL_HITS2008SCREENINGPROB, DB_GETBYID_HITS2008SCREENINGPROB){}

Hits2008ScreeningProbDao::~Hits2008ScreeningProbDao() {}

void Hits2008ScreeningProbDao::fromRow(Row& result, Hits2008ScreeningProb& outObj)
{
	outObj.id 	= result.get<BigSerial>("id",	0);
    outObj.h1_hhid 	= result.get<std::string>(	"h1_hhid",	"");
    outObj.p1 = result.get<double>(	"p1", 	0.0);
    outObj.p2 = result.get<double>(	"p2", 	0.0);
    outObj.p3 = result.get<double>(	"p3", 	0.0);
    outObj.p4 = result.get<double>(	"p4", 	0.0);
    outObj.p5 = result.get<double>(	"p5", 	0.0);
    outObj.p6 = result.get<double>(	"p6", 	0.0);
    outObj.p7 = result.get<double>(	"p7", 	0.0);
    outObj.p8 = result.get<double>(	"p8", 	0.0);
    outObj.p9 = result.get<double>(	"p9", 	0.0);
    outObj.p10 = result.get<double>(	"p10", 	0.0);
    outObj.p11 = result.get<double>(	"p11", 	0.0);
    outObj.p12 = result.get<double>(	"p12", 	0.0);
    outObj.p13 = result.get<double>(	"p13", 	0.0);
    outObj.p14 = result.get<double>(	"p14", 	0.0);
    outObj.p15 = result.get<double>(	"p15", 	0.0);
    outObj.p16 = result.get<double>(	"p16", 	0.0);
    outObj.p17 = result.get<double>(	"p17", 	0.0);
    outObj.p18 = result.get<double>(	"p18", 	0.0);
    outObj.p19 = result.get<double>(	"p19", 	0.0);
    outObj.p20 = result.get<double>(	"p20", 	0.0);
    outObj.p21 = result.get<double>(	"p21", 	0.0);
    outObj.p22 = result.get<double>(	"p22", 	0.0);
    outObj.p23 = result.get<double>(	"p23", 	0.0);
    outObj.p24 = result.get<double>(	"p24", 	0.0);
    outObj.p25 = result.get<double>(	"p25", 	0.0);
    outObj.p26 = result.get<double>(	"p26", 	0.0);
    outObj.p27 = result.get<double>(	"p27", 	0.0);
    outObj.p28 = result.get<double>(	"p28", 	0.0);
    outObj.p29 = result.get<double>(	"p29", 	0.0);
    outObj.p30 = result.get<double>(	"p30", 	0.0);
    outObj.p31 = result.get<double>(	"p31", 	0.0);
    outObj.p32 = result.get<double>(	"p32", 	0.0);
    outObj.p33 = result.get<double>(	"p33", 	0.0);
    outObj.p34 = result.get<double>(	"p34", 	0.0);
    outObj.p35 = result.get<double>(	"p35", 	0.0);
    outObj.p36 = result.get<double>(	"p36", 	0.0);
    outObj.p37 = result.get<double>(	"p37", 	0.0);
    outObj.p38 = result.get<double>(	"p38", 	0.0);
    outObj.p39 = result.get<double>(	"p39", 	0.0);
    outObj.p40 = result.get<double>(	"p40", 	0.0);
    outObj.p41 = result.get<double>(	"p41", 	0.0);
    outObj.p42 = result.get<double>(	"p42", 	0.0);
    outObj.p43 = result.get<double>(	"p43", 	0.0);
    outObj.p44 = result.get<double>(	"p44", 	0.0);
    outObj.p45 = result.get<double>(	"p45", 	0.0);
    outObj.p46 = result.get<double>(	"p46", 	0.0);
    outObj.p47 = result.get<double>(	"p47", 	0.0);
    outObj.p48 = result.get<double>(	"p48", 	0.0);
    outObj.p49 = result.get<double>(	"p49", 	0.0);
    outObj.p50 = result.get<double>(	"p50", 	0.0);
    outObj.p51 = result.get<double>(	"p51", 	0.0);
    outObj.p52 = result.get<double>(	"p52", 	0.0);
    outObj.p53 = result.get<double>(	"p53", 	0.0);
    outObj.p54 = result.get<double>(	"p54", 	0.0);
    outObj.p55 = result.get<double>(	"p55", 	0.0);
    outObj.p56 = result.get<double>(	"p56", 	0.0);
    outObj.p57 = result.get<double>(	"p57", 	0.0);
    outObj.p58 = result.get<double>(	"p58", 	0.0);
    outObj.p59 = result.get<double>(	"p59", 	0.0);
    outObj.p60 = result.get<double>(	"p60", 	0.0);
    outObj.p61 = result.get<double>(	"p61", 	0.0);
    outObj.p62 = result.get<double>(	"p62", 	0.0);
    outObj.p63 = result.get<double>(	"p63", 	0.0);
    outObj.p64 = result.get<double>(	"p64", 	0.0);
    outObj.p65 = result.get<double>(	"p65", 	0.0);
    outObj.p66 = result.get<double>(	"p66", 	0.0);
    outObj.p67 = result.get<double>(	"p67", 	0.0);
    outObj.p68 = result.get<double>(	"p68", 	0.0);
    outObj.p69 = result.get<double>(	"p69", 	0.0);
    outObj.p70 = result.get<double>(	"p70", 	0.0);
    outObj.p71 = result.get<double>(	"p71", 	0.0);
    outObj.p72 = result.get<double>(	"p72", 	0.0);
    outObj.p73 = result.get<double>(	"p73", 	0.0);
    outObj.p74 = result.get<double>(	"p74", 	0.0);
    outObj.p75 = result.get<double>(	"p75", 	0.0);
    outObj.p76 = result.get<double>(	"p76", 	0.0);
    outObj.p77 = result.get<double>(	"p77", 	0.0);
    outObj.p78 = result.get<double>(	"p78", 	0.0);
    outObj.p79 = result.get<double>(	"p79", 	0.0);
    outObj.p80 = result.get<double>(	"p80", 	0.0);
    outObj.p81 = result.get<double>(	"p81", 	0.0);
    outObj.p82 = result.get<double>(	"p82", 	0.0);
    outObj.p83 = result.get<double>(	"p83", 	0.0);
    outObj.p84 = result.get<double>(	"p84", 	0.0);
    outObj.p85 = result.get<double>(	"p85", 	0.0);
    outObj.p86 = result.get<double>(	"p86", 	0.0);
    outObj.p87 = result.get<double>(	"p87", 	0.0);
    outObj.p88 = result.get<double>(	"p88", 	0.0);
    outObj.p89 = result.get<double>(	"p89", 	0.0);
    outObj.p90 = result.get<double>(	"p90", 	0.0);
    outObj.p91 = result.get<double>(	"p91", 	0.0);
    outObj.p92 = result.get<double>(	"p92", 	0.0);
    outObj.p93 = result.get<double>(	"p93", 	0.0);
    outObj.p94 = result.get<double>(	"p94", 	0.0);
    outObj.p95 = result.get<double>(	"p95", 	0.0);
    outObj.p96 = result.get<double>(	"p96", 	0.0);
    outObj.p97 = result.get<double>(	"p97", 	0.0);
    outObj.p98 = result.get<double>(	"p98", 	0.0);
    outObj.p99 = result.get<double>(	"p99", 	0.0);
    outObj.p100 = result.get<double>(	"p100", 	0.0);
    outObj.p101 = result.get<double>(	"p101", 	0.0);
    outObj.p102 = result.get<double>(	"p102", 	0.0);
    outObj.p103 = result.get<double>(	"p103", 	0.0);
    outObj.p104 = result.get<double>(	"p104", 	0.0);
    outObj.p105 = result.get<double>(	"p105", 	0.0);
    outObj.p106 = result.get<double>(	"p106", 	0.0);
    outObj.p107 = result.get<double>(	"p107", 	0.0);
    outObj.p108 = result.get<double>(	"p108", 	0.0);
    outObj.p109 = result.get<double>(	"p109", 	0.0);
    outObj.p110 = result.get<double>(	"p110", 	0.0);
    outObj.p111 = result.get<double>(	"p111", 	0.0);
    outObj.p112 = result.get<double>(	"p112", 	0.0);
    outObj.p113 = result.get<double>(	"p113", 	0.0);
    outObj.p114 = result.get<double>(	"p114", 	0.0);
    outObj.p115 = result.get<double>(	"p115", 	0.0);
    outObj.p116 = result.get<double>(	"p116", 	0.0);
    outObj.p117 = result.get<double>(	"p117", 	0.0);
    outObj.p118 = result.get<double>(	"p118", 	0.0);
    outObj.p119 = result.get<double>(	"p119", 	0.0);
    outObj.p120 = result.get<double>(	"p120", 	0.0);
    outObj.p121 = result.get<double>(	"p121", 	0.0);
    outObj.p122 = result.get<double>(	"p122", 	0.0);
    outObj.p123 = result.get<double>(	"p123", 	0.0);
    outObj.p124 = result.get<double>(	"p124", 	0.0);
    outObj.p125 = result.get<double>(	"p125", 	0.0);
    outObj.p126 = result.get<double>(	"p126", 	0.0);
    outObj.p127 = result.get<double>(	"p127", 	0.0);
    outObj.p128 = result.get<double>(	"p128", 	0.0);
    outObj.p129 = result.get<double>(	"p129", 	0.0);
    outObj.p130 = result.get<double>(	"p130", 	0.0);
    outObj.p131 = result.get<double>(	"p131", 	0.0);
    outObj.p132 = result.get<double>(	"p132", 	0.0);
    outObj.p133 = result.get<double>(	"p133", 	0.0);
    outObj.p134 = result.get<double>(	"p134", 	0.0);
    outObj.p135 = result.get<double>(	"p135", 	0.0);
    outObj.p136 = result.get<double>(	"p136", 	0.0);
    outObj.p137 = result.get<double>(	"p137", 	0.0);
    outObj.p138 = result.get<double>(	"p138", 	0.0);
    outObj.p139 = result.get<double>(	"p139", 	0.0);
    outObj.p140 = result.get<double>(	"p140", 	0.0);
    outObj.p141 = result.get<double>(	"p141", 	0.0);
    outObj.p142 = result.get<double>(	"p142", 	0.0);
    outObj.p143 = result.get<double>(	"p143", 	0.0);
    outObj.p144 = result.get<double>(	"p144", 	0.0);
    outObj.p145 = result.get<double>(	"p145", 	0.0);
    outObj.p146 = result.get<double>(	"p146", 	0.0);
    outObj.p147 = result.get<double>(	"p147", 	0.0);
    outObj.p148 = result.get<double>(	"p148", 	0.0);
    outObj.p149 = result.get<double>(	"p149", 	0.0);
    outObj.p150 = result.get<double>(	"p150", 	0.0);
    outObj.p151 = result.get<double>(	"p151", 	0.0);
    outObj.p152 = result.get<double>(	"p152", 	0.0);
    outObj.p153 = result.get<double>(	"p153", 	0.0);
    outObj.p154 = result.get<double>(	"p154", 	0.0);
    outObj.p155 = result.get<double>(	"p155", 	0.0);
    outObj.p156 = result.get<double>(	"p156", 	0.0);
    outObj.p157 = result.get<double>(	"p157", 	0.0);
    outObj.p158 = result.get<double>(	"p158", 	0.0);
    outObj.p159 = result.get<double>(	"p159", 	0.0);
    outObj.p160 = result.get<double>(	"p160", 	0.0);
    outObj.p161 = result.get<double>(	"p161", 	0.0);
    outObj.p162 = result.get<double>(	"p162", 	0.0);
    outObj.p163 = result.get<double>(	"p163", 	0.0);
    outObj.p164 = result.get<double>(	"p164", 	0.0);
    outObj.p165 = result.get<double>(	"p165", 	0.0);
    outObj.p166 = result.get<double>(	"p166", 	0.0);
    outObj.p167 = result.get<double>(	"p167", 	0.0);
    outObj.p168 = result.get<double>(	"p168", 	0.0);
    outObj.p169 = result.get<double>(	"p169", 	0.0);
    outObj.p170 = result.get<double>(	"p170", 	0.0);
    outObj.p171 = result.get<double>(	"p171", 	0.0);
    outObj.p172 = result.get<double>(	"p172", 	0.0);
    outObj.p173 = result.get<double>(	"p173", 	0.0);
    outObj.p174 = result.get<double>(	"p174", 	0.0);
    outObj.p175 = result.get<double>(	"p175", 	0.0);
    outObj.p176 = result.get<double>(	"p176", 	0.0);
    outObj.p177 = result.get<double>(	"p177", 	0.0);
    outObj.p178 = result.get<double>(	"p178", 	0.0);
    outObj.p179 = result.get<double>(	"p179", 	0.0);
    outObj.p180 = result.get<double>(	"p180", 	0.0);
    outObj.p181 = result.get<double>(	"p181", 	0.0);
    outObj.p182 = result.get<double>(	"p182", 	0.0);
    outObj.p183 = result.get<double>(	"p183", 	0.0);
    outObj.p184 = result.get<double>(	"p184", 	0.0);
    outObj.p185 = result.get<double>(	"p185", 	0.0);
    outObj.p186 = result.get<double>(	"p186", 	0.0);
    outObj.p187 = result.get<double>(	"p187", 	0.0);
    outObj.p188 = result.get<double>(	"p188", 	0.0);
    outObj.p189 = result.get<double>(	"p189", 	0.0);
    outObj.p190 = result.get<double>(	"p190", 	0.0);
    outObj.p191 = result.get<double>(	"p191", 	0.0);
    outObj.p192 = result.get<double>(	"p192", 	0.0);
    outObj.p193 = result.get<double>(	"p193", 	0.0);
    outObj.p194 = result.get<double>(	"p194", 	0.0);
    outObj.p195 = result.get<double>(	"p195", 	0.0);
    outObj.p196 = result.get<double>(	"p196", 	0.0);
    outObj.p197 = result.get<double>(	"p197", 	0.0);
    outObj.p198 = result.get<double>(	"p198", 	0.0);
    outObj.p199 = result.get<double>(	"p199", 	0.0);
    outObj.p200 = result.get<double>(	"p200", 	0.0);
    outObj.p201 = result.get<double>(	"p201", 	0.0);
    outObj.p202 = result.get<double>(	"p202", 	0.0);
    outObj.p203 = result.get<double>(	"p203", 	0.0);
    outObj.p204 = result.get<double>(	"p204", 	0.0);
    outObj.p205 = result.get<double>(	"p205", 	0.0);
    outObj.p206 = result.get<double>(	"p206", 	0.0);
    outObj.p207 = result.get<double>(	"p207", 	0.0);
    outObj.p208 = result.get<double>(	"p208", 	0.0);
    outObj.p209 = result.get<double>(	"p209", 	0.0);
    outObj.p210 = result.get<double>(	"p210", 	0.0);
    outObj.p211 = result.get<double>(	"p211", 	0.0);
    outObj.p212 = result.get<double>(	"p212", 	0.0);
    outObj.p213 = result.get<double>(	"p213", 	0.0);
    outObj.p214 = result.get<double>(	"p214", 	0.0);
    outObj.p215 = result.get<double>(	"p215", 	0.0);
}

void Hits2008ScreeningProbDao::toRow(Hits2008ScreeningProb& data, Parameters& outParams, bool update) {}
