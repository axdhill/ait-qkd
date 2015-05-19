/*
 * about_dialog.cpp
 * 
 * declares the about dialog for QKD Simulate
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

 
// ------------------------------------------------------------
// incs

#include <boost/filesystem.hpp>

// Qt
#include <QDir>
#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

// ait
#include <qkd/utility/environment.h>

#include "about_dialog.h"


using namespace qkd::simulate;


// ------------------------------------------------------------
// vars


/**
 * the about text
 */
QString g_sAboutText = "\
<html>\
<body bgcolor=\"#FF00FF\">\
\
<div align=\"center\">\
<p/>\
<img src=\"image:ait_logo_no_claim.jpg\"/>\
<p/>\
<h1>QKD Simulate V%1</h1>\
</div>\
<p/>\
This program simulates a real quantum channel and<br/>\
creates event tables for alice and bob <br/>\
like the real quantum channel.<br/>\
\
<p>\
Copyright (C) 2013-2015, AIT Austrian Institute of Technology<br/>\
AIT Austrian Institute of Technology GmbH<br/>\
Donau-City-Strasse 1 | 1220 Vienna | Austria<br/>\
<a href=\"http://www.ait.ac.at\">http://www.ait.ac.at</a>\
</p>\
</div>\
\
</body>\
</html>\
";


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cParent     parent widget
 */
about_dialog::about_dialog(QWidget * cParent) : QDialog(cParent) {
    
    setWindowTitle(tr("AIT QKD Simulate V%1").arg(VERSION));
    
    setup_widget();
    
    resize(500, 400);
}


/**
 * setup the widget
 */
void about_dialog::setup_widget() {
    
    // define search path
    boost::filesystem::path cDataPath = qkd::utility::environment::data_path("qkd-simulate");
    QString sDataPath = QString::fromStdString(cDataPath.string());
    QDir::setSearchPaths("image", QStringList(sDataPath));
    
    // setup widgets
    QVBoxLayout * cLyMain = new QVBoxLayout;
    cLyMain->setContentsMargins(8, 4, 8, 4);
    setLayout(cLyMain);

    QLabel * cLbAboutText = new QLabel;
    cLbAboutText->setStyleSheet("QWidget { background: white; color: black; };");
    cLbAboutText->setText(g_sAboutText.arg(VERSION));
    cLbAboutText->setContentsMargins(12, 12, 12, 12);
    
    QScrollArea * cScrollArea = new QScrollArea;
    cScrollArea->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    cScrollArea->setWidget(cLbAboutText);    
    cScrollArea->viewport()->setStyleSheet("QWidget { background: white; color: black; };");
    cLyMain->addWidget(cScrollArea);
    
    QHBoxLayout * cLyButtons = new QHBoxLayout;
    cLyMain->addLayout(cLyButtons);
    
    cLyButtons->addStretch(1);
    
    QPushButton * cBtDone = new QPushButton(tr("Done"));
    cLyButtons->addWidget(cBtDone);
    
    // connectors
    connect(cBtDone, SIGNAL(clicked(bool)), SLOT(accept()));
}
