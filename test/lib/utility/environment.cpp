/*
 * environment.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::environment class
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


// ------------------------------------------------------------
// incs

#include <iostream>
#include <fstream>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code

int test() {
    
    boost::filesystem::path p;
    
    p = qkd::utility::environment::config_path();
    std::cout << "       config path: " << p.string() << std::endl;

    p = qkd::utility::environment::current_path();
    std::cout << "      current path: " << p.string() << std::endl;

    p = qkd::utility::environment::data_path("test");
    std::cout << "  data (test) path: " << p.string() << std::endl;

    p = qkd::utility::environment::home_path();
    std::cout << "         home path: " << p.string() << std::endl;

    p = qkd::utility::environment::prefix_path();
    std::cout << "       prefix path: " << p.string() << std::endl;

    unsigned int nPid = qkd::utility::environment::process_id();
    std::cout << "        process id: " << nPid << std::endl;

    p = qkd::utility::environment::process_image_path();
    std::cout << "process_image path: " << p.string() << std::endl;
    
    // find some files
    p = qkd::utility::environment::find_path("/bin/ls");
    assert(!p.empty());
    
    std::ofstream f("find_file_test");
    f << "find_file_test";
    f.close();
    
    p = qkd::utility::environment::find_path("find_file_test");
    assert(!p.empty());
    
    // iterate over the network interfaces
    std::map<std::string, qkd::utility::nic> cNics = qkd::utility::environment::nics();
    for (auto const & cNic : cNics) {
        
        // the NIC name
        std::cout << "               nic: " << cNic.second.sName << " " <<
            "ipv4: " << cNic.second.sIPv4 << " " <<
            "ipv6: " << cNic.second.sIPv6 << std::endl;
    }
    
    // look up the default gateway
    qkd::utility::nic cDefaultGateway = qkd::utility::environment::default_gateway();
    
    // the NIC name
    std::cout << "   default gateway: " << cDefaultGateway.sName << " " <<
        "ipv4: " << cDefaultGateway.sIPv4 << " " <<
        "ipv6: " << cDefaultGateway.sIPv6 << std::endl;
        
    // host lookups
    std::set<std::string> cAddresses;
    std::cout << "         localhost: " << std::endl;
    cAddresses = qkd::utility::environment::host_lookup("localhost");
    for (auto const & sAddress : cAddresses) {
        std::cout << "                    " << sAddress << std::endl;
    }
    std::cout << "         127.0.0.1: " << std::endl;
    cAddresses = qkd::utility::environment::host_lookup("127.0.0.1");
    for (auto const & sAddress : cAddresses) {
        std::cout << "                    " << sAddress << std::endl;
    }
    std::cout << "             alice: " << std::endl;
    cAddresses = qkd::utility::environment::host_lookup("alice");
    for (auto const & sAddress : cAddresses) {
        std::cout << "                    " << sAddress << std::endl;
    }
    std::cout << "               bob: " << std::endl;
    cAddresses = qkd::utility::environment::host_lookup("bob");
    for (auto const & sAddress : cAddresses) {
        std::cout << "                    " << sAddress << std::endl;
    }
    std::cout << "    www.google.com: " << std::endl;
    cAddresses = qkd::utility::environment::host_lookup("www.google.com");
    for (auto const & sAddress : cAddresses) {
        std::cout << "                    " << sAddress << std::endl;
    }


        

    return 0;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}

