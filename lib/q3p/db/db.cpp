/*
 * db.cpp
 * 
 * Implementation of the Q3P Key DB
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
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

// ait
#include <qkd/q3p/db.h>

#include "db_file.h"
#include "db_null.h"
#include "db_ram.h"


using namespace qkd;
using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * factory method to create or open a key-db
 *
 * @param   sURL        a URL string indicating the key-db source and type
 * @return  an initialized key_db object
 */
key_db db::open(QString sURL) {

    // check what we have
    QUrl cURL(sURL, QUrl::TolerantMode);
    
    // null://
    if (cURL.scheme() == "null") {
        key_db cKeyDB = std::shared_ptr<db>(new qkd::q3p::db_null(sURL));
        cKeyDB->init(sURL);
        return cKeyDB;
    }
    
    // ram://
    if (cURL.scheme() == "ram") {
        key_db cKeyDB = std::shared_ptr<db>(new qkd::q3p::db_ram(sURL));
        cKeyDB->init(sURL);
        return cKeyDB;
    }
    
    // file://
    if (cURL.isLocalFile() || (cURL.scheme() == "")) {
        key_db cKeyDB = std::shared_ptr<db>(new qkd::q3p::db_file(sURL));
        cKeyDB->init(sURL);
        return cKeyDB;
    }
    
    throw db_url_scheme_unknown();
}

