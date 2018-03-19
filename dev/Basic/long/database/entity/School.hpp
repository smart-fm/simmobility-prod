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
				   std::string planningArea = std::string(), BigSerial tazName = INVALID_ID, std::string schoolType = std::string(), bool artProgram = false, bool musicProgram = false, bool langProgram = false, bool expressTest = false
				   ,double studentDensity = 0,int numStudents = 0,int studentLimit = 0);
			virtual ~School();

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
			bool isSapProgram() const;
			void setSapProgram(bool sapProgram);
			int getSchoolSlot() const;
			void setSchoolSlot(int schoolSlot);
			BigSerial getTazName() const;
			void setTazName(BigSerial tazName);
			std::string getSchoolType() const;
			void setSchoolType(std::string schoolType);

			int getNumStudents() const;
			std::vector<School::DistanceIndividual> getSortedDistanceIndList();
			std::vector<BigSerial> getStudents();
			std::vector<BigSerial> getSelectedStudents();
			int getNumSelectedStudents();
			int getNumStudentsCanBeAssigned();
			double getReAllocationProb();
			int getNumOfSelectedStudents();
			std::vector<School*> getSortedProbSchoolList(std::vector<School*> studentsWithProb);
			bool isArtProgram() const;
			void setArtProgram(bool artProgram);
			bool isLangProgram() const;
			void setLangProgram(bool langProgram);
			bool isMusicProgram() const ;
			void setMusicProgram(bool musicProgram);
			bool isExpressTest() const;
			void setExpressTest(bool expressTest);
			int getStudentLimit() const;
			void setStudentLimit(int studentLimit);
			double getStudentDensity() const;
			void setStudentDensity(double studentDensity);


			void addStudent(BigSerial studentId);
			void addIndividualDistance(DistanceIndividual &distanceIndividual);
			void setSelectedStudentList(std::vector<BigSerial>selectedStudents);
			void setNumStudentsCanBeAssigned(int numStudents);
			void setReAllocationProb(double probability);
			void addSelectedStudent(BigSerial individualId);

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
			std::string schoolType;
			bool artProgram;
			bool musicProgram;
			bool langProgram;
			bool expressTest;
			double studentDensity;

			int numStudents;
			std::vector<BigSerial> students;
			std::vector<BigSerial> selectedStudents;
			std::vector<School::DistanceIndividual> distanceIndList;
			int numStudentsCanBeAssigned;
			double reAllocationProb;
			int studentLimit;

		};
	}

}

