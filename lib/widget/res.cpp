/*
 * res.cpp
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

 
// ------------------------------------------------------------
// include

#include <iostream>

// Qt
#include <QtGui/QBoxLayout>
#include <QtGui/QFontDatabase>
#include <QtGui/QPixmapCache>
#include <QtGui/QWidget>

// ait
#include <qkd/widget/res.h>

// get the resources as source
#include "lib/res_alice.png.h"
#include "lib/res_bob.png.h"
#include "lib/res_glass_button_green.png.h"
#include "lib/res_glass_button_green_small.png.h"
#include "lib/res_glass_button_grey.png.h"
#include "lib/res_glass_button_grey_small.png.h"
#include "lib/res_glass_button_red.png.h"
#include "lib/res_glass_button_red_small.png.h"
#include "lib/res_glass_button_yellow.png.h"
#include "lib/res_glass_button_yellow_small.png.h"
#include "lib/res_media_eject.png.h"
#include "lib/res_media_playback_pause.png.h"
#include "lib/res_media_playback_start.png.h"
#include "lib/res_media_playback_stop.png.h"
#include "lib/res_media_record.png.h"
#include "lib/res_module_confirmation.png.h"
#include "lib/res_module_error_correction.png.h"
#include "lib/res_module_error_estimation.png.h"
#include "lib/res_module_keystore.png.h"
#include "lib/res_module_other.png.h"
#include "lib/res_module_peer.png.h"
#include "lib/res_module_pipe_in.png.h"
#include "lib/res_module_pipe_out.png.h"
#include "lib/res_module_presifting.png.h"
#include "lib/res_module_privacy_amplification.png.h"
#include "lib/res_module_sifting.png.h"
#include "lib/res_module.png.h"
#include "lib/res_wwDigital.ttf.h"


using namespace qkd::widget;


// ------------------------------------------------------------
// 
// decl


// fwd
static void load_pixmaps();


// ------------------------------------------------------------
// code


/**
 * get the LCD font with a given point size
 * 
 * @param   nPointSize      pointsize of the LCD font
 * @return  a LCD font
 */
QFont res::lcd_font(int nPointSize) {
    
    // Qt LCD widget is
    //      ... just plain ugly.
    //
    // provide a suited LCD font here
    
    static bool bLoaded = false;
    if (!bLoaded) {
        
        // load the proper LCD Font
        bLoaded = true;
        QByteArray cFontData((const char * )wwDigital_ttf, wwDigital_ttf_len); 
        int nLoadTTF = QFontDatabase::addApplicationFontFromData(cFontData);
        if (nLoadTTF) {
            std::cerr << "failed to load LCD font" << std::endl;
        }
    }
    
    return QFont("WW Digital", nPointSize);    
}


/**
 * get a pixmap based on the given id
 * 
 * if the pixmap has not been found, the returned pixmap
 * is empty (isNull() == true)
 * 
 * @param   sID         the id of the pixmap in question
 * @return  the pixmap
 */
QPixmap res::pixmap(QString const & sID) {
    
    load_pixmaps();
    
    QPixmap cPixmap;
    QPixmapCache::find(sID, &cPixmap);
    
    return cPixmap;
}


/**
 * load the pixmaps into the pixmap cache
 */
void load_pixmaps() {
    
    static bool bLoaded = false;
    if (bLoaded) return;
    
    QPixmapCache::insert("alice",                           QPixmap::fromImage(QImage::fromData(alice_png, alice_png_len)));
    QPixmapCache::insert("bob",                             QPixmap::fromImage(QImage::fromData(bob_png, bob_png_len)));
    QPixmapCache::insert("glass_button_green",              QPixmap::fromImage(QImage::fromData(glass_button_green_png, glass_button_green_png_len)));
    QPixmapCache::insert("glass_button_green_small",        QPixmap::fromImage(QImage::fromData(glass_button_green_small_png, glass_button_green_small_png_len)));
    QPixmapCache::insert("glass_button_grey",               QPixmap::fromImage(QImage::fromData(glass_button_grey_png, glass_button_grey_png_len)));
    QPixmapCache::insert("glass_button_grey_small",         QPixmap::fromImage(QImage::fromData(glass_button_grey_small_png, glass_button_grey_small_png_len)));
    QPixmapCache::insert("glass_button_red",                QPixmap::fromImage(QImage::fromData(glass_button_red_png, glass_button_red_png_len)));
    QPixmapCache::insert("glass_button_red_small",          QPixmap::fromImage(QImage::fromData(glass_button_red_small_png, glass_button_red_small_png_len)));
    QPixmapCache::insert("glass_button_yellow",             QPixmap::fromImage(QImage::fromData(glass_button_yellow_png, glass_button_yellow_png_len)));
    QPixmapCache::insert("glass_button_yellow_small",       QPixmap::fromImage(QImage::fromData(glass_button_yellow_small_png, glass_button_yellow_small_png_len)));
    QPixmapCache::insert("media_eject",                     QPixmap::fromImage(QImage::fromData(media_eject_png, media_eject_png_len)));
    QPixmapCache::insert("media_playback_pause",            QPixmap::fromImage(QImage::fromData(media_playback_pause_png, media_playback_pause_png_len)));
    QPixmapCache::insert("media_playback_start",            QPixmap::fromImage(QImage::fromData(media_playback_start_png, media_playback_start_png_len)));
    QPixmapCache::insert("media_playback_stop",             QPixmap::fromImage(QImage::fromData(media_playback_stop_png, media_playback_stop_png_len)));
    QPixmapCache::insert("media_record",                    QPixmap::fromImage(QImage::fromData(media_record_png, media_record_png_len)));

    QPixmapCache::insert("module_confirmation",             QPixmap::fromImage(QImage::fromData(module_confirmation_png, module_confirmation_png_len)));
    QPixmapCache::insert("module_error_correction",         QPixmap::fromImage(QImage::fromData(module_error_correction_png, module_error_correction_png_len)));
    QPixmapCache::insert("module_error_estimation",         QPixmap::fromImage(QImage::fromData(module_error_estimation_png, module_error_estimation_png_len)));
    QPixmapCache::insert("module_keystore",                 QPixmap::fromImage(QImage::fromData(module_keystore_png, module_keystore_png_len)));
    QPixmapCache::insert("module_other",                    QPixmap::fromImage(QImage::fromData(module_other_png, module_other_png_len)));
    QPixmapCache::insert("module_peer",                     QPixmap::fromImage(QImage::fromData(module_peer_png, module_peer_png_len)));
    QPixmapCache::insert("module_pipe_in",                  QPixmap::fromImage(QImage::fromData(module_pipe_in_png, module_pipe_in_png_len)));
    QPixmapCache::insert("module_pipe_out",                 QPixmap::fromImage(QImage::fromData(module_pipe_out_png, module_pipe_out_png_len)));
    QPixmapCache::insert("module_presifting",               QPixmap::fromImage(QImage::fromData(module_presifting_png, module_presifting_png_len)));
    QPixmapCache::insert("module_privacy_amplification",    QPixmap::fromImage(QImage::fromData(module_privacy_amplification_png, module_privacy_amplification_png_len)));
    QPixmapCache::insert("module_sifting",                  QPixmap::fromImage(QImage::fromData(module_sifting_png, module_sifting_png_len)));

    QPixmapCache::insert("module",                          QPixmap::fromImage(QImage::fromData(module_png, module_png_len)));
    
    bLoaded = true;
}


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
QWidget * res::swap_widget(QBoxLayout * cLayout, QWidget * cWidgetOld, QWidget * cWidgetNew) {
    
    // sanity check
    if (!cWidgetOld) return nullptr;
    if (!cWidgetNew) return nullptr;

    // locate widget in layout
    int nIndex = -1;
    if (cLayout) nIndex = cLayout->indexOf(cWidgetOld);
    int nStretch = 0;
    if (nIndex >= 0) nStretch = cLayout->stretch(nIndex);

    // replace with new widget
    cWidgetNew->setParent(dynamic_cast<QWidget *>(cWidgetOld->parent()));
    cWidgetNew->setSizePolicy(cWidgetOld->sizePolicy());
    cWidgetNew->setMinimumSize(cWidgetOld->minimumSize());
    cWidgetNew->setMaximumSize(cWidgetOld->maximumSize());
    cWidgetNew->resize(cWidgetOld->size());
    if (nIndex >= 0) cLayout->insertWidget(nIndex, cWidgetNew, nStretch);
    
    delete cWidgetOld;
    
    if (cLayout) cLayout->update();
    
    return cWidgetNew;
}
