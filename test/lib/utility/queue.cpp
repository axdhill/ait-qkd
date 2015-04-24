/*
 * queue.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::queue class
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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

#include <fstream>
#include <iostream>
#include <string>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


int test() {
    
    // create a queue
    qkd::utility::queue<std::string> cQueue;
    
    // check for emptyness
    assert(cQueue.empty());
    assert(cQueue.size() == 0);
    
    cQueue.push("One");
    cQueue.push("Two");
    cQueue.push("Three");
    
    // check for emptyness
    assert(!cQueue.empty());
    assert(cQueue.size() == 3);
    
    std::string s1 = cQueue.pop();
    assert(s1 == "One");
    assert(!cQueue.empty());
    assert(cQueue.size() == 2);

    std::string s2 = cQueue.pop();
    assert(s2 == "Two");
    assert(!cQueue.empty());
    assert(cQueue.size() == 1);

    std::string s3 = cQueue.pop();
    assert(s3 == "Three");
    assert(cQueue.empty());
    assert(cQueue.size() == 0);

    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}

