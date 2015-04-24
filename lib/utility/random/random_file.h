/*
 * random_file.h
 * 
 * random number generator interface reading from a file
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

 
#ifndef __QKD_UTLITY_RANDOM_FILE_H_
#define __QKD_UTLITY_RANDOM_FILE_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <fstream>
#include <string>
#include <boost/exception/all.hpp>

// Qt
#include <QtCore/QUrl>

// ait
#include <qkd/utility/random.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * this class reads random numbers from a file
 */
class random_file : public qkd::utility::random_source {


public:


    /**
     * ctor
     */
    explicit random_file(const QUrl & cURL) : m_sFileName(cURL.toLocalFile().toStdString()) { init(); };


    /**
     * describe the random source
     * 
     * @return  a HR-string describing the random source
     */
    virtual std::string describe() const { return std::string("random source using url: file://") + m_sFileName; };
    

private:

    
    /**
     * get a block of random bytes
     * 
     * This function must be overwritten in derived classes
     * 
     * @param   cBuffer     buffer which will hold the bytes
     * @param   nSize       size of buffer in bytes
     */
    virtual void get(char * cBuffer, uint64_t nSize);
    
    
    /**
     * init the object
     */
    void init();
    
    
    /**
     * the url of the file
     */
    std::string m_sFileName;
    

    /**
     * the file
     */
    std::ifstream m_cFileInStream;
    
    
};


}

}

#endif

