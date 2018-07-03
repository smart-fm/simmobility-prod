/*
 * StudyArea.hpp
 *
 *  Created on: 6 Jun 2017
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
namespace long_term
{
class StudyArea
{
public:
	StudyArea( BigSerial id = INVALID_ID,  BigSerial fmTazId = INVALID_ID, std::string studyCode = std::string());

	virtual ~StudyArea();

	/**
	 * Getters and Setters
	 */
	 BigSerial getId() const;
	 const std::string& getStudyCode() const;
	 BigSerial getFmTazId() const;

	 void setId(BigSerial id);
	 void setStudyCode(const std::string& studyCode);
	 void setFmTazId(BigSerial fmTazId);

private:

	 BigSerial id;
	 BigSerial fmTazId;
	 std::string studyCode;
};
}
}
