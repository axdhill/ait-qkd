/*
 * main.cpp
 * 
 * This is a sample AIT QKD Module
 * 
 * This module just simply pust "Hello World!" as key to 
 * stdout and quits. This is totally nonsense but gives a hint
 * to very, very low requirments in building a module.
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

// get Qt
#include <QtCore/QCoreApplication>

// get QKD
#include <qkd/qkd.h>


// ------------------------------------------------------------
// the module

class HelloWorld_QKD : public qkd::module::module {
    
public:
    
    /**
     * constructor
     * 
     * this is to setup the module's identity
     */
    HelloWorld_QKD() : qkd::module::module("hello-world", 
                                           qkd::module::module_type::TYPE_OTHER, 
                                           "This is example module #1: 'Hello World!' as a new key.", 
                                           "Place in here your organisation/company.") {};

private:
    
    
    /**
     * module work
     * 
     * This method is the heart of a QKD module. It operates on a new
     * key. If the input is void, then this method is called at once from
     * within the framework for a new key.
     * 
     * However, this is the "Hello World!" QKD Module and as such, we generate
     * a QKD key with exact this content: Hello World!
     * 
     * =)
     * 
     * @param   cKey                    the key just read from the input pipe
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  true, if the key was successfully processed (which here is always the case)
     */
    bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
        
        if (keys_outgoing() > 0) { 
            
            // at least we had a key already: terminate module!
            std::cout.flush();
            terminate();
            return false;
        }
        
        // the new key is "Hello World!" ...
        qkd::utility::buffer buf;
        buf << std::string("Hello World!");
        cKey.data() = buf;

        return true;
    };
                                           
};


// ------------------------------------------------------------
// code


/**
 * startup
 * 
 * @param   argc        as usual ...
 * @param   argv        as usual ...
 * @return  as usual ...
 */
int main(int argc, char ** argv) {
    
    // get up Qt
    QCoreApplication cApp(argc, argv);
    
    // get up the module
    HelloWorld_QKD hello_world;
    
    // terminate the Qt application if module has finished
    cApp.connect(&hello_world, SIGNAL(terminated()), SLOT(quit()));
    
    // run the module with the proper input and output
    hello_world.set_urls("", "stdout://", "", "");
    hello_world.run();
    
    // start the module later, when all subsystems are on
    hello_world.start_later();

    // run Qt
    int nAppExit = cApp.exec();
    
    // wait until module thread settled
    hello_world.join();

    return nAppExit;
}
