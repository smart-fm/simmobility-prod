/*
 * JobsBySectorByTaz.hpp
 *
 *  Created on: 26 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class JobsByIndustryTypeByTaz
        {
        public:
            JobsByIndustryTypeByTaz(BigSerial tazId = INVALID_ID, int industryType1 = 0, int industryType2 = 0, int industryType3 = 0, int industryType4 = 0, int industryType5 = 0, int industryType6 = 0, int industryType7 = 0,int industryType8 = 0,
                                int industryType9 = 0, int industryType10 = 0, int industryType11 = 0, int industryType98 = 0);
            virtual ~JobsByIndustryTypeByTaz();


            JobsByIndustryTypeByTaz& operator=(const JobsByIndustryTypeByTaz& source);

            /*
             * Getters and Setters
             */
            int getIndustryType1() const;
            void setIndustryType1(int indType1);

            int getIndustryType10() const;
            void setIndustryType10(int indType10);

            int getIndustryType11() const;
            void setIndustryType11(int indType11);

            int getIndustryType2() const;
            void setIndustryType2(int indType2);

            int getIndustryType3() const;
            void setIndustryType3(int indType3);

            int getIndustryType4() const;
            void setIndustryType4(int indType4);

            int getIndustryType5() const;
            void setIndustryType5(int indType5);

            int getIndustryType6() const;
            void setIndustryType6(int indType6);

            int getIndustryType7() const;
            void setIndustryType7(int indType7);

            int getIndustryType8() const;
            void setIndustryType8(int indType8);

            int getIndustryType9() const;
            void setIndustryType9(int indType9);

            int getIndustryType98() const;
            void setIndustryType98(int indType98);

            BigSerial getTazId() const;
            void setTazId(BigSerial tazId);

        private:

            BigSerial tazId;
            int industryType1;
            int industryType2;
            int industryType3;
            int industryType4;
            int industryType5;
            int industryType6;
            int industryType7;
            int industryType8;
            int industryType9;
            int industryType10;
            int industryType11;
            int industryType98;

        };

    }
    }
