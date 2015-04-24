/*
 * null_module.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::module::module class for pure compliation
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

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


/**
 * The NULL module does nothing. Just for compilation.
 */
class null_module : public qkd::module::module {
    
    
public:


    /**
     * ctor
     */
    null_module() : qkd::module::module(
        "null", 
        qkd::module::module_type::TYPE_OTHER, 
        "This is a NULL QKD Module doing nothing.", 
        "(C)opyright 2012, AIT Austrian Institute of Technology, http://www.ait.ac.at") {};

    
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


int test() {
    
    
    return 0;
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
    
    null_module cNullModule;
    cNullModule.set_urls("", "", "", "");
    cNullModule.run();
    
    int r = cApp.exec();
    
    cNullModule.join();
    
    return r;
}

