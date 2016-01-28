/*
 * led.h
 * 
 * this is a LED
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

 
#ifndef __QKD_WIDGET_LED_H_
#define __QKD_WIDGET_LED_H_


// ------------------------------------------------------------
// incs

#include <chrono>

// Qt
#include <QtGui/QWidget>


// ------------------------------------------------------------
// decl


namespace qkd {
    
namespace widget {


/**
 * This is a LED
 */
class led : public QWidget {

    
    Q_OBJECT
    
    
public:


    /**
     * different LED states
     */
    enum class led_state : uint8_t {
        
        LED_STATE_GREY = 0,         /**< led is currently grey (disabled) */
        LED_STATE_GREEN,            /**< led is green */
        LED_STATE_YELLOW,           /**< led is yellow */
        LED_STATE_RED               /**< led is red */
    };


    /**
     * ctor
     * 
     * @param   cParent     parent object
     */
    led(QWidget * cParent = nullptr) : led("", cParent) {}
    
    
    /**
     * ctor
     * 
     * @param   sText       text of the LED
     * @param   cParent     parent object
     */
    led(QString const & sText, QWidget * cParent = nullptr);
    
    
    /**
     * dtor
     */
    virtual ~led();
    
    
    /**
     * get blinking flag
     *
     * @return  the blinking flag
     */
    bool blinking() const { return m_bBlinking; }
    
    
    /**
     * return rate of blinking [1-10Hz]
     * 
     * @return  the rate of blinking in Hz
     */
    uint64_t blinking_hertz() const { return m_nBlinkingHertz; }
    
    
    /**
     * state shown when blinking goes "off"
     * 
     * @return  the "off" blinking state
     */
    led_state blinking_back_state() const { return m_eBlinkingBackState; }
    
    
    /**
     * set blinking flag
     *
     * @param   bBlinking           turn blinking on or off
     */
    void set_blinking(bool bBlinking) { m_bBlinking = bBlinking; update(); }
    
    
    /**
     * set rate of blinking [1-10Hz]
     * 
     * @param   nBlinkingHertz      the new ferquency to blink
     */
    void set_blinking_hertz(uint64_t nBlinkingHertz) { 
        if ((nBlinkingHertz >= 1) && (nBlinkingHertz <= 10)) m_nBlinkingHertz = nBlinkingHertz; 
        update(); 
    }
    
    
    /**
     * set shown when blinking goes "off"
     * 
     * @param   eBlinkingBackState  the blinking back state
     */
    void set_blinking_back_state(led_state eBlinkingBackState) { 
        m_eBlinkingBackState = eBlinkingBackState; 
        update(); 
    }
    
    
    /**
     * sets the new LED state
     * 
     * @param   eState      the new state of the LED
     */
    void set_state(led_state eState) { 
        m_eState = eState; 
        update(); 
    }
    
    
    /**
     * set the text beneath the LED
     * 
     * @param   sText       the new text associated with the LED
     */
    void set_text(QString const & sText) { 
        m_sText = sText; 
        update(); 
    }
    
    
    /**
     * set the flag to show the text
     * 
     * @param   bTextVisible    the new text visibility flag
     */
    void set_text_visible(bool bTextVisible) { 
        m_bTextVisible = bTextVisible; 
        update(); 
    }
    
    
    /**
     * get  the ideal size of the widget
     * 
     * @return  the ideal size
     */
    virtual QSize sizeHint() const;

    
    /**
     * return the LED state
     * 
     * @return  the current led state
     */
    led_state state() const { return m_eState; }
    
    
    /**
     * the text beneath the LED
     * 
     * @return  the text associated with the LED
     */
    QString text() const { return m_sText; }
    
    
    /**
     * flag to show the text
     * 
     * @return  true, if the text should be shown
     */
    bool text_visible() const { return m_bTextVisible; }
    

public slots:
    
    
    /**
     * perform a blink
     */
    void blink();
    
    
protected:
    

    /**
     * draw the widget
     * 
     * @param   cEvent      the associated paint event
     */
    void paintEvent(QPaintEvent * cEvent);
    
    
private:
    
    
    /**
     * enusre the pixmaps are loaded
     */
    void load_pixmaps() const;
    
    
    /**
     * blinkg enabled or not
     */
    bool m_bBlinking;
    
    
    /**
     * blinking back state (target to blink when "off")
     */
    led_state m_eBlinkingBackState;
    
    
    /**
     * rate of blinking [1 - 10Hz]
     */
    uint64_t m_nBlinkingHertz;
    
    
    /**
     * timestamp of last (good) blinking
     */
    std::chrono::system_clock::time_point m_cBlinkingLast;


    /**
     * the current state in blinking
     */
    bool m_bBlinkOn;
    
    
    /**
     * the led state
     */
    led_state m_eState;
    
    
    /**
     * the text beside the LED
     */
    QString m_sText;
    
    
    /**
     * is the text visible
     */
    bool m_bTextVisible;
};


}
}

#endif

