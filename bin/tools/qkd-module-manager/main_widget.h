/*
 * main_widget.h
 * 
 * declares the main widget for QKD Module Manager
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

 
#ifndef __QKD_QKD_MODULE_MANAGER_MAIN_WIDGET_H_
#define __QKD_QKD_MODULE_MANAGER_MAIN_WIDGET_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QtCore/QSettings>
#include <QtGui/QFrame>
#include <QtGui/QMainWindow>

// ait
#include "ui_main_widget.h"



// ------------------------------------------------------------
// decl


// fwd
class QTreeWidgetItem;
namespace qkd { namespace utility { class properties; } namespace widget { class module_frame; } }


namespace qkd {
    
namespace module_manager {


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

    
    /**
     * load widget settings
     * 
     * @param   cSettings       settings object containing our state
     */
    void load_settings(QSettings const & cSettings);
    
    
    /**
     * save widget settings
     * 
     * @param   cSettings       settings object to receive our state
     */
    void save_settings(QSettings & cSettings) const;
    
    
signals:
    
    
    /**
     * quit the app
     */
    void quit();
    
    
public slots:
    
    
    /**
     * start a pipeline
     */
    void pipeline_start();


    /**
     * stop a pipeline
     */
    void pipeline_stop();


    /**
     * select a pipeline file
     */
    void select_pipeline_file();


    /**
     * show module specified by the dbus address
     * 
     * @param   sDBus       the dbus address of the module
     */
    void show_module(std::string sDBus);
    
    
private slots:
    

    /**
     * a new item is set to be the current one in the module list
     * 
     * @param   cCurrent        the new item in the list set current
     * @param   cPrevious       old item
     */
    void module_list_current_changed(QTreeWidgetItem * cCurrent, QTreeWidgetItem * cPrevious);


    /**
     * set a new module tab for all modules
     * 
     * @param   nIndex      the new module tab
     */
    void module_tab_index(int nIndex);
    
    
    /**
     * pipeline text changed
     * 
     * @param   sText       the new pipeline text
     */
    void pipeline_changed(QString const & sText);
    
    
    /**
     * make up system update
     */
    void timeout();
    
    
private:


    /**
     * adds a module widget
     * 
     * @param   sDBus                   DBus address of module
     */
    void add_module_widget(std::string const & sDBus);


    /**
     * removes a module widget
     * 
     * @param   sDBus                   DBus address of module
     */
    void remove_module_widget(std::string const & sDBus);

    /**
     * updates a module widget
     * 
     * @param   cModuleProperties       set of module properties
     */
    void update_module_widget(qkd::utility::properties const & cModuleProperties);
    
    
    /**
     * hash of DBus Address -> Moduel Widgets
     */
    std::map<std::string, qkd::widget::module_frame *> m_cModuleFrame;
    
    
    /**
     * hash of DBus Address -> TreeWidgetItem
     */
    std::map<std::string, QTreeWidgetItem *> m_cModuleTreeWidgetItems;
    
    
    /**
     * hash of DBus Address -> update cycle number
     */
    std::map<std::string, uint64_t> m_cModuleUpdateCycle;
    
    
    /**
     * start icon
     */
    QIcon m_cPipelineStart;


    /**
     * stop icon
     */
    QIcon m_cPipelineStop;


    /**
     * preloaded role icons
     */
    QIcon m_cRoleIcon[2];
    
    
    /**
     * preloaded type icons
     */
    QIcon m_cTypeIcon[8];
    
    
};


}
}

#endif
