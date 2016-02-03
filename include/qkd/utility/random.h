/*
 * random.h
 * 
 * random number generator interface
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

 
#ifndef __QKD_UTILITY_RANDOM_H_
#define __QKD_UTILITY_RANDOM_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>

#include <inttypes.h>

#include <boost/shared_ptr.hpp>

// ait
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// defs

#if defined(__GNUC__) || defined(__GNUCPP__)
#   define UNUSED   __attribute__((unused))
#else
#   define UNUSED
#endif


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


// fwd
class random_source;
typedef boost::shared_ptr<random_source> random;


/**
 * this encapsulates a random number source
 * 
 * a random source is created by calling
 * 
 *      create(URL);
 * 
 * with a proper URL.
 * 
 * If the URL is empty, than a C API random source 
 * using rand() is created.
 * 
 * An URL might look like this:
 * 
 *      file:///dev/urandom
 * 
 * Whenever the end of a file has been reached (eof)
 * the random source starts reading anew from the
 * beginning of the file.
 * 
 * Hence, the floating point retrievals (float and double) do
 * return values in the range [0.0, 1.0)
 */
class random_source {

    
    friend class random_singelton;

    
public:


    /**
     * exception type thrown for unknown reasons when accessing data from the random source
     */
    class random_get_unknown : public std::exception { 
    public: 
        
        /**
         * exception description 
         * @return  a human readable exception description
         */
        const char * what() const noexcept { return "unknown error while accessing random source data"; }
    };
    

    /**
     * exception type thrown on init errors
     */
    class random_init_error : public std::exception { 
    public: 

        /**
         * exception description 
         * @return  a human readable exception description
         */
        const char * what() const noexcept { return "error during init of random source"; } 
    };


    /**
     * exception type thrown for unknown random url schemes
     */
    class random_url_scheme_unknown : public std::exception { 
    public: 

        /**
         * exception description 
         * @return  a human readable exception description
         */
        const char * what() const noexcept { return "unkown random url scheme"; } 
    };


    /**
     * dtor
     */
    virtual ~random_source() {};


    /**
     * stream out
     * 
     * Get the next random value.
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(char & rhs) { 
        get((char*)&rhs, sizeof(rhs)); 
        return *this; 
    }


    /**
     * stream out
     * 
     * Get the next random value.
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(unsigned char & rhs) { 
        get((char*)&rhs, sizeof(rhs)); 
        return *this; 
    }


    /**
     * stream out
     *
     * Get the next random value.
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(int32_t & rhs) { 
        get((char*)&rhs, sizeof(rhs)); 
        return *this; 
    }


    /**
     * stream out
     *
     * Get the next random value.
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(uint32_t & rhs) { 
        get((char*)&rhs, sizeof(rhs)); 
        return *this; 
    }


    /**
     * stream out
     *
     * Get the next random value.
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(int64_t & rhs) { 
        get((char*)&rhs, sizeof(rhs)); 
        return *this; 
    }


    /**
     * stream out
     *
     * Get the next random value.
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(uint64_t & rhs) { 
        get((char*)&rhs, sizeof(rhs)); 
        return *this; 
    }


    /**
     * stream out
     *
     * Get the next random value in [0.0, 1.0).
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(double & rhs) { 
        uint64_t a; 
        uint64_t b; 
        (*this) >> a; 
        (*this) >> b; 
        if (a > b) rhs = (double)b / (double)a; 
        if (b > a) rhs = (double)a / (double)b; 
        if (a == b) rhs = 0.0; 
        return *this; 
    }


    /**
     * stream out
     *
     * Get the next random value in [0.0, 1.0).
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(float & rhs) { 
        uint64_t a; 
        uint64_t b; 
        (*this) >> a; 
        (*this) >> b; 
        if (a > b) rhs = (float)b / (float)a; 
        if (b > a) rhs = (float)a / (float)b; 
        if (a == b) rhs = 0.0; 
        return *this; 
    }


    /**
     * stream out
     *
     * Get the next random value.
     *
     * @param   rhs         right hand side: the next random value
     * @return  the random object
     */
    inline random_source & operator>>(qkd::utility::memory & rhs) { 
        get((char *)rhs.get(), rhs.size()); 
        return *this; 
    }


    /**
     * factory method to create a random source
     * 
     * if the url is left empty, the C-API rand() and srand() is used as source
     *
     * @param   sURL        a URL string indicating the random source
     * @return  an initialized random object
     */
    static random create(std::string sURL = "");
    
    
    /**
     * describe the random source
     * 
     * @return  a HR-string describing the random source
     */
    virtual std::string describe() const { return "NULL random"; }


    /**
     * sets the main random singleton source
     *
     * @param   cRandom     the new random singleton source
     */
    static void set_source(random & cRandom);


    /**
     * returns the main random singleton source
     *
     * @return  the main random singleton
     */
    static random & source();


protected:


    /**
     * ctor
     */
    random_source() {};
    
    
private:

    
    /**
     * get a block of random bytes
     * 
     * This function must be overwritten in derived classes
     * 
     * @param   cBuffer     buffer which will hold the bytes
     * @param   nSize       size of buffer in bytes
     */
    virtual void get(UNUSED char * cBuffer, UNUSED uint64_t nSize) { 
        throw random_get_unknown();
        return; 
    };
    
};


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, char & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, unsigned char & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, int32_t & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, uint32_t & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, int64_t & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}

    
/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, uint64_t & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, float & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, double & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Get the next random value
 * 
 * @param   lhs     the random object
 * @param   rhs     the next random value
 * @return  the random object
 */
inline random & operator>>(random & lhs, qkd::utility::memory & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}
    

    
}

}

#endif

