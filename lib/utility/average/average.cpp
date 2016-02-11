/*
 * average.cpp
 * 
 * moving average implementation
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
 * AIT Austrian Institute of Technology GmbH
 * Donau-City-Strasse 1 | 1220 Vienna | Austria
 * http://www.ait.ac.at
 *
 * This file is part of the AIT QKD Software Suite.
 *
 * The AIT QKD Software Suite is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 * 
 * The AIT QKD Software Suite is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with the AIT QKD Software Suite. 
 * If not, see <http://www.gnu.org/licenses/>.
 */


// ------------------------------------------------------------
// incs

#include <algorithm>

// ait
#include <qkd/utility/average.h>

#include "average_data.h"
#include "average_time.h"
#include "average_value.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * get the average distance in time between two consecutive values within the window
 * 
 * @return  the average distance in time between two consecutive values
 */
std::chrono::high_resolution_clock::duration average_technique::avg_distance_internal() const {
    
    std::chrono::high_resolution_clock::duration res(0);
    if (!d.size()) {
        return res;
    }
    
    std::list<std::chrono::high_resolution_clock::duration> cDistances;
    auto cLastBirth = d.front()->birth();
    std::for_each(++d.begin(), d.end(), [&](average_data_ptr const & i) { cDistances.push_back(i->birth() - cLastBirth); cLastBirth = i->birth(); });
    
    std::chrono::high_resolution_clock::duration cSumDistances(0);
    std::for_each(cDistances.begin(), cDistances.end(), [&](std::chrono::high_resolution_clock::duration const & d) { cSumDistances += d; });
    
    if (cDistances.size()) {
        res = cSumDistances / cDistances.size();
    }

    return res;
}


/**
 * factory method to create a moving average
 * 
 * @param   sTechnique      the technique of the moving average
 * @param   nWindowSize     the window size of the moving average
 * @return  an initialized random object
 * @throws  average_technique_unknown
 */
average average_technique::create(std::string sTechnique, uint64_t nWindowSize) {

    // treat different techniques
    if (sTechnique == "time") return std::shared_ptr<average_technique>(new average_time(nWindowSize));
    if (sTechnique == "value") return std::shared_ptr<average_technique>(new average_value(nWindowSize));

    throw std::invalid_argument("unknown average technique");
}


/**
 * gets the highest recorded value within the window size
 *
 * @return the highest value
 */
double average_technique::max_internal() const {
    return d.size()
           ? (* std::max_element(d.begin(), d.end(), [](average_data_ptr const & i, average_data_ptr const & j) {
                return i->value() < j->value();
            }))->value()
           : 0;
}


/**
 * gets the lowest recorded value within the window size
 *
 * @return the lowest value
 */
double average_technique::min_internal() const {
    return d.size()
           ? (* std::min_element(d.begin(), d.end(), [](average_data_ptr const & i, average_data_ptr const & j) {
                return i->value() < j->value();
            }))->value()
           : 0;
}


/**
 * get the oldest value
 * 
 * @return  the oldest value
 */
double average_technique::oldest_internal() const {
    if (d.size()) return d.front()->value();
    return 0.0;
}


/**
 * get the youngest value
 * 
 * @return  the youngest value
 */
double average_technique::youngest_internal() const {
    if (d.size()) return d.back()->value();
    return 0.0;
}
