//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Types.h
 * Author: Pedro Gandola
 *
 * Created on Feb 27, 2014, 2:04 PM
 */

#pragma once

#include <vector>

namespace sim_mob {
    namespace long_term {
        
        /**
         * Deletes a pointer from a vector of pointers.
         * @param src vector of pointers.
         * @param value, pointer to be deleted.
         */
        template <typename T>
        void deleteValue(std::vector<T*>& src, T*& value) {
            if (value) {
                typename std::vector<T*>::iterator it;
                for (it = src.begin(); it != src.end(); it++) {
                    if ((*it) && value == (*it)) {
                        delete *it;
                        src.erase(it);
                        value = nullptr;
                        return;
                    }
                }
            }
        }
        
        /**
         * Copies a vector of Pointers to another vector of pointers.
         * This **ONLY** replicates pointers.
         * This method does not allocate new memory.
         * @param src vector of pointers.
         * @param dst vector to copy.
         */
        template <typename T>
        void copy(std::vector<T*>& src, std::vector<T*>& dst) {
            typename std::vector<T*>::iterator it;
            for (it = src.begin(); it != src.end(); it++) {
                dst.push_back((*it));
            }
        }
        
        /**
         * Copies a vector of Pointers to another vector of pointers.
         * This **ONLY** replicates pointers.
         * This method does not allocate new memory.
         * @param src vector of pointers.
         * @param dst vector to copy.
         */
        template <typename T>
        void copy(std::vector<T*>& src, std::vector<const T*>& dst) {
            typename std::vector<T*>::iterator it;
            for (it = src.begin(); it != src.end(); it++) {
                dst.push_back((*it));
            }
        }
    }
}



