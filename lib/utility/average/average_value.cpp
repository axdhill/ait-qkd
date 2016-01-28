/*
 * average_value.cpp
 * 
 * moving average over values implementation
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

// ait
#include <qkd/utility/average.h>

#include "average_data.h"
#include "average_value.h"

using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * add a value to the average calculation
 * 
 * @param   nValue      value to add
 */
void average_value::add_internal(double nValue) {
    m_nSum += nValue;
    data().push_back(average_data_ptr(new average_data(nValue)));
    trim();
}


/**
 * get the current average value as average
 *
 * @return  the current average value as average
 */
double average_value::avg_internal() const {
    if (!data().size()) return 0.0;
    return m_nSum / (double)data().size();
}


/**
 * get the current average value as sum
 *
 * @return  the current average value as sum
 */
double average_value::sum_internal() const {
    return m_nSum;
}


/**
 * trim the data according to the window size
 */
void average_value::trim() const {
    auto & cDataList = data();
    while ((cDataList.size()) && (cDataList.size() > window())) {
        m_nSum -= cDataList.front()->value();
        cDataList.pop_front();
    }
}
