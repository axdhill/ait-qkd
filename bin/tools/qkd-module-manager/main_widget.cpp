/*
 * main_widget.cpp
 * 
 * declares the main widget for the QKD Module Manager
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

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// ait
#include <qkd/module/module.h>
#include <qkd/utility/dbus.h>
#include <qkd/utility/investigation.h>
#include <qkd/widget/module_frame.h>
#include <qkd/widget/res.h>

#include "main_widget.h"


using namespace qkd::module_manager;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cParent     parent object
 */
main_widget::main_widget(QMainWindow * cParent) : QFrame(cParent) {
    
    setupUi(this);
    
    QStringList cHeaderLabels;
    cHeaderLabels << QApplication::translate("main_widget", "ID", 0, QApplication::UnicodeUTF8);
    cHeaderLabels << QApplication::translate("main_widget", "DBus", 0, QApplication::UnicodeUTF8);
    cHeaderLabels << QApplication::translate("main_widget", "Type", 0, QApplication::UnicodeUTF8);    
    cHeaderLabels << QApplication::translate("main_widget", "Status", 0, QApplication::UnicodeUTF8);
    cHeaderLabels << QApplication::translate("main_widget", "Pipeline", 0, QApplication::UnicodeUTF8);
    cHeaderLabels << QApplication::translate("main_widget", "Role", 0, QApplication::UnicodeUTF8);
    cTvModules->setHeaderLabels(cHeaderLabels);
    
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_PRESIFTING]             = qkd::widget::res::pixmap("module_presifting").scaledToHeight(22);
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_SIFTING]                = qkd::widget::res::pixmap("module_sifting").scaledToHeight(22);
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_ERROR_ESTIMATION]       = qkd::widget::res::pixmap("module_error_estimation").scaledToHeight(22);
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_ERROR_CORRECTION]       = qkd::widget::res::pixmap("module_error_correction").scaledToHeight(22);
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_CONFIRMATION]           = qkd::widget::res::pixmap("module_confirmation").scaledToHeight(22);
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_PRIVACY_AMPLIFICATION]  = qkd::widget::res::pixmap("module_privacy_amplification").scaledToHeight(22);
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_KEYSTORE]               = qkd::widget::res::pixmap("module_keystore").scaledToHeight(22);
    m_cTypeIcon[(uint8_t)qkd::module::module_type::TYPE_OTHER]                  = qkd::widget::res::pixmap("module_other").scaledToHeight(22);

    m_cRoleIcon[0] = qkd::widget::res::pixmap("alice").scaledToHeight(22);
    m_cRoleIcon[1] = qkd::widget::res::pixmap("bob").scaledToHeight(22);
    
    m_cPipelineStart = qkd::widget::res::pixmap("media_playback_start").scaledToHeight(22);
    m_cPipelineStop = qkd::widget::res::pixmap("media_playback_stop").scaledToHeight(22);
    cBtnPipelineStart->setIcon(m_cPipelineStart);
    cBtnPipelineStop->setIcon(m_cPipelineStop);
    
    connect(cCbPipeline, SIGNAL(editTextChanged(const QString &)), SLOT(pipeline_changed(const QString &)));
    connect(cBtnPipeline, SIGNAL(clicked()), SLOT(select_pipeline_file()));
    connect(cBtnPipelineStart, SIGNAL(clicked()), SLOT(pipeline_start()));
    connect(cBtnPipelineStop, SIGNAL(clicked()), SLOT(pipeline_stop()));
    connect(cBtnQuit, SIGNAL(clicked()), SIGNAL(quit()));
    connect(cTvModules, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(module_list_current_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
    
    QTimer * cTimer = new QTimer(this);
    connect(cTimer, SIGNAL(timeout()), SLOT(timeout()));
    cTimer->start(250);
}


/**
 * dtor
 */
main_widget::~main_widget() {
}


/**
 * adds a module widget
 * 
 * @param   sDBus                   DBus address of module
 */
void main_widget::add_module_widget(std::string const & sDBus) {
    
    if (m_cModuleFrame.find(sDBus) != m_cModuleFrame.end()) return;
    
    m_cModuleFrame[sDBus] = new qkd::widget::module_frame(cStModules, qkd::utility::dbus::qkd_dbus());
    connect(m_cModuleFrame[sDBus]->tab(), SIGNAL(currentChanged(int)), SLOT(module_tab_index(int)));
    cStModules->addWidget(m_cModuleFrame[sDBus]);
}


/**
 * load widget settings
 * 
 * @param   cSettings       settings object containing our state
 */
void main_widget::load_settings(QSettings const & cSettings) {
    
    cSpMain->restoreState(cSettings.value("main_widget_splitter").toByteArray());
    
    QByteArray cModuleListData = cSettings.value("main_widget_modules").toByteArray();
    QDataStream cStream(&cModuleListData, QIODevice::ReadOnly);
    
    bool bSorting;
    cStream >> bSorting;
    int nSortColumn;
    cStream >> nSortColumn;
    if (bSorting) cTvModules->sortItems(nSortColumn, Qt::AscendingOrder);
    
    QList<int> l;
    cStream >> l;
    
    for (int i = 0; i < l.size(); ++i) {
        if (i < cTvModules->columnCount()) cTvModules->setColumnWidth(i, l.at(i));
    }
    
    cCbPipeline->addItems(cSettings.value("pipeline_file").toStringList());
}
    
    
/**
 * a new item is set to be the current one in the module list
 * 
 * @param   cCurrent        the new item in the list set current
 * @param   cPrevious       old item
 */
void main_widget::module_list_current_changed(QTreeWidgetItem * cCurrent, UNUSED QTreeWidgetItem * cPrevious) {
    if (cCurrent == nullptr) return;
    show_module(cCurrent->text(1).toStdString());
}


/**
 * set a new module tab for all modules
 * 
 * @param   nIndex      the new module tab
 */
void main_widget::module_tab_index(int nIndex) {

    for (auto iter : m_cModuleFrame) {
        QTabWidget * cTab = iter.second->tab();
        if ((cTab->count() <= nIndex) && (cTab->currentIndex() != nIndex)) {
            cTab->setCurrentIndex(2);
        }
    }
}


/**
 * pipeline text changed
 * 
 * @param   sText       the new pipeline text
 */
void main_widget::pipeline_changed(QString const & sText) {
    cBtnPipelineStart->setEnabled(!sText.isEmpty());
    cBtnPipelineStop->setEnabled(!sText.isEmpty());
}


/**
 * start a pipeline
 */
void main_widget::pipeline_start() {
    
    std::list<boost::filesystem::path> cSearchPaths = { "." };
    std::list<boost::filesystem::path> cPipelineCommands = qkd::utility::environment::find_files("qkd-pipeline", cSearchPaths, true, true, true, true);
    if (cPipelineCommands.size() == 0) {
        QMessageBox::critical(this, "AIT QKD Module Manager", tr("Cannot locate path to qkd-pipeline command.\nAborting."));
        return;
    }
    
    std::string sPipelineCommand = cPipelineCommands.front().string();
    qkd::utility::debug() << "using '" << sPipelineCommand << "' as pipeline command";
    
}


/**
 * stop a pipeline
 */
void main_widget::pipeline_stop() {
}


/**
 * removes a module widget
 * 
 * @param   sDBus                   DBus address of module
 */
void main_widget::remove_module_widget(std::string const & sDBus) {
    
    if (m_cModuleFrame.find(sDBus) == m_cModuleFrame.end()) return;
    
    cStModules->removeWidget(m_cModuleFrame[sDBus]);
    m_cModuleFrame.erase(sDBus);
}


/**
 * save widget settings
 * 
 * @param   cSettings       settings object to receive our state
 */
void main_widget::save_settings(QSettings & cSettings) const {
    
    cSettings.setValue("main_widget_splitter", cSpMain->saveState());
    
    QByteArray cModuleListData;
    QDataStream cStream(&cModuleListData, QIODevice::WriteOnly);
    
    cStream << cTvModules->isSortingEnabled();
    cStream << cTvModules->sortColumn();
    
    QList<int> l;
    for (int i = 0; i < cTvModules->columnCount(); ++i) l.append(cTvModules->columnWidth(i));
    cStream << l;
    cSettings.setValue("main_widget_modules", cModuleListData);
    
    QStringList cPipelineFiles;
    for (int i = 0; i < cCbPipeline->count(); ++i) cPipelineFiles << cCbPipeline->itemText(i);
    cSettings.setValue("pipeline_file", cPipelineFiles);
    cSettings.setValue("pipeline_file_last", cCbPipeline->currentText());
}


/**
 * select a pipeline file
 */
void main_widget::select_pipeline_file() {
    
    QDir cDirectory = QDir::home();
    if (!cCbPipeline->currentText().isEmpty()) cDirectory = QDir(cCbPipeline->currentText()); 

    QString sFile = QFileDialog::getOpenFileName(this, tr("Open Pipeline Config File"), cDirectory.path());
    if (sFile.isEmpty()) return;
    
    cCbPipeline->addItem(sFile);
    cCbPipeline->setCurrentIndex(cCbPipeline->findText(sFile));
}


/**
 * show module specified by the dbus address
 * 
 * @param   sDBus       the dbus address of the module
 */
void main_widget::show_module(std::string sDBus) {
    
    auto iter = m_cModuleFrame.find(sDBus);
    if (iter == m_cModuleFrame.end()) {
        qkd::utility::debug() << "requested module frame for '" << sDBus << "' but module frame does not exist.";
        return;
    }
    
    // pop module frame to front
    cStModules->setCurrentWidget((*iter).second);
}


/**
 * make up system update
 */
void main_widget::timeout() {
    
    static uint64_t nUpdateCycle = 0;
    nUpdateCycle++;

    qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();
    
    for (auto const & cModulePair : cInvestigation.modules()) {
        
        qkd::utility::properties const & cModule = cModulePair.second;
        
        QTreeWidgetItem * cItem = nullptr;
        auto iter = m_cModuleTreeWidgetItems.find(cModule.at("dbus"));
        if (iter == m_cModuleTreeWidgetItems.end()) {
            
            // new module
            cItem = new QTreeWidgetItem;
            cTvModules->addTopLevelItem(cItem);
            m_cModuleTreeWidgetItems[cModule.at("dbus")] = cItem;
            add_module_widget(cModule.at("dbus"));
        }
        else cItem = (*iter).second;
        
        uint8_t nTypeId = std::stoi(cModule.at("type"));
        uint8_t nRoleId = std::stoi(cModule.at("role"));
        
        cItem->setText(0, QString::fromStdString(cModule.at("id")));
        cItem->setText(1, QString::fromStdString(cModule.at("dbus")));
        cItem->setText(2, QString::fromStdString(cModule.at("type_name")));
        if (nTypeId < 8) cItem->setIcon(2, m_cTypeIcon[nTypeId]);
        cItem->setText(3, QString::fromStdString(cModule.at("state_name")));
        cItem->setText(4, QString::fromStdString(cModule.at("pipeline")));
        cItem->setText(5, QString::fromStdString(cModule.at("role_name")));
        if (nRoleId < 2) cItem->setIcon(5, m_cRoleIcon[nRoleId]);
        
        m_cModuleUpdateCycle[cModule.at("dbus")] = nUpdateCycle;
        
        update_module_widget(cModule);
    }
    
    // walk over update cycle and collect those not updated
    // --> they are not part of the system
    std::list<std::string> cModulesToDelete;
    for (auto const & cCycleData : m_cModuleUpdateCycle) {
        
        if (cCycleData.second == nUpdateCycle) continue;
        cModulesToDelete.push_back(cCycleData.first);
    }
    for (auto const & sDBusAddress : cModulesToDelete) {
        
        remove_module_widget(sDBusAddress);
        delete m_cModuleTreeWidgetItems[sDBusAddress];
        m_cModuleTreeWidgetItems.erase(sDBusAddress);
        m_cModuleUpdateCycle.erase(sDBusAddress);
    }
}


/**
 * updates a module widget
 * 
 * @param   cModuleProperties       set of module properties
 */
void main_widget::update_module_widget(qkd::utility::properties const & cModuleProperties) {
    
    auto iter = m_cModuleFrame.find(cModuleProperties.at("dbus"));
    if (iter == m_cModuleFrame.end()) return;
    (*iter).second->update(cModuleProperties);
}
