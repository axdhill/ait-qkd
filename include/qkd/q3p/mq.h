/*
 * mq.h
 * 
 * this file describes the Q3P message queue handling
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

 
#ifndef __QKD_Q3P_MQ_H_
#define __QKD_Q3P_MQ_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>

#include <inttypes.h>

#include <boost/shared_ptr.hpp>

// Qt
#include <QtCore/QObject>
#include <QtDBus/QtDBus>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

// fwd
class engine_instance;
    
class mq_instance;
typedef boost::shared_ptr<mq_instance> mq;


/**
 * This is the message queue handler object.
 * 
 * The message queue handler object is a "Key Pump".
 * Once an engine launches it serves a message queue.
 * 
 *      see manpages for message queues
 * 
 * A operating system message queue is a FIFO. We place keys
 * in this FIFO. Each key is taken from the engine's application
 * buffer at has the very same size as the application's buffer
 * quantum.
 * 
 * The offered DBus interface is
 * 
 *      DBus Interface: "at.ac.ait.q3p.mq"
 * 
 * 
 * Properties of at.ac.ait.q3p.mq
 * 
 *      -name-          -read/write-    -description-
 * 
 *      name                R           name of the message queue
 * 
 *      paused              R           check if the message queue is currently paused
 * 
 * 
 * Methods of at.ac.ait.q3p.mq
 * 
 *      -name-                  -description-
 * 
 *      purge()                 cleanse out all of the message queue
 * 
 *      pause()                 pause message queue filling
 * 
 *      resume()                resume message queue filling
 * 
 * 
 * Signals emitted by at.ac.ait.q3p.mq
 * 
 *      mode_changed(bool)      emitted when paused/resume has happened
 * 
 *      purged                  emitted when mq has been purged
 * 
 */
class mq_instance : public QObject {
    

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.q3p.mq")
    
    Q_PROPERTY(QString name READ name)                      /**< message queue name */
    Q_PROPERTY(bool paused READ paused)                     /**< production state of message queue */
    
public:
    
    
    /**
     * exception type thrown if we don't have an engine
     */
    struct mq_no_engine : virtual std::exception, virtual boost::exception { };
    
    
    /**
     * ctor
     * 
     * @param   cEngine     the parent engine
     * @throws  mq_no_engine
     */
    mq_instance(qkd::q3p::engine_instance * cEngine);
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    inline qkd::q3p::engine_instance * engine() { return m_cEngine; };
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    inline qkd::q3p::engine_instance const * engine() const { return m_cEngine; };
    
    
    /**
     * name of the message queue
     * 
     * @return  the name of the message queue
     */
    inline QString name() const { return QString::fromStdString(m_sName); };

    
    /**
     * return the current production state of the message queue
     * 
     * @return  true, if paused
     */
    inline bool paused() const { return m_bPaused; };


signals:
    
    
    /**
     * the production state has changed (either pause or resume)
     * 
     * @param   bPaused         true, if we just entered paused mode
     */
    void mode_changed(bool bPaused);
    
    
    /**
     * emitted when the mq has been purged
     */
    void purged();
    
    
public slots:
    
    
    /**
     * pause message queue fillment
     */
    void pause();
    
    
    /**
     * fill message queue with keys
     */
    void produce();
    
    
    /**
     * purge the message queue
     */
    void purge();
    
    
    /**
     * resume message queue fillment
     */
    void resume();
    
    
private:
    
    
    /**
     * the Q3P engine
     */
    qkd::q3p::engine_instance * m_cEngine;
    
    
    /**
     * name of the message queue
     */
    std::string m_sName;


    /**
     * production state of the mq
     */
    bool m_bPaused;
    

    // pimpl
    class mq_data;
    boost::shared_ptr<mq_data> d;
};
  

}

}


#endif

