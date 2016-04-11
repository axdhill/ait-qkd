/*
 * led.cpp
 * 
 * Implement a LED
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
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
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

// ait
#include <qkd/widget/led.h>
#include <qkd/widget/res.h>


using namespace qkd::widget;


// ------------------------------------------------------------
// vars


/**
 * the pixmaps of the LEDs
 */
static QMap<qkd::widget::led::led_state, QPixmap> g_cPixmaps;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cParent     parent object
 * @param   sText       text of the LED
 */
led::led(QString const & sText, QWidget * cParent) : 
    QWidget(cParent), 
    m_bBlinking(false), 
    m_eBlinkingBackState(led::led_state::LED_STATE_GREY), 
    m_nBlinkingHertz(2), 
    m_bBlinkOn(true), 
    m_eState(led::led_state::LED_STATE_GREY), 
    m_sText(sText), 
    m_bTextVisible(true) {
 
    load_pixmaps();
    setMinimumHeight(16);
    m_cBlinkingLast = std::chrono::system_clock::now();
    
    QTimer * cTimer = new QTimer(this);
    connect(cTimer, SIGNAL(timeout()), SLOT(blink()));
    cTimer->start(100);
}


/**
 * dtor
 */
led::~led() {
}


/**
 * perform a blink
 */
void led::blink() {
    
    if (!blinking()) return;
    
    std::chrono::system_clock::time_point cNow = std::chrono::system_clock::now();
    uint64_t nBlinkInterval = 1000 / blinking_hertz();
    uint64_t nSinceLast = std::chrono::duration_cast<std::chrono::milliseconds>(cNow - m_cBlinkingLast).count();

    if (nSinceLast < nBlinkInterval) return;
    
    uint64_t nBlinks = nSinceLast / nBlinkInterval;
    
    m_cBlinkingLast = m_cBlinkingLast + std::chrono::milliseconds(nBlinks * nBlinkInterval);
    if (!(nBlinks & 0x00000001)) return;
    
    m_bBlinkOn = !m_bBlinkOn;
    update();
}


/**
 * ensure the pixmaps are loaded
 */
void led::load_pixmaps() const {
    
    if (g_cPixmaps.size() > 0) return;
    
    QPixmap cPixGreenSmall = res::pixmap("glass_button_green_small");
    QPixmap cPixGreySmall = res::pixmap("glass_button_grey_small"); 
    QPixmap cPixRedSmall = res::pixmap("glass_button_red_small");
    QPixmap cPixYellowSmall = res::pixmap("glass_button_yellow_small");
        
    g_cPixmaps.insert(led::led_state::LED_STATE_GREY,   cPixGreySmall);
    g_cPixmaps.insert(led::led_state::LED_STATE_GREEN,  cPixGreenSmall);
    g_cPixmaps.insert(led::led_state::LED_STATE_YELLOW, cPixYellowSmall);
    g_cPixmaps.insert(led::led_state::LED_STATE_RED,    cPixRedSmall);
}


/**
 * draw the widget
 * 
 * @param   cEvent      the associated paint event
 */
void led::paintEvent(QPaintEvent * cEvent) {
    
    QWidget::paintEvent(cEvent);
    
    led_state eState = state();
    if (blinking() && !m_bBlinkOn) eState = m_eBlinkingBackState;
    
    if (!g_cPixmaps.contains(eState)) return;
    
    QPainter cPainter(this);
    QPixmap cPixmapScaled = g_cPixmaps[eState].scaledToHeight(size().height());
    cPainter.drawPixmap(0, 0, cPixmapScaled);

    if (m_sText.length() == 0) return;
    QRectF cRect(cPixmapScaled.width() + 4, 0, size().width() - (cPixmapScaled.width() + 4), size().height());
    cPainter.drawText(cRect, Qt::AlignLeft | Qt::AlignVCenter, m_sText);
}


/**
 * get  the ideal size of the widget
 * 
 * @return  the ideal size
 */
QSize led::sizeHint() const {
    
    if (!g_cPixmaps.contains(state())) return QSize(0, 0);
    
    QFontMetrics cFM(font());
    QSize cFontSize = cFM.size(Qt::TextSingleLine, m_sText);
    
    QPixmap cPixmapScaled = g_cPixmaps[state()].scaledToHeight(cFontSize.height());
    QMargins cMargins = contentsMargins();

    // here + 4 is added to the width, since the font width does to produce erroneous results
    // same for height
    return QSize(cMargins.left() + cPixmapScaled.width() + 4 + cFontSize.width() + cMargins.right() + 4, cMargins.top() + std::max(cPixmapScaled.height(), cFontSize.height()) + cMargins.bottom() + 4);
}
