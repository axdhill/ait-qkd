/*
 * prepare_measure_heterodyne.h
 * 
 * continue creation for prepare and measure heterodyne
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2016 AIT Austrian Institute of Technology
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

 
#ifndef __QKD_KEY_GEN_CV_PM_HETERODYNE_H_
#define __QKD_KEY_GEN_CV_PM_HETERODYNE_H_


// ------------------------------------------------------------
// incs

// ait
#include "mode.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace cv {


/**
 * the entangled creation mode
 */
class prepare_measure_heterodyne : public mode {
    
    
public:
    
    
    /**
     * add program arguments to our mode settings
     * 
     * @param   cArguments      the arguments as passed from the command line
     * @return  true, if all arguments are ok
     */
    bool consume_arguments(boost::program_options::variables_map const & cArguments);
    
    
    /**
     * report some help on this key generation mode
     * 
     * @return  help shown to the user on stdout about this mode
     */
    static std::string help();
    
    
    /**
     * produce a set of pseudo random cv-keys
     * 
     * @param   cKeyAlice       alice key
     * @param   cKeyBob         bob key
     */
    void produce(qkd::key::key & cKeyAlice, qkd::key::key & cKeyBob);
    
};


}
    
}

#endif
