/*
 * res.h
 * 
 * this holds libqkd-wide resources (eg. QPixmaps, ...)
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

 
#ifndef __QKD_WIDGET_RES_H_
#define __QKD_WIDGET_RES_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QtGui/QFont>
#include <QtGui/QPixmap>


// ------------------------------------------------------------
// decl


// fwd
class QBoxLayout;
class QWidget;


namespace qkd {
    
namespace widget {


/**
 * This holds the libqkd-wide resources
 */
class res {

    
public:
    
    
    /**
     * get the LCD font with a given point size
     * 
     * @param   nPointSize      pointsize of the LCD font
     * @return  a LCD font
     */
    static QFont lcd_font(int nPointSize = -1);


    /**
     * get a pixmap based on the given id
     * 
     * if the pixmap has not been found, the returned pixmap
     * is empty (isNull() == true)
     * 
     * @param   sID         the id of the pixmap in question
     * @return  the pixmap
     */
    static QPixmap pixmap(QString const & sID);
    
    
    /**
     * replaces a widget in the hierachy
     * 
     * the old widget is deleted
     * 
     * @param   cLayout         the box layout containing the widget
     * @param   cWidgetOld      the old widget
     * @param   cWidgetNew      the new widget
     * @return  returns the new widget on success
     */
    static QWidget * swap_widget(QBoxLayout * cLayout, QWidget * cWidgetOld, QWidget * cWidgetNew);

    
private:
    
    
    /**
     * dtor
     */
    ~res() = delete;
    
};


}
}

#endif

