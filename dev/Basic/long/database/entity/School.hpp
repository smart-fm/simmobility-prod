/*
 * School.hpp
 *
 *  Created on: 7 Nov 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class School
		{
		public:
			School(BigSerial id = INVALID_ID, BigSerial fmBuildingId = INVALID_ID,double floorArea = 0, int schoolSlot = 0, double centroidX = 0, double centroidY = 0, bool giftedProgram = false, bool sapProgram = false,
				   std::string planningArea = std::string(), BigSerial tazName = INVALID_ID, bool primarySchool = false, bool preSchool = false);
			virtual ~School();

			struct DistanceIndividual
			{
				BigSerial individualId;
				double distanceToSchool;
			};

			struct OrderByDistance
			{
				bool operator ()( const DistanceIndividual *a, const DistanceIndividual *b ) const

				{
					return a->distanceToSchool < b->distanceToSchool;
				}
			};

			struct OrderByProbability
			{
				bool operator ()( const School *a, const School *b ) const
				{
					return a->reAllocationProb > b->reAllocationProb;
				}
			};

			double getCentroidX() const;
			void setCentroidX(double centroidX);
			double getCentroidY() const;
			void setCentroidY(double centroidY);
			double getFloorArea() const;
			void setFloorArea(double floorArea);
			BigSerial getFmBuildingId() const;
			void setFmBuildingId(BigSerial fmBuildingId);
			bool isGiftedProgram() const;
			void setGiftedProgram(bool giftedProgram);
			BigSerial getId() const;
			void setId(BigSerial id);
			std::string getPlanningArea() const;
			void setPlanningArea(std::string planningArea);
			bool isPreSchool() const ;
			void setPreSchool(bool preSchool);
			bool isPrimarySchool() const;
			void setPrimarySchool(bool primarySchool);
			bool isSapProgram() const;
			void setSapProgram(bool sapProgram);
			int getSchoolSlot() const;
			void setSchoolSlot(int schoolSlot);
			BigSerial getTazName() const;
			void setTazName(BigSerial tazName);

			int getNumStudents() const;
			std::vector<School::DistanceIndividual*> getSortedDistanceIndList();
			std::vector<BigSerial*> getStudents();
			int getNumSelectedStudents();
			int getNumStudentsCanBeAssigned();
			double getReAllocationProb();
			int getNumOfSelectedStudents();
			std::vector<School*> getSortedProbSchoolList(std::vector<School*> studentsWithProb);

			void addStudent(BigSerial *studentId);
			void addIndividualDistance(DistanceIndividual *distanceIndividual);
			void setSelectedStudentList(std::vector<BigSerial*>selectedStudents);
			void setNumStudentsCanBeAssigned(int numStudents);
			void setReAllocationProb(double probability);
			void addSelectedStudent(BigSerial *individualId);

			BigSerial id;
			BigSerial fmBuildingId;
			double floorArea;
			int schoolSlot;
			double centroidX;
			double centroidY;
			bool giftedProgram;
			bool sapProgram;
			std::string planningArea;
			BigSerial tazName;
			bool primarySchool;
			bool preSchool;

			int numStudents;
			std::vector<BigSerial*> students;
			std::vector<BigSerial*> selectedStudents;
			std::vector<School::DistanceIndividual*> distanceIndList;
			int numStudentsCanBeAssigned;
			double reAllocationProb;


		};
	}

}

