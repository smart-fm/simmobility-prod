/*
 * JobAssignmentCoeffs.hpp
 *
 *  Created on: 25 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
    	class JobAssignmentCoeffs
		{
		public:
    		JobAssignmentCoeffs(int id = INVALID_ID, double betaInc1 = 0, double betaInc2 = 0, double betaInc3 = 0, double betaLgs = 0, double betaS1 = 0, double betaS2 = 0, double betaS3 = 0,double betaS4 = 0,
    							double betaS5 = 0, double betaS6 = 0, double betaS7 = 0, double betaS8 = 0, double betaS9 = 0, double betaS10 = 0, double betaS11 = 0, double betaS98 = 0,double betaLnJob = 0);
			virtual ~JobAssignmentCoeffs();


			JobAssignmentCoeffs& operator=(const JobAssignmentCoeffs& source);


			/*
			 * Getters and Setters
			 */
			double getBetaInc1() const;
			void setBetaInc1(double betaInc1);

			double getBetaInc2() const;
			void setBetaInc2(double betaInc2);

			double getBetaInc3() const;
			void setBetaInc3(double betaInc3);

			double getBetaLgs() const;
			void setBetaLgs(double betaLgs);

			double getBetaLnJob() const;
			void setBetaLnJob(double betaLnJob);

			double getBetaS1() const;
			void setBetaS1(double betaS1);

			double getBetaS10() const;
			void setBetaS10(double betaS10);

			double getBetaS11() const;
			void setBetaS11(double betaS11);

			double getBetaS2() const;
			void setBetaS2(double betaS2);

			double getBetaS3() const;
			void setBetaS3(double betaS3);

			double getBetaS4() const;
			void setBetaS4(double betaS4);

			double getBetaS5() const;
			void setBetaS5(double betaS5);

			double getBetaS6() const;
			void setBetaS6(double betaS6);

			double getBetaS7() const;
			void setBetaS7(double betaS7);

			double getBetaS8() const;
			void setBetaS8(double betaS8);

			double getBetaS9() const;
			void setBetaS9(double betaS9);

			double getBetaS98() const;
			void setBetaS98(double betaS98);

			int getId() const;
			void setId(int id);


		private:

			int id;
			double betaInc1;
			double betaInc2;
			double betaInc3;
			double betaLgs;
			double betaS1;
			double betaS2;
			double betaS3;
			double betaS4;
			double betaS5;
			double betaS6;
			double betaS7;
			double betaS8;
			double betaS9;
			double betaS10;
			double betaS11;
			double betaS98;
			double betaLnJob;

		};

    }
    }
