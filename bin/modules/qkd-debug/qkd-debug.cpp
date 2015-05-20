/*
 * qkd-debug.cpp
 * 
 * This is the implementation code for the qkd-debug module.
 * 
 * The qkd-debug QKD Module dumps human readable information about
 * the bypassing key
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


// ------------------------------------------------------------
// incs

#include <atomic>
#include <fstream>
#include <iostream>

#include <boost/format.hpp>

// ait
#include <qkd/utility/syslog.h>

#include "qkd-debug.h"
#include "qkd_debug_dbus.h"


// ------------------------------------------------------------
// defs


#define MODULE_DESCRIPTION      "This is the qkd-debug QKD Module: it write human readable output to a file."
#define MODULE_ORGANISATION     "(C)opyright 2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-debug pimpl
 */
class qkd_debug::qkd_debug_data {
    
public:

    
    /**
     * ctor
     */
    qkd_debug_data() : bTryToOpen(true) {};
    
    std::recursive_mutex cPropertyMutex;    /**< property mutex */
    std::string sFileURL;                   /**< file URL */
    std::atomic<bool> bTryToOpen;           /**< if true, try to open file again */
    
    std::ofstream cKeyFile;                 /**< the key file */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_debug::qkd_debug() : qkd::module::module("debug", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_debug::qkd_debug_data>(new qkd_debug::qkd_debug_data());
    
    // enforce DBus registration
    new DebugAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_debug::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        
        // module specific config here
        if (sKey == "file_url") {
            set_file_url(QString::fromStdString(cEntry.second));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * return the file URL to write
 * 
 * @return  the file URL to write to
 */
QString qkd_debug::file_url() const {
    
    // get exclusive access to properties
    std::unique_lock<std::recursive_mutex> cLock(d->cPropertyMutex);
    return QString::fromStdString(d->sFileURL);
}


/**
 * module work
 * 
 * @param   cKey                    will be set to the loaded key from the file
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_debug::process(qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {
    
    // do not process NULL keys
    if (cKey == qkd::key::key::null()) return false;

    // check if our input stream is open
    if (!d->cKeyFile.is_open() && d->bTryToOpen) {
        
        // do not try again
        d->bTryToOpen = false;
        
        // only proceed if we DO have a file to write to
        if (!d->sFileURL.empty()) {
        
            // get exclusive access to properties
            std::unique_lock<std::recursive_mutex> cLock(d->cPropertyMutex);
            
            // check URL
            QUrl cURL(QString::fromStdString(d->sFileURL));
            if (!cURL.isLocalFile()) {
                qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "'" << d->sFileURL << "' seems not to point to a local file - wont proceed";
                return true;
            }
            else {
                
                // open file
                d->cKeyFile.open(cURL.toLocalFile().toStdString());
                if (!d->cKeyFile.is_open()) {
                    qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to open file '" << d->sFileURL << "'";
                    return true;
                }
            }
        }
    }
    
    // create output
    std::stringstream ss;
    
    uint64_t nBits = cKey.size() * 8;
    double nDisclosedBitsRate = (double)cKey.meta().nDisclosedBits / (double)nBits;
    
    ss << "key #" << cKey.id() << "\n";
    ss << "\tbits:                \t" << nBits << "\n";
    ss << "\tdisclosed bits:      \t" << cKey.meta().nDisclosedBits << " (" << boost::format("%05.2f") % (nDisclosedBitsRate * 100.0) << "%)\n";
    ss << "\terror bits:          \t" << cKey.meta().nErrorBits << "\n";
    ss << "\terror rate:          \t" << cKey.meta().nErrorRate << "\n";
    ss << "\tauth-scheme-incoming:\t" << cKey.meta().sCryptoSchemeIncoming << "\n";
    ss << "\tauth-scheme-outgoing:\t" << cKey.meta().sCryptoSchemeOutgoing << "\n";
    ss << "\tstate:               \t" << cKey.state_string() << "\n";
    
    // checksum
    ss << "\tcrc32:               \t" << cKey.data().crc32() << "\n";
        
    // write information about the key
    if (d->cKeyFile.is_open()) d->cKeyFile << ss.str();
    else std::cerr << ss.str();

    return true;
}


/**
 * sets the new file URL to write
 * 
 * @param   sFileURL        the new file URL to write to
 */
void qkd_debug::set_file_url(QString sFileURL) {
    
    // get exclusive access to properties
    std::unique_lock<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // close already opened file
    if (d->cKeyFile.is_open()) d->cKeyFile.close();
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "copying input keys to: '" << sFileURL.toStdString() << "'";    
    d->sFileURL = sFileURL.toStdString();
    d->bTryToOpen = true;
}

