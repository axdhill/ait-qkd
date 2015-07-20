/*
 * plot.h
 * 
 * an enhanced plot
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2015 AIT Austrian Institute of Technology
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

 
#ifndef __QKD_WIDGET_PLOT_H_
#define __QKD_WIDGET_PLOT_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QtCore/QSize>
#include <QtGui/QWidget>

// Qwt
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#include <qwt/qwt_plot.h>
#include <qwt/qwt_text.h>
#pragma GCC diagnostic pop


// ------------------------------------------------------------
// decl


namespace qkd {
    
namespace widget {


/**
 * This is an enhanced plot based on QwtPlot
 */
class plot : public QwtPlot {

    
    Q_OBJECT
    
    
public:


    /**
     * ctor
     * 
     * @param   cParent     parent widget
     */
    explicit plot(QWidget * cParent = nullptr);
    
    
    /**
     * ctor
     * 
     * @param   sTitle      title string
     * @param   cParent     parent widget
     */
    explicit plot(QwtText const & sTitle, QWidget * cParent = nullptr);
    

    /**
     * dtor
     */
    virtual ~plot() {}
    

    /**
     * return minimum size
     * 
     * @return  minimum size for plot
     */
    virtual QSize minimumSizeHint() const;
    
    
    /**
     * return ideal size
     * 
     * @return  ideal size for plot
     */
    virtual QSize sizeHint() const;
};


}
}

#endif

