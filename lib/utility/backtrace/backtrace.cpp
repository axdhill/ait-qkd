/*
 * backtrace.cpp
 * 
 * implements functions in backtrace.h
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

 
// ------------------------------------------------------------
// incs

#include <iostream>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


#ifdef __linux__
#   include <execinfo.h>
#endif

// ait
#include <qkd/utility/backtrace.h>


using namespace qkd::utility;


// ------------------------------------------------------------
// decl


namespace qkd {

/**
 * this is the global instance of the real backtrace
 */
class internal_backtrace : public backtrace {

public:

    /**
     * ctor
     */
    internal_backtrace();


    /**
     * the dump trace signal handler
     *
     * @param   nSignal     the signal forwarded by the OS
     */
    static void dump_trace(int nSignal);

};

}


// ------------------------------------------------------------
// vars


/**
 * by declaring this global variable the ctor is run, which sets
 * up the backtrace signal handler
 */
qkd::internal_backtrace g_cInternalBacktrace;


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd::internal_backtrace::internal_backtrace() {
    signal(SIGSEGV, internal_backtrace::dump_trace);
}


/**
 * the dump trace signal handler
 *
 * @param   nSignal     the signal forwarded by the OS
 */
void qkd::internal_backtrace::dump_trace(int nSignal) {

    // get the stack
    void * cBuffer[255];
    const int nCalls = ::backtrace(cBuffer, 255); 

    // dump stack
    std::cerr << "\n\n**** WOHA! WOHA! WOHA! Something nasty has happend!! =( ****\nDumping backtrace. The lines below might come handy ...\nSorry, the AIT QDK team.\n" << std::endl;
    backtrace_symbols_fd(cBuffer, nCalls, 2);

    signal(nSignal, SIG_DFL);
    kill(getpid(), nSignal);
}

