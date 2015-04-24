/*
 * module_frame.h
 * 
 * a GUI to inspect some states of a running module
 * 
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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

 
#ifndef __QKD_WIDGET_MODULE_FRAME_H_
#define __QKD_WIDGET_MODULE_FRAME_H_


// ------------------------------------------------------------
// incs

#include <string>

#include <boost/shared_ptr.hpp>

// Qt
#include <QtDBus/QDBusConnection>
#include <QtGui/QFrame>


// ------------------------------------------------------------
// decl


// fwd
class QTabWidget;
namespace Ui { class module_frame; }
namespace qkd { namespace utility { class properties; } }


namespace qkd {
    
namespace widget {


/**
 * This class let you manage some part of a module visually
 */
class module_frame : public QFrame {

    
    Q_OBJECT
    
    
public:


    /**
     * ctor
     * 
     * @param   cParent     parent object
     * @param   cDBus       DBus session object where to module resides
     */
    module_frame(QWidget * cParent, QDBusConnection cDBus);
    

    /**
     * dtor
     */
    virtual ~module_frame();
    
    
    /**
     * return the DBus Address of this module frame
     * 
     * the DBus address serves as a unique ID of modules
     * 
     * @return  the DBus address of the module managed
     */
    std::string dbus() const;
    
    
    /**
     * get the included tab widget
     * 
     * @return  the tab widget used
     */
    QTabWidget * tab();
    
    
public slots:
    
    
    /**
     * clicked resume button
     */
    void clicked_resume();


    /**
     * clicked stop button
     */
    void clicked_stop();


    /**
     * refresh the last values
     * 
     * this places the recet properties into the UI
     */
    void refresh_ui();
    
    
    /**
     * pause the module
     */
    void pause();
    

    /**
     * run/resume the module
     */
    void resume();
    
    
    /**
     * terminate the module
     */
    void terminate();
    
    
    /**
     * update the data shown
     * 
     * the given properties as retrieved as by 
     * qkd::utility::investigation for the modules
     * 
     * @param   cProperties     new properties of the module
     */
    void update(qkd::utility::properties const & cProperties);
    

    /**
     * update bits tab plot
     */
    void update_tab_bits();
    

    /**
     * update keys tab plot
     */
    void update_tab_keys();
    

    /**
     * update qber tab plot
     */
    void update_tab_qber();
    

private slots:
    
    
    /**
     * apply new debug state
     * 
     * @param   nState      new Qt::CheckState var
     */
    void apply_debug(int nState);
    
    
    /**
     * apply new hint
     */
    void apply_hint();
    
    
    /**
     * apply new pipeline
     */
    void apply_pipeline();
    
    
    /**
     * apply new url in
     */
    void apply_url_in();
    
    
    /**
     * apply new url out
     */
    void apply_url_out();
    
    
    /**
     * apply new url peer
     */
    void apply_url_peer();
    
    
private:    

  
    /**
     * user interface
     */
    Ui::module_frame * m_cUI;
    

    // pimpl
    class module_frame_data;
    boost::shared_ptr<module_frame_data> d;
};


}
}

#endif

