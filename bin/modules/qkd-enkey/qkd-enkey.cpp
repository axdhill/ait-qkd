/*
 * qkd-enkey.cpp
 * 
 * This is the implementation for the qkd-enkey module.
 * 
 * The qkd-enkey QKD Module picks up a blob and pushes
 * the content as key-stream to pipeout.
 * 
 * This acts much like qkd-cat but with BLOBs of key data.
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

#include <atomic>
#include <fstream>

// ait
#include <qkd/utility/syslog.h>

#include "qkd-enkey.h"
#include "qkd_enkey_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-enkey QKD Module: it picks up a BLOB and pushes the content as key-stream to Pipe-Out."
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-enkey pimpl
 */
class qkd_enkey::qkd_enkey_data {
    
public:

    
    /**
     * ctor
     */
    qkd_enkey_data() { 
        nKeyId = 1;
        nKeySize = 1024;
        bLoop = false;
    };
    
    std::recursive_mutex cPropertyMutex;    /**< property mutex */
    qkd::key::key_id nKeyId;                /**< current key id we work on */
    uint64_t nKeySize;                      /**< size of a single key */
    bool bLoop;                             /**< loop flag */
    std::string sFileURL;                   /**< file URL */
    
    std::ifstream cKeyFile;                 /**< the key file */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_enkey::qkd_enkey() : qkd::module::module("enkey", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_enkey::qkd_enkey_data>(new qkd_enkey::qkd_enkey_data());
    
    // apply default values
    set_key_size(1024);
    set_loop(false);
    set_url_pipe_in("");
    
    // enforce DBus registration
    new EnkeyAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_enkey::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
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
        else 
        if (sKey == "key_size") {
            set_key_size(std::stoull(cEntry.second));
        }
        else 
        if (sKey == "loop") {
            if (cEntry.second == "true") set_loop(true);
            else
            if (cEntry.second == "false") set_loop(false);
            else {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "at key \"" << cEntry.first << "\" - can\t parse value \"" << cEntry.second << "\".";
            }
            
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the current key id we are blob'in
 * 
 * @return  the current key id
 */
qulonglong qkd_enkey::current_id() const {
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nKeyId;
}


/**
 * return the file URL to read
 * 
 * @return  the file URL to read from
 */
QString qkd_enkey::file_url() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return QString::fromStdString(d->sFileURL);
}


/**
 * return the size of a single key in bytes
 * 
 * @return  the size of single key in bytes
 */
uint64_t qkd_enkey::key_size() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nKeySize;
}


/**
 * return the loop flag
 * 
 * @return  the loop flag
 */
bool qkd_enkey::loop() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->bLoop;
}


/**
 * module work
 * 
 * @param   cKey                    will be set to the loaded key from the file
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_enkey::process(qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {

    // check if our input stream is open
    if (!d->cKeyFile.is_open()) {
        
        // get exclusive access to properties
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        
        // check URL
        QUrl cURL(QString::fromStdString(d->sFileURL));
        if (!cURL.isLocalFile()) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "'" << d->sFileURL << "' seems not to point to a local file - wont proceed";
            pause();
            return false;
        }
        
        // open file
        d->cKeyFile.open(cURL.toLocalFile().toStdString());
        if (!d->cKeyFile.is_open()) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to open file '" << d->sFileURL << "'";
            pause();
            return false;
        }
    }
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // read in a blob
    qkd::utility::memory cBlob(d->nKeySize);
    uint64_t nRead = 0;
    while (nRead < d->nKeySize) {
        
        // read in a portion of the file
        d->cKeyFile.read(reinterpret_cast<char *>(cBlob.get() + nRead), d->nKeySize - nRead);
        nRead += d->cKeyFile.gcount();
        if (nRead == d->nKeySize) break;
        
        // ok: read less bytes than needed
        if (d->cKeyFile.eof()) {
            
            // eof! rewind?
            if (qkd::utility::debug::enabled()) qkd::utility::debug() << "reached end-of-file";
            if (!d->bLoop) {
            
                // no loop -> bye!
                qkd::utility::syslog::info() << "insufficiant key material (" << nRead << " of " << d->nKeySize << ") and not looping - aborting";
                pause();
                return false;
            }
            
            // rewind!
            if (qkd::utility::debug::enabled()) qkd::utility::debug() << "rewind read position";
            d->cKeyFile.clear();
            d->cKeyFile.seekg(0);
        }
        else {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "error reading from file";
            pause();
            return false;
        }
    }
    
    // create the key
    d->nKeyId = qkd::key::key::counter().inc();
    cKey = qkd::key::key(d->nKeyId, cBlob);
    cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_OTHER;
    
    return true;
}


/**
 * sets the new file URL to read
 * 
 * @param   sFileURL        the new file URL to read from
 */
void qkd_enkey::set_file_url(QString sFileURL) {
    
    // don't change URL when already running
    if (is_working_state()) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "refusing to change file URL when already running";
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refusing to change file URL when already running";
        return;
    }
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "reading input keys from: '" << sFileURL.toStdString() << "'";    
    d->sFileURL = sFileURL.toStdString();
}


/**
 * sets the key_size in bytes
 * 
 * @param   nKeySize        the new size of a single key in bytes
 */
void qkd_enkey::set_key_size(uint64_t nKeySize) {
    
    // we must at least have a valid key size of > 0
    if (nKeySize == 0) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refusing setting key size to 0";
        return;
    }
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nKeySize = nKeySize;
}


/**
 * sets the loop flag
 * 
 * @param   bLoop           the new loop flag
 */
void qkd_enkey::set_loop(bool bLoop) {
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->bLoop = bLoop;
}

