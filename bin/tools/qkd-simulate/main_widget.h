/*
 * main_widget.h
 * 
 * declares the main widget for QKD Simulate
 * 
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 * Autor: Philipp Grabenweger
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

 
#ifndef __QKD_QKD_SIMULATE_MAIN_WIDGET_H_
#define __QKD_QKD_SIMULATE_MAIN_WIDGET_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QtGui/QFrame>
#include <QtGui/QFileDialog>
#include <QtGui/QMainWindow>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// ait
#include "about_dialog.h"
#include "channel/channel.h"
#include "ui_main_widget.h"


// ------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {


/**
 * This class represenst the main widget of QKD Simulate
 */
class main_widget : public QFrame, private Ui::main_widget {

    
    Q_OBJECT
    
    
public:


    /**
     * ctor
     * 
     * @param   cParent     parent object
     */
    main_widget(QMainWindow * cParent);
    

    /**
     * dtor
     */
    virtual ~main_widget();
    
    
public slots:
    
    
    /**
     * check current widget states
     */
    void check_ui();


    /**
     * about clicked
     */
    void clicked_about();
    

    /**
     * default clicked
     */
    void clicked_default();
    

    /**
     * dump channel parameters to text file
     */
    void clicked_dump_parameters();
    
    
    /**
     * load clicked
     */
    void clicked_load();
    

    /**
     * save clicked
     */
    void clicked_save();
    
    
    /**
     * clicked select event file Alice
     */
    void clicked_select_event_file_alice();
    
    
    /**
     * clicked select event file Bob
     */
    void clicked_select_event_file_bob();
    
    
    /**
     * clicked select free file Alice
     */
    void clicked_select_free_file_alice();
    
    
    /**
     * clicked select free file Bob
     */
    void clicked_select_free_file_bob();
    
    
    /**
     * start clicked
     */
    void clicked_start();
    
    
    /**
     * stop clicked
     */
    void clicked_stop();
    

    /**
     * update simualtion view: simulation progress ...
     */
    void update_simulation_view();
    
    
    /**
     * update values from the input
     */
    void update_values();

    
signals:
    
    
    /**
     * quit the app
     */
    void quit();
    
    
    /**
     * an update message event occured
     */
    void update_message(QString sMessage);
    

private:
    
    
    /**
     * load alice settings from an XML Dom Element
     * 
     * @param   cElement    the DOM XML Element
     */
    void load_alice(QDomElement const & cElement);
    
    
    /**
     * load bob settings from an XML Dom Element
     * 
     * @param   cElement    the DOM XML Element
     */
    void load_bob(QDomElement const & cElement);
    
    
    /**
     * load general settings from an XML Dom Element
     * 
     * @param   cElement    the DOM XML Element
     */
    void load_general(QDomElement const & cElement);
    
    
    /**
     * load output settings from an XML Dom Element
     * 
     * @param   cElement    the DOM XML Element
     */
    void load_output(QDomElement const & cElement);
    
    
    /**
     * load source settings from an XML Dom Element
     * 
     * @param   cElement    the DOM XML Element
     */
    void load_source(QDomElement const & cElement);
    
    
    /**
     * load from an XML string
     * 
     * 
     * @param   cDomDoc     the XML DOM Doc string containing the values
     */
    void load_xml(QDomDocument const & cDomDoc);
    
    
    /**
     * update alice detector
     */
    void update_detector_alice();


    /**
     * update bob detector
     */
    void update_detector_bob();


    /**
     * update values for the fiber
     */
    void update_fiber();


    /**
     * update values for the output
     */
    void update_output();


    /**
     * update values for the source
     */
    void update_source();


    // --------------------------------------------------
    // the channel used
    
    
    channel * m_cChannel;
    
    
    // --------------------------------------------------
    // sub dialogs
    
    about_dialog * m_cDgAbout;
};


}
}

#endif
