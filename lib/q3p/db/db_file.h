/*
 * db_file.h
 * 
 * A flat file acting as key store DB
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
 

#ifndef __QKD_Q3P_DB_DB_FILE_H_
#define __QKD_Q3P_DB_DB_FILE_H_


// ------------------------------------------------------------
// incs

#include <inttypes.h>

// Qt
#include <QtCore/QObject>

// ait
#include <qkd/key/key.h>
#include <qkd/q3p/db.h>

#include "db_ram.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    
    
    
/**
 * This is a Key DB working with files.
 * 
 * Actually this uses spares files which are memory mapped by the kernel.
 * Should be fast enough.
 * 
 * The file contains:
 * 
 *      key meta table ... size: max_id() - min_id()
 *                         an unsigned char holding flags:
 * 
 *                         7-6-5-4-3-2-1-0
 *                         . . . . . E R V
 * 
 *                         V ... key is valid when set
 *                         R ... key is in real sync
 *                         E ... key is in eventual sync
 * 
 *     key material ... 32 bytes (256 bits) for each key in a row
 */
class db_file : public db_ram {
    
    
    Q_OBJECT
    
    
public:    
    
 
    /**
     * ctor
     * 
     * @param   sURL        url of the DB to create
     */
    db_file(QString sURL) : db_ram(sURL), m_nFD(0) {};
    
    
    /**
     * dtor
     */
    ~db_file() { close(); };
    
    
    /**
     * describe the key-DB
     * 
     * @return  a HR-string describing the key-DB
     */
    QString describe() const;
    
    
private:
    
    
    /**
     * close the key DB
     */
    void close_internal();
    

    /**
     * inits the key-DB
     * 
     * This method MUST be overwritten in subclasses
     * 
     * @param   sURL        the url to the DB
     */
    void init(QString sURL);
    
    
    /**
     * get the maximum ID of a key
     * 
     * @return  the maximum ID of a key
     */
    qkd::key::key_id max_id_internal() const { return 1 << 24; };
    

    /**
     * get the minimum ID of a key
     * 
     * @return  the minimum ID of a key
     */
    qkd::key::key_id min_id_internal() const { return 0; };
    

    /**
     * get the fixed size of key material manged in the key DB
     * 
     * This is the amount of key material hold by a single
     * key in the database
     * 
     * @return  the fixed size in bytes of a key in the DB
     */
    uint64_t quantum_internal() const { return (256 / 8); };
    

    /**
     * sync and flushes the DB to disk
     */
    void sync_internal();
    
    
    /**
     * fd of the file
     */
    int m_nFD;

};
  

}

}

#endif
