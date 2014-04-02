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
#include <boost/unordered_map.hpp>
#include "database/DB_Connection.hpp"
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
         * Deletes all dynamically allocated objects within the given map.
         * @param src map of pointers.
         */
        template <typename K, typename T>
        void deleteAll(boost::unordered_map<K, T*>& src) {
            typename boost::unordered_map<K, T*>::iterator it;
            for (it = src.begin(); it != src.end(); it++) {
                safe_delete_item(it->second);
            }
            src.clear();
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
         * Copies a vector of Pointers to another vector of constant pointers.
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

        /**
         * Copies a map of pointers to a vector of constant pointers.
         * This **ONLY** replicates pointers.
         * This method does not allocate new memory.
         * @param src map of pointers.
         * @param dst vector to copy.
         */
        template <typename K, typename T>
        void copy(boost::unordered_map<K, T*>& src, std::vector<const T*>& dst) {
            typename boost::unordered_map<K, T*>::iterator it;
            for (it = src.begin(); it != src.end(); it++) {
                dst.push_back(it->second);
            }
        }

        /**
         * Load data from datasouce from given connection using the 
         * given list and template DAO.
         * @param conn Datasource connection.
         * @param list (out) to fill.
         */
        template <typename T, typename K>
        inline void loadData(db::DB_Connection& conn, K& list) {
            if (conn.isConnected()) {
                T dao(conn);
                dao.getAll(list);
            }
        }

        /**
         * Load data from datasouce from given connection using the 
         * given list and template DAO.
         * This function fills the given map using the given getter function. 
         * 
         * Maps should be like map<KEY, *Obj> 
         *    - KEY object returned by given getter function.
         *    - *Obj pointer to the loaded object. 
         * 
         * @param conn Datasource connection.
         * @param list (out) to fill.
         * @param map (out) to fill.
         * @param getter function pointer to get the map KEY.
         */
        template <typename T, typename K, typename M, typename F>
        inline void loadData(db::DB_Connection& conn, K& list, M& map, F getter) {
            loadData<T>(conn, list);
            //Index all buildings.
            for (typename K::iterator it = list.begin(); it != list.end(); it++) {
                map.insert(std::make_pair(((*it)->*getter)(), *it));
            }
        }
    }
}



