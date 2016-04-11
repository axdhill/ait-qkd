/*
 * lcd.cpp
 * 
 * Implement a LCD
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
#include <QtGui/QPainter>
#include <QtGui/QPaintDevice>

// ait
#include <qkd/common_macros.h>
#include <qkd/widget/lcd.h>
#include <qkd/widget/res.h>


using namespace qkd::widget;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   sText       text of the LCD
 * @param   cParent     parent object
 */
lcd::lcd(QString const & sText, QWidget * cParent) : QLineEdit(sText, cParent) {
    setReadOnly(true);
}


/**
 * the widget has been resized
 * 
 * @param   cEvent      resize event information
 */
void lcd::resizeEvent(UNUSED QResizeEvent * cEvent) {
    
    QMargins cMargins = textMargins();
    
    // fix the point size of the LCD font
    // (72 point == 1 inch)
    
    // Qt does not give the real values ... we have to make some crude fixes here :(
    int nHeightOfText = height() - cMargins.top() - cMargins.bottom() - 12;
    double nPixelPerPoint = logicalDpiY() / 72.0;
    int nIdealPointSize = std::max<int>(nHeightOfText / nPixelPerPoint, 6);
    setFont(res::lcd_font(nIdealPointSize));
}

