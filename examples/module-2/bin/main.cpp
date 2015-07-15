/*
 * main.cpp
 * 
 * Common startup-code for any qkd module
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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

// get the module
#include "my_module.h"


// ------------------------------------------------------------
// code


/**
 * startup
 */
int main(int argc, char ** argv) {
    
    QCoreApplication cApp(argc, argv);
    
    // init the module
    my_module MyModule;
    MyModule.set_urls("", "stdout://", "", "");
    MyModule.start_later();
    
    // run
    cApp.connect(&MyModule, SIGNAL(terminated()), SLOT(quit()));
    int nAppExit = cApp.exec();
    MyModule.join();

    return nAppExit;
}
