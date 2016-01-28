/*
 * debug.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::debug class
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2016 AIT Austrian Institute of Technology
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


// ------------------------------------------------------------
// incs

#include <iostream>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// vars


/**
 * debug counter
 */
int g_nDebugNumber = 0;


/**
 * been in debug callback flag
 */
bool g_bInCallback = false;


// ------------------------------------------------------------
// code


/**
 * debug callback
 * 
 * @param   sLine       the line currently captured
 */
void my_debug_callback(std::string const & sLine) {

    static bool bGot1 = false;
    static bool bGot2 = false;

    g_bInCallback = true;

    assert(g_nDebugNumber != 0);

    if (g_nDebugNumber == 1 && sLine == "1") bGot1 = true;
    if (g_nDebugNumber == 2 && sLine == "2") bGot2 = true;

    // first call is not allowed (global debug must be disabled by default)
    if (g_nDebugNumber == 2) assert(bGot1);
    if (g_nDebugNumber == 3) assert(bGot2);
}


int test() {

    // install callback handler
    qkd::utility::debug::set_callback(my_debug_callback);

    // first call: my not pass --> global disabled per default
    qkd::utility::debug() << g_nDebugNumber;
    g_nDebugNumber++;

    // second call: must pass --> force enabled
    qkd::utility::debug(true) << g_nDebugNumber;
    g_nDebugNumber++;

    // enable global
    qkd::utility::debug::enabled() = true;

    qkd::utility::debug(true) << g_nDebugNumber;
    g_nDebugNumber++;
    qkd::utility::debug() << g_nDebugNumber;
    g_nDebugNumber++;

    assert(g_bInCallback);

    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}

