/* 
 * File:   Types.h
 * Author: Pedro Gandola
 *
 * Created on April 4, 2013, 2:04 PM
 */

#pragma once

#include "stddef.h"
#include "util/LangHelpers.hpp"

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


typedef int UnitId;

