/*
 * PrimarySchool.hpp
 *
 *  Created on: 10 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include "Individual.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class Individual;
		class PrimarySchool
		{
		public:
			PrimarySchool(BigSerial schoolId = INVALID_ID, BigSerial postcode = INVALID_ID, double centroidX = 0, double centroidY = 0, std::string schoolName = EMPTY_STR, int giftedProgram = false, int sapProgram = false, std::string dgp = EMPTY_STR, BigSerial tazId = INVALID_ID, int numStudents = 0 );
			virtual ~PrimarySchool();

			PrimarySchool(const PrimarySchool& source);
			PrimarySchool& operator=(const PrimarySchool& source);

			struct DistanceIndividual
			{
				BigSerial individualId;
				double distanceToSchool;
			};

			struct OrderByDistance
			{
				bool operator ()( const DistanceIndividual &a, const DistanceIndividual &b ) const
				{
					return a.distanceToSchool < b.distanceToSchool;
				}
			};

			struct OrderByProbability
			{
				bool operator ()( const PrimarySchool *a, const PrimarySchool *b ) const
				{
					return a->reAllocationProb > b->reAllocationProb;
				}
			};

			double getCentroidX() const;
			double getCentroidY() const;
			const std::string& getDgp() const;
			int isGiftedProgram() const;
			BigSerial getPostcode() const;
			int isSapProgram() const;
			BigSerial getSchoolId() const;
			const std::string& getSchoolName() const;
			BigSerial getTazId() const;
			int getNumStudents() const;
			std::vector<PrimarySchool::DistanceIndividual> getSortedDistanceIndList();
			std::vector<Individual*> getStudents();
			std::vector<BigSerial> getSelectedStudents();
			int getNumStudentsCanBeAssigned();
			double getReAllocationProb();
			int getNumOfSelectedStudents();

			void setCentroidX(double centroidX);
			void setCentroidY(double centroidY);
			void setDgp(const std::string& dgp);
			void setGiftedProgram(int giftedProgram);
			void setPostcode(BigSerial postcode);
			void setSapProgram(int sapProgram);
			void setSchoolId(BigSerial schoolId);
			void setSchoolName(const std::string& schoolName);
			void setTazId(BigSerial tazId);
			void addStudent(Individual *student);
			void addIndividualDistance(DistanceIndividual &distanceIndividual);
			void setSelectedStudentList(std::vector<BigSerial>&selectedStudentsList);
			void setNumStudentsCanBeAssigned(int numStudents);
			void setReAllocationProb(double probability);
			void addSelectedStudent(BigSerial individualId);


		private:
			friend class PrimarySchoolDao;

			BigSerial schoolId;
			BigSerial postcode;
			double centroidX;
			double centroidY;
			std::string schoolName;
			int giftedProgram;
			int sapProgram;
			std::string dgp;
			BigSerial tazId;
			int numStudents;
			std::vector<Individual*> students;
			std::vector<BigSerial> selectedStudents;
			std::vector<PrimarySchool::DistanceIndividual> distanceIndList;
			int numStudentsCanBeAssigned;
			double reAllocationProb;
		};
	}

}


