/*
 * qkd-dekey.cpp
 * 
 * This is the implementation for the qkd-dekey module.
 * 
 * The qkd-dekey QKD Module picks up a key-file and pushes
 * the content to pipeout.
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

// ait
#include <qkd/utility/syslog.h>

#include "qkd-dekey.h"
#include "qkd_dekey_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-dekey QKD Module: it picks up a keyfile and pushes the content to Pipe-Out."
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-dekey pimpl
 */
class qkd_dekey::qkd_dekey_data {
    
public:

    
    /**
     * ctor
     */
    qkd_dekey_data() : bTryToOpen(true) {};
    
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
qkd_dekey::qkd_dekey() : qkd::module::module("dekey", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_dekey::qkd_dekey_data>(new qkd_dekey::qkd_dekey_data());
    
    // enforce DBus registration
    new DekeyAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_dekey::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        
        // module specific config here
        if (sKey == "alice.file_url") {
            if (is_alice()) set_file_url(QString::fromStdString(cEntry.second));
        }
        else 
        if (sKey == "bob.file_url") {
            if (is_bob()) set_file_url(QString::fromStdString(cEntry.second));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * return the file URL to read
 * 
 * @return  the file URL to read from
 */
QString qkd_dekey::file_url() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
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
bool qkd_dekey::process(qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {
    
    // do not work on empty keys
    if (!cKey.size()) return false;

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
            }
            else {
                
                // open file
                d->cKeyFile.open(cURL.toLocalFile().toStdString());
                if (!d->cKeyFile.is_open()) {
                    qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to open file '" << d->sFileURL << "'";
                }
            }
        }
    }

    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // write block of raw key data
    d->cKeyFile.write((const char *)cKey.data().get(), cKey.data().size());
    
    return true;
}


/**
 * sets the new file URL to read
 * 
 * @param   sFileURL        the new file URL to read from
 */
void qkd_dekey::set_file_url(QString sFileURL) {
    
    // get exclusive access to properties
    std::unique_lock<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // close already opened file
    if (d->cKeyFile.is_open()) d->cKeyFile.close();
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "blob'in input keys to: '" << sFileURL.toStdString() << "'";    
    d->sFileURL = sFileURL.toStdString();
    d->bTryToOpen = true;
}

