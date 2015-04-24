/*
 * qkd.h
 * 
 * The all-in-one include header for the AIT QKD library
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

 
#ifndef __QKD_H_
#define __QKD_H_


// ------------------------------------------------------------
// incs

// QKD utility stuff
#include <qkd/utility/atof.h>
#include <qkd/utility/average.h>
#include <qkd/utility/backtrace.h>
#include <qkd/utility/bigint.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/checksum.h>
#include <qkd/utility/dbus.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/investigation.h>
#include <qkd/utility/memory.h>
#include <qkd/utility/queue.h>
#include <qkd/utility/properties.h>
#include <qkd/utility/random.h>
#include <qkd/utility/shannon.h>
#include <qkd/utility/si_units.h>
#include <qkd/utility/syslog.h>
#include <qkd/utility/zip.h>

// QKD key stuff
#include <qkd/key/key.h>
#include <qkd/key/key_ring.h>

// QKD crypto stuff
#include <qkd/crypto/association.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>

// Q3P
#include <qkd/q3p/channel.h>
#include <qkd/q3p/db.h>
#include <qkd/q3p/engine.h>
#include <qkd/q3p/message.h>

// QKD module stuff
#include <qkd/module/communicator.h>
#include <qkd/module/message.h>
#include <qkd/module/module.h>

// QKD widgets
#include <qkd/widget/lcd.h>
#include <qkd/widget/led.h>
#include <qkd/widget/module_frame.h>
#include <qkd/widget/plot.h>
#include <qkd/widget/res.h>

// QKD versioning
#include <qkd/version.h>

#endif

