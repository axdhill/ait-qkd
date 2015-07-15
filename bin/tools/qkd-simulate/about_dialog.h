/*
 * main_widget.h
 * 
 * declares the about dialog for QKD Simulate
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

 
#ifndef __QKD_QKD_SIMULATE_ABOUT_DIALOG_H_
#define __QKD_QKD_SIMULATE_ABOUT_DIALOG_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QDialog>


// ------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {    

    
/**
 * This dialog show what this is all about
 */
class about_dialog : public QDialog {
    

    Q_OBJECT
    
    
public:


    /**
     * ctor
     * 
     * @param   cParent     parent widget
     */
    about_dialog(QWidget * cParent = nullptr);
    
    
    /**
     * dtor
     */
    virtual ~about_dialog() {};


private:    
    
    
    /**
     * setup the widget
     */
    void setup_widget();
};


}
}


#endif
