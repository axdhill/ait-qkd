/*
 * main_widget.cpp
 * 
 * declares the main widget for the QKD Simulate
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
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

#include <iostream>

// Qt
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

// ait
#include <qkd/common_macros.h>

#include "default_values.h"
#include "main_widget.h"
#include "channel/channel_bb84.h"
#include "channel/detector/detection_modes.h"


using namespace qkd::simulate;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cParent     parent object
 */
main_widget::main_widget(QMainWindow * cParent) : QFrame(cParent) {
    
    // setup this widget
    setupUi(this);
    
    // create the channel
    m_cChannel = new channel_bb84();
    
    // setup sub widgets
    m_cDgAbout = new about_dialog(this);
    m_cDgAbout->setModal(true);
    
    // load the default values
    QDomDocument cDomDoc;
    cDomDoc.setContent(g_sDefaultValues);
    load_xml(cDomDoc);
    
    // connectors
    connect(cEdFiberAbsorptionCoeff, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdFiberLength, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdMultiPhotonRate, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdNoisePhotonRate, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdSourcePhotonRate, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdSourceSigErrProb, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdSimEndTime, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdSyncStandDeviation, SIGNAL(textChanged(QString)), SLOT(update_values()));
    
    connect(cCkMultPhotSim, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkSyncPulse, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkTransmLoss, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkLoopSimulation, SIGNAL(stateChanged(int)), SLOT(update_values()));

    connect(cCkDarkCountsAlice, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkDarkCountsBob, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkDetLossAlice, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkDetLossBob, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkJitterSimAlice, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkJitterSimBob, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkWaitForSyncInitiatorAlice, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkWaitForAllDetectorsAlice, SIGNAL(stateChanged(int)), SLOT(update_values()));
    connect(cCkWaitForAllDetectorsBob, SIGNAL(stateChanged(int)), SLOT(update_values()));

    connect(cEdDetectionEffAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetDarkCountRateAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetDownTimeAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetTimeStndDeviationAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetTimeDelayAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDistanceIndepLossAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdTimeSlotWidthAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdTimeSlotDelayAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdTableSizeAlice, SIGNAL(textChanged(QString)), SLOT(update_values()));

    connect(cEdDetectionEffBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetDarkCountRateBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetDownTimeBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetTimeStndDeviationBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDetTimeDelayBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdDistanceIndepLossBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdTimeSlotWidthBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdTimeSlotDelayBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    connect(cEdTableSizeBob, SIGNAL(textChanged(QString)), SLOT(update_values()));
    
    connect(cRdFreeUDP, SIGNAL(clicked(bool)), SLOT(update_values()));
    connect(cRdFreeFile, SIGNAL(clicked(bool)), SLOT(update_values()));
    connect(cRdEventPipe, SIGNAL(clicked(bool)), SLOT(update_values()));
    connect(cRdEventFile, SIGNAL(clicked(bool)), SLOT(update_values()));
    
    connect(cBtnFreeFileAlice, SIGNAL(clicked(bool)), SLOT(clicked_select_free_file_alice()));
    connect(cBtnFreeFileBob, SIGNAL(clicked(bool)), SLOT(clicked_select_free_file_bob()));
    connect(cBtnEventFileAlice, SIGNAL(clicked(bool)), SLOT(clicked_select_event_file_alice()));
    connect(cBtnEventFileBob, SIGNAL(clicked(bool)), SLOT(clicked_select_event_file_bob()));

    connect(cBtnAbout, SIGNAL(clicked()), SLOT(clicked_about()));
    connect(cBtnDefault, SIGNAL(clicked()), SLOT(clicked_default()));
    connect(cBtnLoad, SIGNAL(clicked()), SLOT(clicked_load()));
    connect(cBtnSave, SIGNAL(clicked()), SLOT(clicked_save()));
    connect(cBtnStart, SIGNAL(clicked()), SLOT(clicked_start()));
    connect(cBtnStop, SIGNAL(clicked()), SLOT(clicked_stop()));
    connect(cBtnQuit, SIGNAL(clicked()), SIGNAL(quit()));
    connect(cBtnDumpParameters, SIGNAL(clicked()), SLOT(clicked_dump_parameters()));
    
    // enforce widgets states
    check_ui();
    
    // install our update timer
    QTimer * cTimer = new QTimer(this);
    cTimer->setInterval(100);
    connect(cTimer, SIGNAL(timeout()), SLOT(update_simulation_view()));
    cTimer->start();
}


/**
 * dtor
 */
main_widget::~main_widget() {
    if (m_cChannel) {
        delete m_cChannel;
        m_cChannel = nullptr;
    }
}


/**
 * check current widget states
 */
void main_widget::check_ui() {

    // this implementation currently does not support multi photons yet ..
    
    cLbMultiPhotonRate->setEnabled(false);
    cEdMultiPhotonRate->setEnabled(false);
    cLbMultiPhotonRateUnit->setEnabled(false);
    cCkMultPhotSim->setEnabled(false);

    // TODO: Don't know why this is disabled
    // gotta ask Andreas Poppe why
    cLbTimeSlotDelayAlice->setEnabled(false);
    cEdTimeSlotDelayAlice->setEnabled(false);
    cLbTimeSlotDelayAliceUnit->setEnabled(false);
    
    // automatic widget enabling/disabling
    cCkWaitForSyncInitiatorAlice->setEnabled(cCkSyncPulse->isChecked());
    cCkWaitForAllDetectorsAlice->setEnabled(cCkSyncPulse->isChecked());
    cCkWaitForAllDetectorsBob->setEnabled(cCkSyncPulse->isChecked());
    
    try {
        
        // sync pulse or free running
        if (cCkSyncPulse->isChecked()) {
            
            // wait for all detectors
            if (cCkWaitForAllDetectorsAlice->isChecked()) {
                cCkWaitForSyncInitiatorAlice->setEnabled(false);
                m_cChannel->alice()->set_detection_mode(detection_mode::SYNC_ALL_READY);
            }
            else 
            if (cCkWaitForSyncInitiatorAlice->isChecked()) {
                m_cChannel->alice()->set_detection_mode(detection_mode::SYNC_INITIATOR_READY);
            }
            else {
                m_cChannel->alice()->set_detection_mode(detection_mode::SYNC);
            }
            
            if (cCkWaitForAllDetectorsBob->isChecked()) {
                m_cChannel->bob()->set_detection_mode(detection_mode::SYNC_ALL_READY);
            }
            else {
                m_cChannel->bob()->set_detection_mode(detection_mode::SYNC);
            }
        }
        else {
            m_cChannel->alice()->set_detection_mode(detection_mode::FREE_RUNNING);
            m_cChannel->bob()->set_detection_mode(detection_mode::FREE_RUNNING); 
        }
        
        // set infinite looping
        m_cChannel->set_looping(cCkLoopSimulation->isChecked());
        
    }
    catch (UNUSED std::exception & cException) { }

    // source
    cEdSimEndTime->setEnabled(!(cCkSyncPulse->isChecked()));
    cLbTableSizeAlice->setEnabled(cCkSyncPulse->isChecked());
    cEdTableSizeAlice->setEnabled(cCkSyncPulse->isChecked());
    cLbTableSizeAliceUnit->setEnabled(cCkSyncPulse->isChecked());
    cLbTableSizeBob->setEnabled(cCkSyncPulse->isChecked());
    cEdTableSizeBob->setEnabled(cCkSyncPulse->isChecked());
    cLbTableSizeBobUnit->setEnabled(cCkSyncPulse->isChecked());
    
    // alice
    cEdFiberAbsorptionCoeff->setEnabled(cCkTransmLoss->isChecked());
    cLbFiberAbsorptionCoeff->setEnabled(cEdFiberAbsorptionCoeff->isEnabled());
    cLbFiberAbsorptionCoeffUnit->setEnabled(cEdFiberAbsorptionCoeff->isEnabled());
    
    cEdDistanceIndepLossAlice->setEnabled(cCkDetLossAlice->isChecked());
    cLbDistanceIndepLossAlice->setEnabled(cEdDistanceIndepLossAlice->isEnabled());
    cLbDistanceIndepLossAliceUnit->setEnabled(cEdDistanceIndepLossAlice->isEnabled());

    cEdDetectionEffAlice->setEnabled(cCkDetLossAlice->isChecked());
    cLbDetectionEffAlice->setEnabled(cEdDetectionEffAlice->isEnabled());
    cLbDetectionEffAliceUnit->setEnabled(cEdDetectionEffAlice->isEnabled());
    
    cEdDetDownTimeAlice->setEnabled(cCkDetLossAlice->isChecked());
    cLbDetDownTimeAlice->setEnabled(cEdDetDownTimeAlice->isEnabled());
    cLbDetDownTimeAliceUnit->setEnabled(cEdDetDownTimeAlice->isEnabled());
    
    cEdDetDarkCountRateAlice->setEnabled(cCkDarkCountsAlice->isChecked());
    cLbDetDarkCountRateAlice->setEnabled(cEdDetDarkCountRateAlice->isEnabled());
    cLbDetDarkCountRateAliceUnit->setEnabled(cEdDetDarkCountRateAlice->isEnabled());
    
    cEdDetTimeStndDeviationAlice->setEnabled(cCkJitterSimAlice->isChecked());
    cLbDetTimeStndDeviationAlice->setEnabled(cEdDetTimeStndDeviationAlice->isEnabled());
    cLbDetTimeStndDeviationAliceUnit->setEnabled(cEdDetTimeStndDeviationAlice->isEnabled());
    
    cEdDetTimeDelayAlice->setEnabled(cCkJitterSimAlice->isChecked());
    cLbDetTimeDelayAlice->setEnabled(cEdDetTimeDelayAlice->isEnabled());
    cLbDetTimeDelayAliceUnit->setEnabled(cEdDetTimeDelayAlice->isEnabled());
    
     // bob
    cEdDistanceIndepLossBob->setEnabled(cCkDetLossBob->isChecked());
    cLbDistanceIndepLossBob->setEnabled(cEdDistanceIndepLossBob->isEnabled());
    cLbDistanceIndepLossBobUnit->setEnabled(cEdDistanceIndepLossBob->isEnabled());
    
    cEdDetectionEffBob->setEnabled(cCkDetLossBob->isChecked());
    cLbDetectionEffBob->setEnabled(cEdDetectionEffBob->isEnabled());
    cLbDetectionEffBobUnit->setEnabled(cEdDetectionEffBob->isEnabled());

    cEdDetDownTimeBob->setEnabled(cCkDetLossBob->isChecked());
    cLbDetDownTimeBob->setEnabled(cEdDetDownTimeBob->isEnabled());
    cLbDetDownTimeBobUnit->setEnabled(cEdDetDownTimeBob->isEnabled());
    
    cEdDetDarkCountRateBob->setEnabled(cCkDarkCountsBob->isChecked());
    cLbDetDarkCountRateBob->setEnabled(cEdDetDarkCountRateBob->isEnabled());
    cLbDetDarkCountRateBobUnit->setEnabled(cEdDetDarkCountRateBob->isEnabled());
    
    cEdDetTimeStndDeviationBob->setEnabled(cCkJitterSimBob->isChecked());
    cLbDetTimeStndDeviationBob->setEnabled(cEdDetTimeStndDeviationBob->isEnabled());
    cLbDetTimeStndDeviationBobUnit->setEnabled(cEdDetTimeStndDeviationBob->isEnabled());

    cEdDetTimeDelayBob->setEnabled(cCkJitterSimBob->isChecked());
    cLbDetTimeDelayBob->setEnabled(cEdDetTimeDelayBob->isEnabled());
    cLbDetTimeDelayBobUnit->setEnabled(cEdDetTimeDelayBob->isEnabled());
    
    cRdFreeFile->setEnabled(!cCkSyncPulse->isChecked());
    cRdFreeUDP->setEnabled(!cCkSyncPulse->isChecked());
    cRdEventFile->setEnabled(cCkSyncPulse->isChecked());
    cRdEventPipe->setEnabled(cCkSyncPulse->isChecked());
    
    cEdFreeUDPAlice->setEnabled(cRdFreeUDP->isChecked() && !cCkSyncPulse->isChecked());
    cEdFreeUDPBob->setEnabled(cRdFreeUDP->isChecked() && !cCkSyncPulse->isChecked());
    cEdFreeFileAlice->setEnabled(cRdFreeFile->isChecked() && !cCkSyncPulse->isChecked());
    cBtnFreeFileAlice->setEnabled(cRdFreeFile->isChecked() && !cCkSyncPulse->isChecked());
    cEdFreeFileBob->setEnabled(cRdFreeFile->isChecked() && !cCkSyncPulse->isChecked());
    cBtnFreeFileBob->setEnabled(cRdFreeFile->isChecked() && !cCkSyncPulse->isChecked());
    
    cEdEventPipeAlice->setEnabled(cRdEventPipe->isChecked() && cCkSyncPulse->isChecked());
    cEdEventPipeBob->setEnabled(cRdEventPipe->isChecked() && cCkSyncPulse->isChecked());
    cEdEventFileAlice->setEnabled(cRdEventFile->isChecked() && cCkSyncPulse->isChecked());
    cBtnEventFileAlice->setEnabled(cRdEventFile->isChecked() && cCkSyncPulse->isChecked());
    cEdEventFileBob->setEnabled(cRdEventFile->isChecked() && cCkSyncPulse->isChecked());
    cBtnEventFileBob->setEnabled(cRdEventFile->isChecked() && cCkSyncPulse->isChecked());

    cBtnStop->setEnabled(!cBtnStart->isEnabled());
}


/**
 * about clicked
 */
void main_widget::clicked_about() {
    m_cDgAbout->exec();
}


/**
 * default clicked
 */
void main_widget::clicked_default() {
    QDomDocument cDomDoc;
    cDomDoc.setContent(g_sDefaultValues);
    load_xml(cDomDoc);
}


/**
 * dump channel parameters to text file
 */
void main_widget::clicked_dump_parameters() {
    
    // open up dialog
    std::string sFileName = QFileDialog::getSaveFileName(this, tr("Dump simulator parameters")).toStdString();
    if (sFileName.empty()) return;
    
    // write to file
    std::ofstream cStream;
    cStream.open(sFileName, std::ios_base::trunc);
    if (!cStream.good()) {
        std::cerr << "failed to open file " << sFileName << " for writing" << std::endl;
        return;
    }
    
    // write parameters and close
    m_cChannel->write_parameters(cStream);
    cStream.close();
}


/**
 * load clicked
 */
void main_widget::clicked_load() {
    
    // open up dialog
    QString sFileName = QFileDialog::getOpenFileName(this, tr("Load simulator configuration"));
    if (sFileName.isEmpty()) return;

    // pick file
    QFile cFile(sFileName);
    if (!cFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("qkd-simulate"), tr("failed to open %1").arg(sFileName));
        return;
    }
    
    // read in and parse
    QDomDocument cDomDoc;
    cDomDoc.setContent(&cFile);
    load_xml(cDomDoc);
    
    // log to the user
    std::cerr << "loaded file " << cFile.fileName().toStdString() << std::endl;
}


/**
 * save clicked
 */
void main_widget::clicked_save() {
    
    // open up dialog
    QString sFileName = QFileDialog::getSaveFileName(this, tr("Save simulator configuration"));
    if (sFileName.isEmpty()) return;
    
    // pick file
    QFile cFile(sFileName);
    if (!cFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "failed to open file " << cFile.fileName().toStdString() << " for writing" << std::endl;
        return;
    }
    
    // create xml doc
    QString sValue;
    sValue += "<qkd-simulate>\n";
    sValue += "    <source>\n";
    sValue += "        <source_photon_rate value=\"" + cEdSourcePhotonRate->text() + "\" />\n";
    sValue += "        <fiber_length value=\"" + cEdFiberLength->text() + "\" />\n";
    sValue += "        <fiber_absorption_coeff value=\"" + cEdFiberAbsorptionCoeff->text() + "\" />\n";
    sValue += "        <source_signal_error_probability value=\"" + cEdSourceSigErrProb->text() + "\" />\n";
    sValue += "        <sync_det_time_stnd_deviation value=\"" + cEdSyncStandDeviation->text() + "\" />\n"; // P.G.: changed cEdDetTimeStndDeviationAlice to cEdSyncStandDeviation
    sValue += "        <multi_photon_rate value=\"" + cEdMultiPhotonRate->text() + "\" />\n";
    sValue += "        <noise_photon_rate value=\"" + cEdNoisePhotonRate->text() + "\" />\n";
    sValue += "        <simulation_end_time value=\"" + cEdSimEndTime->text() + "\" />\n";
    sValue += "    </source>\n";
    sValue += "    <alice>\n";
    sValue += "        <detection_efficiency value=\"" + cEdDetectionEffAlice->text() + "\" />\n";
    sValue += "        <dark_count_rate value=\"" + cEdDetDarkCountRateAlice->text() + "\" />\n";
    sValue += "        <time_slot_width value=\"" + cEdTimeSlotWidthAlice->text() + "\" />\n";
    sValue += "        <time_slot_delay value=\"" + cEdTimeSlotDelayAlice->text() + "\" />\n";
    sValue += "        <distance_indep_loss value=\"" + cEdDistanceIndepLossAlice->text() + "\" />\n";
    sValue += "        <det_time_stnd_deviation value=\"" + cEdDetTimeStndDeviationAlice->text() + "\" />\n";
    sValue += "        <det_time_delay value=\"" + cEdDetTimeDelayAlice->text() + "\" />\n";
    sValue += "        <det_down_time value=\"" + cEdDetDownTimeAlice->text() + "\" />\n";
    sValue += "        <table_size value=\"" + cEdTableSizeAlice->text() + "\" />\n";
    sValue += "    </alice>\n";
    sValue += "    <bob>\n";
    sValue += "        <detection_efficiency value=\"" + cEdDetectionEffBob->text() + "\" />\n";
    sValue += "        <dark_count_rate value=\"" + cEdDetDarkCountRateBob->text() + "\" />\n";
    sValue += "        <time_slot_width value=\"" + cEdTimeSlotWidthBob->text() + "\" />\n";
    sValue += "        <time_slot_delay value=\"" + cEdTimeSlotDelayBob->text() + "\" />\n";
    sValue += "        <distance_indep_loss value=\"" + cEdDistanceIndepLossBob->text() + "\" />\n";
    sValue += "        <det_time_stnd_deviation value=\"" + cEdDetTimeStndDeviationBob->text() + "\" />\n";
    sValue += "        <det_time_delay value=\"" + cEdDetTimeDelayBob->text() + "\" />\n";
    sValue += "        <det_down_time value=\"" + cEdDetDownTimeBob->text() + "\" />\n";
    sValue += "        <table_size value=\"" + cEdTableSizeBob->text() + "\" />\n";
    sValue += "    </bob>\n";
    sValue += "    <general>\n";
    sValue += "        <multi_photon_simulation value=\"" + (cCkMultPhotSim->isChecked() ? QString("true") : QString("false")) + "\" />\n";
    sValue += "        <sync_pulse_simulation value=\"" + (cCkSyncPulse->isChecked() ? QString("true") : QString("false")) + "\" />\n";
    sValue += "        <transmission_loss_simulation value=\"" + (cCkTransmLoss->isChecked() ? QString("true") : QString("false")) + "\" />\n";
    sValue += "        <inifinte_loop_simulation value=\"" + (cCkLoopSimulation->isChecked() ? QString("true") : QString("false")) + "\" />\n";
    sValue += "    </general>\n";
    sValue += "    <output>\n";
    sValue += "        <free value=\"" + (cRdFreeFile->isChecked() ? QString("file") : QString("udp")) + "\" />\n";
    sValue += "        <free_udp_alice value=\"" + cEdFreeUDPAlice->text() + "\" />\n";
    sValue += "        <free_udp_bob value=\"" + cEdFreeUDPBob->text() + "\" />\n";
    sValue += "        <free_file_alice value=\"" + cEdFreeFileAlice->text() + "\" />\n";
    sValue += "        <free_file_bob value=\"" + cEdFreeFileBob->text() + "\" />\n";
    sValue += "        <event value=\"" + (cRdEventFile->isChecked() ? QString("file") : QString("pipe")) + "\" />\n";
    sValue += "        <event_pipe_alice value=\"" + cEdEventPipeAlice->text() + "\" />\n";
    sValue += "        <event_pipe_bob value=\"" + cEdEventPipeBob->text() + "\" />\n";
    sValue += "        <event_file_alice value=\"" + cEdEventFileAlice->text() + "\" />\n";
    sValue += "        <event_file_bob value=\"" + cEdEventFileBob->text() + "\" />\n";
    sValue += "    </output>\n";
    sValue += "</qkd-simulate>\n";
    
    // write file
    QTextStream cStream(&cFile);
    cStream << sValue;
    
    // log to the user
    std::cerr << "saved file " << cFile.fileName().toStdString() << std::endl;
}


/**
 * clicked select event file Alice
 */
void main_widget::clicked_select_event_file_alice() {
    
    // open up dialog
    QString sFileName = QFileDialog::getSaveFileName(this, tr("Save Alice's event stream to file"));
    if (sFileName.isEmpty()) return;

    // set file
    cEdEventFileAlice->setText(sFileName);
}


/**
 * clicked select event file Bob
 */
void main_widget::clicked_select_event_file_bob() {
    
    // open up dialog
    QString sFileName = QFileDialog::getSaveFileName(this, tr("Save Bob's event stream to file"));
    if (sFileName.isEmpty()) return;

    // set file
    cEdEventFileBob->setText(sFileName);
}


/**
 * clicked select free file Alice
 */
void main_widget::clicked_select_free_file_alice() {
    
    // open up dialog
    QString sFileName = QFileDialog::getSaveFileName(this, tr("Save Alice's TTM signals to file"));
    if (sFileName.isEmpty()) return;

    // set file
    cEdFreeFileAlice->setText(sFileName);
}


/**
 * clicked select free file Bob
 */
void main_widget::clicked_select_free_file_bob() {
    
    // open up dialog
    QString sFileName = QFileDialog::getSaveFileName(this, tr("Save Bob's TTM signals to file"));
    if (sFileName.isEmpty()) return;

    // set file
    cEdFreeFileBob->setText(sFileName);
}


/**
 * start clicked
 */
void main_widget::clicked_start() {
    
    // setup output
    if (cCkSyncPulse->isChecked()) {
        
        // don't tell the TTM to post any data
        m_cChannel->ttm().set_output_mode(ttm::OUTPUT_MODE_NONE);
        
        // sync pulse events --> channel
        if (cRdEventPipe->isChecked()) {
            m_cChannel->set_piping(true);
            m_cChannel->set_pipe_alice(cEdEventPipeAlice->text().toStdString());
            m_cChannel->set_pipe_bob(cEdEventPipeBob->text().toStdString());
        }
        else
        if (cRdEventFile->isChecked()) {
            m_cChannel->delete_files();
            m_cChannel->set_piping(false);
            m_cChannel->set_file_alice(cEdEventFileAlice->text().toStdString());
            m_cChannel->set_file_bob(cEdEventFileBob->text().toStdString());
        }
        else {
            std::cerr << "Huh! Donnow where to push sync pulse events to!" << std::endl;
        }
    }
    else {
        
        // free running --> TTM
        if (cRdFreeUDP->isChecked()) {
            m_cChannel->ttm().set_output_mode(ttm::OUTPUT_MODE_UDP);
            m_cChannel->ttm().set_udp_address_alice(cEdFreeUDPAlice->text().toStdString());
            m_cChannel->ttm().set_udp_address_bob(cEdFreeUDPBob->text().toStdString());
        }
        else 
        if (cRdFreeFile->isChecked()) {
            m_cChannel->ttm().delete_files();
            m_cChannel->ttm().set_output_mode(ttm::OUTPUT_MODE_FILE);
            m_cChannel->ttm().set_filename_alice(cEdFreeFileAlice->text().toStdString());
            m_cChannel->ttm().set_filename_bob(cEdFreeFileBob->text().toStdString());
        }
        else {
            std::cerr << "Huh! Donnow where to push free runnings time tags to!" << std::endl;
        }
    }
    
    
    m_cChannel->launch_detector_thread();
}


/**
 * stop clicked
 */
void main_widget::clicked_stop() {
    m_cChannel->interrupt_thread();
}


/**
 * load alice settings from an XML Dom Element
 * 
 * @param   cElement    the DOM XML Element
 */
void main_widget::load_alice(QDomElement const & cElement) {

    // iterate over the child nodes
    for (QDomNode cNode = cElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        QDomElement cValue = cNode.toElement();
        if (cValue.isNull()) continue;
        
        // check for present "value" attribute
        if (!cValue.hasAttribute("value")) {
            std::cerr << "found alice key \"" << cValue.tagName().toStdString() << "\" but with no value attribute" << std::endl;
            continue;
        }
        
        // check the values we know
        if (cValue.tagName() == "detection_efficiency") {
            cEdDetectionEffAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "dark_count_rate") {
            cEdDetDarkCountRateAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "time_slot_width") {
            cEdTimeSlotWidthAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "time_slot_delay") {
            cEdTimeSlotDelayAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "distance_indep_loss") {
            cEdDistanceIndepLossAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "det_time_stnd_deviation") {
            cEdDetTimeStndDeviationAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "det_time_delay") {
            cEdDetTimeDelayAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "det_down_time") {
            cEdDetDownTimeAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "table_size") {
            cEdTableSizeAlice->setText(cValue.attribute("value"));
        }
        else {
            std::cerr << "unknown alice key \"" << cValue.tagName().toStdString() << "\"" << std::endl;
        }
    }    
}


/**
 * load bob settings from an XML Dom Element
 * 
 * @param   cElement    the DOM XML Element
 */
void main_widget::load_bob(QDomElement const & cElement) {
    
    // iterate over the child nodes
    for (QDomNode cNode = cElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        QDomElement cValue = cNode.toElement();
        if (cValue.isNull()) continue;

        // check for present "value" attribute
        if (!cValue.hasAttribute("value")) {
            std::cerr << "found bob key \"" << cValue.tagName().toStdString() << "\" but with no value attribute" << std::endl;
            continue;
        }
        
        // check the values we know
        if (cValue.tagName() == "detection_efficiency") {
            cEdDetectionEffBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "dark_count_rate") {
            cEdDetDarkCountRateBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "time_slot_width") {
            cEdTimeSlotWidthBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "time_slot_delay") {
            cEdTimeSlotDelayBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "distance_indep_loss") {
            cEdDistanceIndepLossBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "det_time_stnd_deviation") {
            cEdDetTimeStndDeviationBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "det_time_delay") {
            cEdDetTimeDelayBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "det_down_time") {
            cEdDetDownTimeBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "table_size") {
            cEdTableSizeBob->setText(cValue.attribute("value"));
        }
        else {
            std::cerr << "unknown bob key \"" << cValue.tagName().toStdString() << "\"" << std::endl;
        }
    }
}


/**
 * load general settings from an XML Dom Element
 * 
 * @param   cElement    the DOM XML Element
 */
void main_widget::load_general(QDomElement const & cElement) {
    
    // iterate over the child nodes
    for (QDomNode cNode = cElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        QDomElement cValue = cNode.toElement();
        if (cValue.isNull()) continue;
        
        // check for present "value" attribute
        if (!cValue.hasAttribute("value")) {
            std::cerr << "found general key \"" << cValue.tagName().toStdString() << "\" but with no value attribute" << std::endl;
            continue;
        }
        
        // check the values we know
        if (cValue.tagName() == "multi_photon_simulation") {
            cCkMultPhotSim->setChecked(cValue.attribute("value") == "true");
        }
        else
        if (cValue.tagName() == "sync_pulse_simulation") {
            cCkSyncPulse->setChecked(cValue.attribute("value") == "true");
        }
        else
        if (cValue.tagName() == "transmission_loss_simulation") {
            cCkTransmLoss->setChecked(cValue.attribute("value") == "true");
        }
        else
        if (cValue.tagName() == "inifinte_loop_simulation") {
            cCkLoopSimulation->setChecked(cValue.attribute("value") == "true");
        }
        else {
            std::cerr << "unknown general key \"" << cValue.tagName().toStdString() << "\"" << std::endl;
        }
    }
}


/**
 * load output settings from an XML Dom Element
 * 
 * @param   cElement    the DOM XML Element
 */
void main_widget::load_output(QDomElement const & cElement) {
    
    // iterate over the child nodes
    for (QDomNode cNode = cElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        QDomElement cValue = cNode.toElement();
        if (cValue.isNull()) continue;
        
        // check for present "value" attribute
        if (!cValue.hasAttribute("value")) {
            std::cerr << "found output key \"" << cValue.tagName().toStdString() << "\" but with no value attribute" << std::endl;
            continue;
        }
        
        // check the values we know
        if (cValue.tagName() == "free") {
            QString sFreeValue = cValue.attribute("value");
            if (sFreeValue == "udp") {
                cRdFreeUDP->setChecked(true);
            }
            else
            if (sFreeValue == "file") {
                cRdFreeFile->setChecked(true);
            }
            else {
                std::cerr << "unknown output key for free running: \"" << sFreeValue.toStdString() << "\"" << std::endl;
            }
        }
        else
        if (cValue.tagName() == "free_udp_alice") {
            cEdFreeUDPAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "free_udp_bob") {
            cEdFreeUDPBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "free_file_alice") {
            cEdFreeFileAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "free_file_bob") {
            cEdFreeFileBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "event") {
            QString sEventValue = cValue.attribute("value");
            if (sEventValue == "pipe") {
                cRdEventPipe->setChecked(true);
            }
            else
            if (sEventValue == "file") {
                cRdEventFile->setChecked(true);
            }
            else {
                std::cerr << "unknown output key for free running: \"" << sEventValue.toStdString() << "\"" << std::endl;
            }
        }
        else
        if (cValue.tagName() == "event_pipe_alice") {
            cEdEventPipeAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "event_pipe_bob") {
            cEdEventPipeBob->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "event_file_alice") {
            cEdEventFileAlice->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "event_file_bob") {
            cEdEventFileBob->setText(cValue.attribute("value"));
        }
        else {
            std::cerr << "unknown output key \"" << cValue.tagName().toStdString() << "\"" << std::endl;
        }
    }
}


/**
 * load source settings from an XML Dom Element
 * 
 * @param   cElement    the DOM XML Element
 */
void main_widget::load_source(QDomElement const & cElement) {
    
    // iterate over the child nodes
    for (QDomNode cNode = cElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        QDomElement cValue = cNode.toElement();
        if (cValue.isNull()) continue;
        
        // check for present "value" attribute
        if (!cValue.hasAttribute("value")) {
            std::cerr << "found source key \"" << cValue.tagName().toStdString() << "\" but with no value attribute" << std::endl;
            continue;
        }
        
        // check the values we know
        if (cValue.tagName() == "source_photon_rate") {
            cEdSourcePhotonRate->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "fiber_length") {
            cEdFiberLength->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "fiber_absorption_coeff") {
            cEdFiberAbsorptionCoeff->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "source_signal_error_probability") {
            cEdSourceSigErrProb->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "sync_det_time_stnd_deviation") {
            cEdSyncStandDeviation->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "multi_photon_rate") {
            cEdMultiPhotonRate->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "noise_photon_rate") {
            cEdNoisePhotonRate->setText(cValue.attribute("value"));
        }
        else
        if (cValue.tagName() == "simulation_end_time") {
            cEdSimEndTime->setText(cValue.attribute("value"));
        }
        else {
            std::cerr << "unknown source key \"" << cValue.tagName().toStdString() << "\"" << std::endl;
        }
    }
}


/**
 * load from an XML string
 * 
 * @param   cDomDoc     the XML DOM Doc string containing the values
 */
void main_widget::load_xml(QDomDocument const & cDomDoc) {
    
    // check the root element
    QDomElement cRootElement = cDomDoc.documentElement();
    if (cRootElement.tagName() != "qkd-simulate") {
        QMessageBox::critical(this, tr("Failed to apply values"), tr("Unknown format."), QMessageBox::Ok);
        return;
    }
    
    // iterate over the child nodes
    for (QDomNode cNode = cRootElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        QDomElement cElement = cNode.toElement();
        if (!cElement.isNull()) {
            
            if (cElement.tagName() == "source") load_source(cElement);
            if (cElement.tagName() == "alice") load_alice(cElement);
            if (cElement.tagName() == "bob") load_bob(cElement);
            if (cElement.tagName() == "output") load_output(cElement);
            if (cElement.tagName() == "general") load_general(cElement);
        }
    }    
}


/**
 * update alice detector
 */
void main_widget::update_detector_alice() {
    
    bool ok;
    
    // dark count rate
    if (cEdDetectionEffAlice->text().toDouble(&ok) != m_cChannel->alice()->efficiency() && ok) {
        try { 
            m_cChannel->alice()->set_efficiency(cEdDetectionEffAlice->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice efficiency: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice efficiency");
        throw std::invalid_argument("alice efficiency");
    }
    
    // down time
    if (cEdDetDownTimeAlice->text().toDouble(&ok) != m_cChannel->alice()->down_time() && ok) {
        try { 
            m_cChannel->alice()->set_down_time(cEdDetDownTimeAlice->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice down_time: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice down_time");
        throw std::invalid_argument("alice down_time");
    }
    
    // dark counts
    m_cChannel->alice()->set_dark_counts(cCkDarkCountsAlice->isChecked());
    
    // dark count rate
    if (cEdDetDarkCountRateAlice->text().toDouble(&ok) != m_cChannel->alice()->dark_count_rate() && ok) {
        try { 
            m_cChannel->alice()->set_dark_count_rate(cEdDetDarkCountRateAlice->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice dark_count_rate: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice dark_count_rate");
        throw std::invalid_argument("alice dark_count_rate");
    }
    
    // jitter
    m_cChannel->alice()->set_jitter(cCkJitterSimAlice->isChecked());
    
    // loss
    m_cChannel->alice()->set_loss(cCkDetLossAlice->isChecked());
    
    // photon time standard deviation
    if (cEdDetTimeStndDeviationAlice->text().toDouble(&ok) != m_cChannel->alice()->photon_time_stnd_deviation() && ok) {
        try { 
            m_cChannel->alice()->set_photon_time_stnd_deviation(cEdDetTimeStndDeviationAlice->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice photon_time_stnd_deviation: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice photon_time_stnd_deviation");
        throw std::invalid_argument("alice photon_time_stnd_deviation");
    }
    
    // photon detection time delay
    if (cEdDetTimeDelayAlice->text().toDouble(&ok) != m_cChannel->alice()->photon_time_delay() && ok) {
        try { 
            m_cChannel->alice()->set_photon_time_delay(cEdDetTimeDelayAlice->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice photon_time_delay: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice photon_time_delay");
        throw std::invalid_argument("alice photon_time_delay");
    }

    // distance independent loss
    if (cEdDistanceIndepLossAlice->text().toDouble(&ok) != m_cChannel->alice()->loss_rate() && ok) {
        try { 
            m_cChannel->alice()->set_loss_rate(cEdDistanceIndepLossAlice->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice loss_rate: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice loss_rate");
        throw std::invalid_argument("alice loss_rate");
    }

    // time slot width
    if (cEdTimeSlotWidthAlice->text().toDouble(&ok) != m_cChannel->alice()->time_slot_width() && ok) {
        try { 
            m_cChannel->alice()->set_time_slot_width(cEdTimeSlotWidthAlice->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice time_slot_width: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice time_slot_width");
        throw std::invalid_argument("alice time_slot_width");
    }

    // table size
    if (cEdTableSizeAlice->text().toULong(&ok) != m_cChannel->alice()->event_table_size() && ok) {
        try { 
            m_cChannel->alice()->set_event_table_size(cEdTableSizeAlice->text().toULong());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set alice table size: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: alice table size");
        throw std::invalid_argument("alice table size");
    }
}


/**
 * update bob detector
 */
void main_widget::update_detector_bob() {
    
    bool ok;
    
    // dark count rate
    if (cEdDetectionEffBob->text().toDouble(&ok) != m_cChannel->bob()->efficiency() && ok) {
        try { 
            m_cChannel->bob()->set_efficiency(cEdDetectionEffBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob efficiency: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob efficiency");
        throw std::invalid_argument("bob efficiency");
    }
    
    // down time
    if (cEdDetDownTimeBob->text().toDouble(&ok) != m_cChannel->bob()->down_time() && ok) {
        try { 
            m_cChannel->bob()->set_down_time(cEdDetDownTimeBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob down_time: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob down_time");
        throw std::invalid_argument("bob down_time");
    }
    
    // dark counts
    m_cChannel->bob()->set_dark_counts(cCkDarkCountsBob->isChecked());
    
    // dark count rate
    if (cEdDetDarkCountRateBob->text().toDouble(&ok) != m_cChannel->bob()->dark_count_rate() && ok) {
        try { 
            m_cChannel->bob()->set_dark_count_rate(cEdDetDarkCountRateBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob dark_count_rate: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob dark_count_rate");
        throw std::invalid_argument("bob dark_count_rate");
    }
    
    // jitter
    m_cChannel->bob()->set_jitter(cCkJitterSimBob->isChecked());
    
    // loss
    m_cChannel->bob()->set_loss(cCkDetLossBob->isChecked());
    
    // photon time standard deviation
    if (cEdDetTimeStndDeviationBob->text().toDouble(&ok) != m_cChannel->bob()->photon_time_stnd_deviation() && ok) {
        try { 
            m_cChannel->bob()->set_photon_time_stnd_deviation(cEdDetTimeStndDeviationBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob photon_time_stnd_deviation: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob photon_time_stnd_deviation");
        throw std::invalid_argument("bob photon_time_stnd_deviation");
    }
    
    // photon detection time delay
    if (cEdDetTimeDelayBob->text().toDouble(&ok) != m_cChannel->bob()->photon_time_delay() && ok) {
        try { 
            m_cChannel->bob()->set_photon_time_delay(cEdDetTimeDelayBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob photon_time_delay: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob photon_time_delay");
        throw std::invalid_argument("bob photon_time_delay");
    }

    // distance independent loss
    if (cEdDistanceIndepLossBob->text().toDouble(&ok) != m_cChannel->bob()->loss_rate() && ok) {
        try { 
            m_cChannel->bob()->set_loss_rate(cEdDistanceIndepLossBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob loss_rate: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob loss_rate");
        throw std::invalid_argument("bob loss_rate");
    }

    // time slot width
    if (cEdTimeSlotWidthBob->text().toDouble(&ok) != m_cChannel->bob()->time_slot_width() && ok) {
        try { 
            m_cChannel->bob()->set_time_slot_width(cEdTimeSlotWidthBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob time_slot_width: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob time_slot_width");
        throw std::invalid_argument("bob time_slot_width");
    }

    // timeslot center shift
    if (cEdTimeSlotDelayBob->text().toDouble(&ok) != m_cChannel->timeslot_center_shift() && ok) {
        try { 
            m_cChannel->set_timeslot_center_shift(cEdTimeSlotDelayBob->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set timeslot_center_shift: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: timeslot_center_shift");
        throw std::invalid_argument("timeslot_center_shift");
    }
    
    // table size
    if (cEdTableSizeBob->text().toULong(&ok) != m_cChannel->bob()->event_table_size() && ok) {
        try { 
            m_cChannel->bob()->set_event_table_size(cEdTableSizeBob->text().toULong());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set bob table size: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: bob table size");
        throw std::invalid_argument("bob table size");
    }
}


/**
 * update values for the fiber
 */
void main_widget::update_fiber() {
    
    bool ok;
    
    // fiber absorption_coefficient
    if (cEdFiberAbsorptionCoeff->text().toDouble(&ok) != m_cChannel->fiber().absorption_coefficient() && ok) {
        try { 
            m_cChannel->fiber().set_absorption_coefficient(cEdFiberAbsorptionCoeff->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set fiber absorption_coefficient: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: fiber absorption_coefficient");
        throw std::invalid_argument("fiber absorption_coefficient");
    }
    
    // fiber length
    if (cEdFiberLength->text().toDouble(&ok) != m_cChannel->fiber().length() && ok) {
        try { 
            m_cChannel->fiber().set_length(cEdFiberLength->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set fiber length: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: fiber length");
        throw std::invalid_argument("fiber length");
    }
    
    // transmission loss
    m_cChannel->fiber().set_loss(cCkTransmLoss->isChecked());
}


/**
 * update source
 */
void main_widget::update_source() {
    
    bool ok;
    
    // photon rate
    if (cEdSourcePhotonRate->text().toDouble(&ok) != m_cChannel->source().photon_rate() && ok) {
        try { 
            m_cChannel->source().set_photon_rate(cEdSourcePhotonRate->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set source photon_rate: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: source photon_rate");
        throw std::invalid_argument("source photon_rate");
    }
    
    // signal error rate
    if (cEdSourceSigErrProb->text().toDouble(&ok) != m_cChannel->source().signal_error_probability() && ok) {
        try { 
            m_cChannel->source().set_signal_error_probability(cEdSourceSigErrProb->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set source signal_error_probability: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: source signal_error_probability");
        throw std::invalid_argument("source signal_error_probability");
    }
    
    // multi photon simulation
    m_cChannel->source().set_multi_photons(cCkMultPhotSim->isChecked());
    
    // multi photon rate
    if (cEdMultiPhotonRate->text().toDouble(&ok) != m_cChannel->source().multi_photon_rate() && ok) {
        try { 
            m_cChannel->source().set_multi_photon_rate(cEdMultiPhotonRate->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set source multi_photon_rate: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: source multi_photon_rate");
        throw std::invalid_argument("source multi_photon_rate");
    }
    
    // noise photon rate
    if (cEdNoisePhotonRate->text().toDouble(&ok) != m_cChannel->fiber().noise_photon_rate() && ok) {
        try { 
            m_cChannel->fiber().set_noise_photon_rate(cEdNoisePhotonRate->text().toDouble());
        }
        catch (std::exception & cException) {
            emit update_message("failed to set fiber noise_photon_rate: " + QString::fromStdString(cException.what()));
            throw;
        }
    }
    else if (!ok) {
        emit update_message("value conversion error: fiber noise_photon_rate");
        throw std::invalid_argument("fiber noise_photon_rate");
    }
}


/**
 * update values for the output
 */
void main_widget::update_output() {
    
}



/**
 * update simulation view: simulation progress ...
 */
void main_widget::update_simulation_view() {
    
    bool bRunningSimulation = m_cChannel->is_simulation_running();
    
    cBtnStart->setEnabled(!bRunningSimulation);
    cBtnStop->setEnabled(bRunningSimulation);
    cLbProgress->setEnabled(bRunningSimulation);
    cPbProgress->setEnabled(bRunningSimulation);
    
    int64_t nValue = 0;
    if (bRunningSimulation) {
        if (m_cChannel->manager()->get_sim_end_time() != 0) {
            nValue = (m_cChannel->manager()->get_time() * 100) / m_cChannel->manager()->get_sim_end_time();
        }
    }
    cPbProgress->setValue(nValue);
    cLcdRound->display(static_cast<int>(m_cChannel->round()));
}


/**
 * update values from the input
 */
void main_widget::update_values() {
    
    bool ok;
    
    // clear any error value
    emit update_message("");
    
    try {
    
        // sync stnd deviation
        if (cEdSyncStandDeviation->text().toDouble(&ok) != m_cChannel->sync_stnd_deviation() && ok) {
            try { 
                m_cChannel->set_sync_stnd_deviation(cEdSyncStandDeviation->text().toDouble());
            }
            catch (std::exception & cException) {
                emit update_message("failed to set source sync_stnd_deviation: " + QString::fromStdString(cException.what()));
                throw;
            }
        }
        else if (!ok) {
            emit update_message("value conversion error: sync_stnd_deviation");
            throw std::invalid_argument("sync_stnd_deviation");
        }
        
        // simulation end time
        if (cEdSimEndTime->text().toDouble(&ok) != m_cChannel->sim_end_time() && ok) {
            try { 
                m_cChannel->set_sim_end_time(cEdSimEndTime->text().toDouble());
            }
            catch (std::exception & cException) {
                emit update_message("failed to set simulation end time: " + QString::fromStdString(cException.what()));
                throw;
            }
        }
        else if (!ok) {
            emit update_message("value conversion error: simulation end time");
            throw std::invalid_argument("simulation end time");
        }
        
        // update the parts
        update_source();
        update_fiber();
        update_detector_alice();
        update_detector_bob();
        update_output();
        
    }
    catch (UNUSED std::exception & cException) { }
    
    
    m_cChannel->update_delay_times();
    
    // ensure the widget states are ok
    check_ui();
}
