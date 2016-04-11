/*
 * main_window.cpp
 * 
 * implements the main window for QKD module-manager
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
// incs

#include <boost/filesystem.hpp>

// Qt
#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QStatusBar>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>

// ait
#include <qkd/utility/environment.h>

#include "main_widget.h"
#include "main_window.h"


using namespace qkd::module_manager;


// ------------------------------------------------------------
// code


/**
 * ctor
 */
main_window::main_window() : QMainWindow(), m_cMainWidget(nullptr) {
    
    setWindowTitle(tr("AIT QKD Module Manager V%1").arg(VERSION));
    
    boost::filesystem::path cDataPath = qkd::utility::environment::data_path("qkd-module-manager");
    QString sDataPath = QString::fromStdString(cDataPath.string());
    
    QPixmap cPixAITLogo = QPixmap(sDataPath + QDir::separator() + "ait_logo.png");
    if (!cPixAITLogo.isNull()) setWindowIcon(QIcon(cPixAITLogo));
    
    QScrollArea * cScrMain = new QScrollArea(this);
    m_cMainWidget = new main_widget(this);
    cScrMain->setWidget(m_cMainWidget);
    cScrMain->setWidgetResizable(true);
    setCentralWidget(cScrMain);
    
    load_settings();
    statusBar()->showMessage("ready");
    
    connect(m_cMainWidget, SIGNAL(quit()), SLOT(quitApp()));
}


/**
 * centers the window on the desktop with default width and height
 */
void main_window::center_window() {
    
    QSize cMinimumSize = QSize(800, 600);
    QDesktopWidget* cDesktop = qApp->desktop();
    
    int nDefaultWidth = std::min<int>(cDesktop->width(), cMinimumSize.width());
    int nDefaultHeight = std::min<int>(cDesktop->height(), cMinimumSize.height());
    int nX = (cDesktop->width() - nDefaultWidth) / 2;
    int nY = (cDesktop->height() - nDefaultHeight) / 2;
    
    resize(QSize(nDefaultWidth, nDefaultHeight));
    move(QPoint(nX, nY));
}


/**
 * handle close event
 *
 * @param   cEvent      the event passed
 */
void main_window::closeEvent(QCloseEvent* cEvent) {
    save_settings();
    QMainWindow::closeEvent(cEvent);
}


/**
 * load window settings
 */
void main_window::load_settings() {
    
    QSettings cSettings("AIT", "qkd-module-manager");
    if (cSettings.contains("geometry")) restoreGeometry(cSettings.value("geometry").toByteArray());
    else center_window();
    restoreState(cSettings.value("window_state").toByteArray());
    
    m_cMainWidget->load_settings(cSettings);
}

    
/**
 * quit app
 */
void main_window::quitApp() {
    close();
}


/**
 * save window settings
 */
void main_window::save_settings() const {
    
    QSettings cSettings("AIT", "qkd-module-manager");
    cSettings.setValue("geometry", saveGeometry());
    cSettings.setValue("window_state", saveState());
    
    m_cMainWidget->save_settings(cSettings);
}


