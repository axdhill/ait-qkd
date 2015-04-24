/*
 * terminate_module.cpp
 * 
 * This is a test file.
 *
 * TEST: this the terminate_module functionality: terminate after an amount of keys processed
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2015 AIT Austrian Institute of Technology
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
 * The terminate_module test the terminatation after an amount of keys
 */
class terminate_module : public qkd::module::module {
    
    
public:


    /**
     * ctor
     */
    terminate_module() : qkd::module::module(
        "config", 
        qkd::module::module_type::TYPE_OTHER, 
        "This is a terminate qkd module: it terminate after an amount of processed keys.", 
        "(C)opyright 2015, AIT Austrian Institute of Technology, http://www.ait.ac.at") {};

    
private:
    
    
    /**
     * module work
     * 
     * @param   cKey                    the key just read from the input pipe
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  true, if the key is to be pushed to the output pipe
     */
    bool process(UNUSED qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) { 

        // flip a coin to forward key or not
        // for the test: this return value must not make a change
        char nRandom = 0;
        (*random()) >> nRandom;
        bool bForwardKey = ((nRandom & 0x01) != 0);

        qkd::utility::debug() << "key forward: " << bForwardKey;

        return bForwardKey; 
    };
    
};


/**
 * startup
 * 
 * @param   argc        as usual ...
 * @param   argv        as usual ...
 * @return  as usual ...
 */
int main(int argc, char * * argv) {
    
    // enable debug
    qkd::utility::debug::enabled() = true;

    QCoreApplication cApp(argc, argv);
    
    // instantiate module
    terminate_module cTerminateModule;

    // terminate after 10 keys
    cTerminateModule.set_terminate_after(10);
    cTerminateModule.set_urls("", "", "", "");
    cTerminateModule.start_later();
    
    // terminate if module has finished
    cApp.connect(&cTerminateModule, SIGNAL(terminated()), SLOT(quit()));
    
    // run Qt
    int nAppExit = cApp.exec();
    
    // join worker thread (cleanup)
    cTerminateModule.join();
    
    return nAppExit;
}

