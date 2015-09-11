/*
 * qkd-statistics.cpp
 * 
 * This is the implementation for the qkd-statistics module.
 * 
 * The qkd-statistics QKD Module picks up a key-file and pushes
 * the content to pipeout.
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

#include <boost/format.hpp>

// ait
#include <qkd/utility/average.h>
#include <qkd/utility/shannon.h>
#include <qkd/utility/syslog.h>

#include "qkd-statistics.h"
#include "qkd_statistics_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-statistics QKD Module: it places statistics info of the bypassing keystream into a file"
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-statistics pimpl
 */
class qkd_statistics::qkd_statistics_data {
    
public:

    
    /**
     * ctor
     */
    qkd_statistics_data(qkd_statistics * cParent) : 
            cParentModule(cParent), bWarningDisplayed(false), bHeaderWritten(false) {

        nKeysOutgoing = 0;
        nKeyBitsOutgoing = 0;
        cKeysOutgoingRate = qkd::utility::average_technique::create("time", 1000);
        cKeyBitsOutgoingRate = qkd::utility::average_technique::create("time", 1000);
    };
    
    qkd_statistics * cParentModule;                     /**< the encapsuling parent module */

    std::recursive_mutex cPropertyMutex;                /**< property mutex */
    std::string sFileURL;                               /**< file URL */
    std::ofstream cStatisticsFile;                      /**< the statistics file */

    bool bWarningDisplayed;                             /**< displayed initial warning on file opening errors */
    bool bHeaderWritten;                                /**< header has been written */

    uint64_t nKeysOutgoing;                             /**< number of keys outgoing  */
    uint64_t nKeyBitsOutgoing;                          /**< number of keys bits outgoing */
    
    qkd::utility::average cKeysOutgoingRate;            /**< calculate gain of keys outgoing of the last second */
    qkd::utility::average cKeyBitsOutgoingRate;         /**< calculate gain of key bits outgoing of the last second */


    /**
     * ensure we have a file to write statistics to
     *
     * @returns true, if we have
     */
    bool ensure_file_open();


    /**
     * write header
     */
    void write_header();


    /**
     * write statistics
     *
     * @param   cKey        current key
     */
    void write_statistics(qkd::key::key const & cKey);

};


// ------------------------------------------------------------
// code


/**
 * ensure we have a file to write statistics to
 *
 * @returns true, if we have
 */
bool qkd_statistics::qkd_statistics_data::ensure_file_open() {

    if (cStatisticsFile.is_open()) return true;

    if (sFileURL.empty()) {
        
        if (!bWarningDisplayed) {
            qkd::utility::debug() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": no file to write statistics given.";
            bWarningDisplayed = true;
        }
        return false;
    }
    
    std::unique_lock<std::recursive_mutex> cLock(cPropertyMutex);
    
    QUrl cURL(QString::fromStdString(sFileURL));
    if (cURL.scheme().isEmpty()) {
        
        // scheme given we might add file command
        boost::filesystem::path cFilePath(sFileURL);
        if (cFilePath.is_relative()) cFilePath = boost::filesystem::current_path() / cFilePath;
        cURL = QUrl(QString("file://") + QString::fromStdString(cFilePath.string()));
    }

    if (!cURL.isLocalFile()) {

        if (!bWarningDisplayed) {
            qkd::utility::syslog::crit() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": " 
                    << "'" 
                    << sFileURL 
                    << "' seems not to point to a local file - wont proceed";
            bWarningDisplayed = true;
        }
        return false;
    }

    cStatisticsFile.open(cURL.toLocalFile().toStdString());
    if (cStatisticsFile.is_open()) {
        return true;
    }

    if (!bWarningDisplayed) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to open file '" 
                << sFileURL 
                << "'";
        bWarningDisplayed = true;
    }

    return false;
}


/**
 * write header
 */
void qkd_statistics::qkd_statistics_data::write_header() {

    if (bHeaderWritten) return;
    cStatisticsFile 
            << "timestamp         id         bits       qber   disclosed bits  state         sh.eff. total keys   total bits         keys/second  bps"
            << std::endl;
    bHeaderWritten = true;
}


/**
 * write statistics
 *
 * @param   cKey        current key
 */
void qkd_statistics::qkd_statistics_data::write_statistics(qkd::key::key const & cKey) {

    std::lock_guard<std::recursive_mutex> cLock(cPropertyMutex);

    boost::format cLineFormater = 
            boost::format("%015ums %010u %010u %6.4f %010u      %-13s %7.5f %012u %018u %12.0f %14.0f");

    double nShannonEfficiency = qkd::utility::shannon_efficiency(
            cKey.meta().nErrorRate, 
            (double)cKey.meta().nDisclosedBits / (cKey.size() * 8.0));

    nKeysOutgoing += 1;
    nKeyBitsOutgoing += cKey.size() * 8;
    cKeysOutgoingRate << 1.0;
    cKeyBitsOutgoingRate << (cKey.size() * 8.0);

    double kps = cKeysOutgoingRate->sum();
    double bps = cKeyBitsOutgoingRate->sum();

    auto cTimePoint = std::chrono::duration_cast<std::chrono::milliseconds>(cParentModule->age());
    cLineFormater % cTimePoint.count();
    cLineFormater % cKey.id();
    cLineFormater % (cKey.size() * 8);
    cLineFormater % cKey.meta().nErrorRate;
    cLineFormater % cKey.meta().nDisclosedBits;
    cLineFormater % cKey.state_string();
    cLineFormater % nShannonEfficiency;
    cLineFormater % nKeysOutgoing;
    cLineFormater % nKeyBitsOutgoing;
    cLineFormater % kps;
    cLineFormater % bps;
    
    cStatisticsFile << cLineFormater.str() << std::endl;
}


/**
 * ctor
 */
qkd_statistics::qkd_statistics() : 
        qkd::module::module("statistics", 
        qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_statistics::qkd_statistics_data>(new qkd_statistics::qkd_statistics_data(this));
    
    // enforce DBus registration
    new StatisticsAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_statistics::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    for (auto const & cEntry : cConfig) {
        
        if (!is_config_key(cEntry.first)) continue;
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        if (sKey == "alice.file_url") {
            if (is_alice()) set_file_url(QString::fromStdString(cEntry.second));
        }
        else 
        if (sKey == "bob.file_url") {
            if (is_bob()) set_file_url(QString::fromStdString(cEntry.second));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": found unknown key: \"" 
                    << cEntry.first 
                    << "\" - don't know how to handle this.";
        }
    }
}


/**
 * return the file URL to read
 * 
 * @return  the file URL to read from
 */
QString qkd_statistics::file_url() const {
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
bool qkd_statistics::process(qkd::key::key & cKey, 
        UNUSED qkd::crypto::crypto_context & cIncomingContext, 
        UNUSED qkd::crypto::crypto_context & cOutgoingContext) {
    
    if (!d->ensure_file_open()) return true;

    d->write_header();
    d->write_statistics(cKey);

    return true;
}


/**
 * sets the new file URL to read
 * 
 * @param   sFileURL        the new file URL to read from
 */
void qkd_statistics::set_file_url(QString sFileURL) {
    
    std::unique_lock<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    if (d->cStatisticsFile.is_open()) d->cStatisticsFile.close();
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << __FILENAME__ 
            << "statistics file set to: '" 
            << sFileURL.toStdString() 
            << "'";    
    }
    d->sFileURL = sFileURL.toStdString();
    d->bWarningDisplayed = false;
    d->bHeaderWritten = false;
}

