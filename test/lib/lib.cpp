/*
 * lib.cpp
 *
 * This is a test file.
 *
 * TEST: check if we can include the main header and link to the library
 * 
 *       this is done be calling the version function.
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

#if defined(__GNUC__) || defined(__GNUCPP__)
#   define UNUSED   __attribute__((unused))
#else
#   define UNUSED
#endif

#include <iostream>

// include the all-in-one header
#include <qkd/qkd.h>


int test() {
    
    qkd_version_9_9999();
    qkd_version_9_9999_1();
    qkd_version_9_9999_2();
    qkd_version_9_9999_3();
    qkd_version_9_9999_4();
    qkd_version_9_9999_5();
    qkd_version_9_9999_6();
    qkd_version_9_9999_7();
    
    std::cout << qkd::version() << std::endl;
    
    return 0;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}

