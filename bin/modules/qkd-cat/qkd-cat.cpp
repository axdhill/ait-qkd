/*
 * qkd-cat.cpp
 * 
 * This is the implementation for the qkd-cat module.
 * 
 * The qkd-cat QKD Module picks up a key-file and pushes
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

#include <fstream>

// ait
#include <qkd/utility/syslog.h>

#include "qkd-cat.h"
#include "qkd_cat_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-cat QKD Module: it picks up a keyfile and pushes the content to Pipe-Out."
#define MODULE_ORGANISATION     "(C)opyright 2012, 2013 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-cat pimpl
 */
class qkd_cat::qkd_cat_data {
    
public:

    
    /**
     * ctor
     */
    qkd_cat_data() { 
        bLoop = false;
    };
    
    std::recursive_mutex cPropertyMutex;    /**< property mutex */
    bool bLoop;                             /**< loop flag */
    std::string sFileURL;                   /**< file URL */
    
    std::ifstream cKeyFile;                 /**< the key file */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_cat::qkd_cat() : qkd::module::module("cat", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_cat::qkd_cat_data>(new qkd_cat::qkd_cat_data());
    
    // apply default values
    set_loop(false);
    set_url_pipe_in("");
    
    // enforce DBus registration
    new CatAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_cat::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
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
 * return the file URL to read
 * 
 * @return  the file URL to read from
 */
QString qkd_cat::file_url() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return QString::fromStdString(d->sFileURL);
}


/**
 * return the loop flag
 * 
 * @return  the loop flag
 */
bool qkd_cat::loop() const {
    
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
bool qkd_cat::process(qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {

    // check if our input stream is open
    if (!d->cKeyFile.is_open()) {
        
        // get exclusive access to properties
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);

        // check URL
        QUrl cURL(QString::fromStdString(d->sFileURL));
        if (cURL.scheme().isEmpty()) {
            
            // scheme given we might add file command
            boost::filesystem::path cFilePath(d->sFileURL);
            if (cFilePath.is_relative()) cFilePath = boost::filesystem::canonical(cFilePath);
            cURL = QUrl(QString("file://") + QString::fromStdString(cFilePath.string()));
        }
        
        // test if get the local file
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
    
    // check eof
    if (d->cKeyFile.eof()) {
        
        // get exclusive access to properties
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "reached end-of-file";
        
        // if loop is not set we are done.
        if (!d->bLoop) {
            pause();
            return false;
        }
        
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "rewind read position";
        d->cKeyFile.clear();
        d->cKeyFile.seekg(0);
        
        return false;
    }
    
    // file is open: read a key
    d->cKeyFile >> cKey;
    
    // do not push empty keys
    if (cKey.size() == 0) return false;

    return true;
}


/**
 * sets the new file URL to read
 * 
 * @param   sFileURL        the new file URL to read from
 */
void qkd_cat::set_file_url(QString sFileURL) {
    
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
 * sets the loop flag
 * 
 * @param   bLoop           the new loop flag
 */
void qkd_cat::set_loop(bool bLoop) {
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->bLoop = bLoop;
}

