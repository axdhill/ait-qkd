/*
 * queue.h
 * 
 * a thread-safe queue (first in - first out)
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

 
#ifndef __QKD_UTILITY_QUEUE_H_
#define __QKD_UTILITY_QUEUE_H_


// ------------------------------------------------------------
// incs

#include <mutex>
#include <queue>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * this is the same as a STL queue
 * ... but thread safe
 */
template<class T> class queue : private std::queue<T> {


public:
    
    
    /**
     * dtor
     */
    virtual ~queue() {};


    /**
     * check if queue is empty
     * 
     * @return  true, if there is nothing in it
     */
    bool empty() const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return std::queue<T>::empty(); };
    
    
    /**
     * get the mutex
     * 
     * @return  the mutex for synchronized access
     */
    std::recursive_mutex & mutex() const { return m_cMTX; };
    

    /**
     * dequeue an item
     * 
     * @return  item removed from the queue
     */
    T pop() { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); T x = std::queue<T>::front(); std::queue<T>::pop(); return x; };
    
    
    /**
     * enqueue an item
     * 
     * @param   x       item to enqueue
     */
    void push(T const & x) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); std::queue<T>::push(x); };
    
    
    /**
     * number of elements in the queue
     * 
     * @return  number of elements
     */
    uint64_t size() const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return std::queue<T>::size(); };
    
    
private:
    
    
    /**
     * object mutex
     */
    mutable std::recursive_mutex m_cMTX;

};


}
    
}



#endif

