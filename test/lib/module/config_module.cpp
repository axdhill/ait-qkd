/*
 * config_module.cpp
 * 
 * This is a test file.
 *
 * TEST: this is a test module which tests the configuration possibilities
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


/**
 * The CONFIG QKD Module tests the configuration URL handling
 */
class config_module : public qkd::module::module {
    
    
public:


    /**
     * ctor
     */
    config_module() : qkd::module::module(
        "config", 
        qkd::module::module_type::TYPE_OTHER, 
        "This is a CONFIG QKD Module: it outputs all config values given by configure() on stdout.", 
        "(C)opyright 2012, AIT Austrian Institute of Technology, http://www.ait.ac.at") {};

    
protected:
    
    
    /**
     * apply the loaded key value map to the module
     * 
     * @param   sURL            URL of config file loaded
     * @param   cConfig         map of key --> value
     */
    void apply_config(std::string const & sURL, qkd::utility::properties const & cConfig);
    

private:
    
    
    /**
     * module work
     * 
     * @param   cKey                    the key just read from the input pipe
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  true, if the key is to be pushed to the output pipe
     */
    bool process(UNUSED qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) { return true; };
    
};


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void config_module::apply_config(std::string const & sURL, qkd::utility::properties const & cConfig) {

    // from where did we get the current config
    std::cout << "configuration loaded from '" << sURL << "':" << std::endl;
    
    // print out what we have
    for (auto const & cEntry : cConfig) {
        std::cout << cEntry.first << " = " << cEntry.second << std::endl;
    }
}


/**
 * startup
 * 
 * @param   argc        as usual ...
 * @param   argv        as usual ...
 * @return  as usual ...
 */
int main(int argc, char * * argv) {
    
    QCoreApplication cApp(argc, argv);
    
    // instantiate module
    config_module cConfigModule;
    
    // terminate if module has finished
    cApp.connect(&cConfigModule, SIGNAL(terminated()), SLOT(quit()));
    
    // run Qt
    int nAppExit = cApp.exec();
    
    // join worker thread (cleanup)
    cConfigModule.join();
    
    return nAppExit;
}

