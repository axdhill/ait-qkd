/*
 * plot.cpp
 * 
 * Implement an enhanced plot
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2016 AIT Austrian Institute of Technology
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

 
// ------------------------------------------------------------
// include

// Qt
#include <QtWidgets/QApplication>

#if defined(__GNUC__) and not defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#endif

#include "qwt_scale_widget.h"

#if defined(__GNUC__) and not defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <qkd/widget/plot.h>


using namespace qkd::widget;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cParent     parent widget
 */
plot::plot(QWidget * cParent) : plot(QwtText(), cParent) {
}


/**
 * ctor
 * 
 * @param   sTitle      title string
 * @param   cParent     parent widget
 */
plot::plot(QwtText const & sTitle, QWidget * cParent) : QwtPlot(sTitle, cParent) {
    
    // fix font (Qwt hardcoded 10pt)
    for (int i = 0; i < QwtPlot::Axis::axisCnt; ++i) setAxisFont(i, QApplication::font());
}


/**
 * return minimum size
 * 
 * @return  minimum size for plot
 */
QSize plot::minimumSizeHint() const {
    return QSize(0, 0);
}


/**
 * return ideal size
 * 
 * @return  ideal size for plot
 */
QSize plot::sizeHint() const {
    return QSize(0, 0);
}
