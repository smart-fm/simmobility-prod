/* 
 * File:   Types.h
 * Author: Pedro Gandola
 *
 * Created on April 4, 2013, 2:04 PM
 */

#pragma once

#include <string>
#include "stddef.h"
#include "util/LangHelpers.hpp"

typedef long long BigSerial;
typedef BigSerial UnitId;

enum Sex {
    UNKNOWN_SEX = 0,
    MASCULINE = 1,
    FEMININE = 2
};

enum Race {
    UNKNOWN_RACE = 0,
    CHINISE = 1,
    MALAY = 2,
    INDIAN = 3,
    OTHER = 4,
};

enum EmploymentStatus {
    UNKNOWN_STATUS = -1,
    UNEMPLOYED = 0,
    EMPLOYED = 1
};

enum TimeUnit {
    DAILY = 0,
    WEEKLY = 1,
    MONTHLY = 2,
    YEARLY = 3
};

enum UnitType {
    ROOM_1 = 0,
    ROOM_2 = 1,
    ROOM_3 = 2,
    ROOM_4 = 3,
    ROOM_5 = 4,
    EXECUTIVE = 5,
    UNKNOWN_UNIT_TYPE
};

static Race ToRace(int value) {
    switch (value) {
        case CHINISE: return CHINISE;
        case MALAY: return MALAY;
        case INDIAN: return INDIAN;
        case OTHER: return OTHER;
        default: return UNKNOWN_RACE;
    }
}

static Sex ToSex(int value) {
    switch (value) {
        case MASCULINE: return MASCULINE;
        case FEMININE: return FEMININE;
        default: return UNKNOWN_SEX;
    }
}

static EmploymentStatus ToEmploymentStatus(int value) {
    switch (value) {
        case UNEMPLOYED: return UNEMPLOYED;
        case EMPLOYED: return EMPLOYED;
        default: return UNKNOWN_STATUS;
    }
}

static TimeUnit ToTimeUnit(int value) {
    switch (value) {
        case DAILY: return DAILY;
        case WEEKLY: return WEEKLY;
        case MONTHLY: return MONTHLY;
        case YEARLY: return YEARLY;
        default: return DAILY;
    }
}

static UnitType ToUnitType(int value) {
    switch (value) {
        case ROOM_1: return ROOM_1;
        case ROOM_2: return ROOM_2;
        case ROOM_3: return ROOM_3;
        case ROOM_4: return ROOM_4;
        case ROOM_5: return ROOM_5;
        case EXECUTIVE: return EXECUTIVE;
        default: return UNKNOWN_UNIT_TYPE;
    }
}