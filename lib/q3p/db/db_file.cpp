/*
 * db_file.cpp
 * 
 * Implementation of the Q3P Key file DB
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

// ait
#include <qkd/q3p/db.h>
#include <qkd/utility/syslog.h>

#include "db_file.h"


using namespace qkd;
using namespace qkd::q3p;


// ------------------------------------------------------------
// decls


/**
 * total size of the file
 * 
 * the total size is the meta table + key material
 * 
 * @param   cDB     the database
 * @return  the size of the flat file
 */
static inline uint64_t file_size(qkd::q3p::db const & cDB);

    
// ------------------------------------------------------------
// code


/**
 * close the key DB
 */
void db_file::close_internal() {
    
    // don't proceed if we ain't got something to do
    if (m_nFD <= 0) return;
    
    sync();
    
    // unmap and close file
    if (meta()) munmap(meta(), file_size(*this));
    ::close(m_nFD);
    
    data() = nullptr;
    meta() = nullptr;
    
    m_nFD = 0;
}


/**
 * describe the key-DB
 * 
 * @return  a HR-string describing the key-DB
 */
QString db_file::describe() const {
    return QString("flat file DB at %1").arg(url());
}


/**
 * inits the key-DB
 * 
 * This method MUST be overwritten in subclasses
 * 
 * @param   sURL        the url to the DB
 */
void db_file::init(QString sURL) {
    
    // setup init values
    data() = nullptr;
    meta() = nullptr;
    
    m_nFD = 0;
    
    std::string sFileName = QUrl(sURL, QUrl::TolerantMode).toLocalFile().toStdString();

    // syslog
    qkd::utility::syslog::info() << "opening file DB at \"" << sURL.toStdString() << "\"";

    // open file
    m_nFD = ::open(sFileName.c_str(), O_RDWR | O_CREAT, 0666);
    if (m_nFD == -1) {
        
        // error ...
        std::string sError = strerror(errno);
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed opening file DB at \"" << sURL.toStdString() << "\": " << sError;
        throw qkd::q3p::db::db_init_error();
    }
    
    // truncate file
    if (ftruncate(m_nFD, file_size(*this))) {
        
        // error ...
        std::string sError = strerror(errno);
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to map file DB at \"" << sURL.toStdString() << "\": " << sError;
        throw qkd::q3p::db::db_init_error();
    }
    
    // mmap file
    void * cData = mmap(nullptr, file_size(*this), PROT_READ | PROT_WRITE, MAP_SHARED, m_nFD, 0);
    if (cData == (void *)-1) {
        
        // error ...
        std::string sError = strerror(errno);
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to map file DB at \"" << sURL.toStdString() << "\": " << sError;
        throw qkd::q3p::db::db_init_error();
    }
    
    // we have it!
    meta() = (unsigned char *)cData;
    data() = meta() + amount();
    
    // reset DB
    reset();
    
    // syslog
    qkd::utility::syslog::info() << "opened file DB at \"" << sURL.toStdString() << "\"";
}


/**
 * sync and flushes the DB to disk
 */
void db_file::sync_internal() {
    
    // sanity check
    if (!meta()) return;
    
    // write memory back to disk
    int nError = msync(meta(), file_size(*this), MS_SYNC);
    if (nError == -1) {
        
        // error ...
        std::string sError = strerror(errno);
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to sync file DB to disk: " << sError;
    }
}


/**
 * total size of the file
 * 
 * the total size is the meta table + key material
 * 
 * @param   cDB     the database
 * @return  the size of the flat file
 */
uint64_t file_size(qkd::q3p::db const & cDB) { 
    return (cDB.amount() + cDB.amount() * cDB.quantum()); 
}


