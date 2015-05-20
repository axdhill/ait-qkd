/*
 * main.cpp
 * 
 * This is the Q3P MQ helper main startup file.
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <mqueue.h>

// Qt
#include <QtDBus>

// ait
#include <qkd/utility/dbus.h>
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decl


/**
 * a single message queue serviced by a q3p link
 */
class mq {
    
public:    
    
    QString sNode;          /**< the node name as it appears on the DBus */
    QString sLink;          /**< the link serviced by the node */
    QString sMQ;            /**< the message queue serviced by the q3p engine (aka Link) */
};


/**
 * a list of MQs
 */
typedef std::list<mq> mq_list;


// ------------------------------------------------------------
// fwd


void dump(QString sMQ, bool bHexOutput, uint64_t nMessages);
mq_list scan_dbus();
mq_list scan_node(QString const & sNode);
void show_list(mq_list const & cList);


// ------------------------------------------------------------
// code


/**
 * dump the content of a message queue to stdout
 * 
 * @param   sMQ             name of the message queue
 * @param   bHexOutput      if true, the output is hex
 * @param   nMessages       number of messages to read
 */
void dump(QString sMQ, bool bHexOutput, uint64_t nMessages) {
    
    // check for infinite loop
    bool bLoopInfinite = (nMessages == 0);
    
    // open the MQ in read mode
    mqd_t nMQD = mq_open(sMQ.toAscii(), O_RDONLY);
    if (nMQD == -1) {
        std::cerr << "failed to open message queue: " << strerror(errno) << std::endl;
        std::cerr << "please check if '" << sMQ.toStdString() << "' really names a valid message queue." << std::endl;
        std::cerr << "(hint mount the 'mqueue' filesystem under /dev/mqueue as specified by 'man mq_overview')" << std::endl;
        return;
    }
    
    // as long as there is a message ...
    struct mq_attr cAttr;
    
    // read out from the MQ attributes
    memset(&cAttr, 0, sizeof(cAttr));
    mq_getattr(nMQD, &cAttr);
    
    // prepare space for the next message
    qkd::utility::memory cMessage(cAttr.mq_msgsize);
        
    // forever ... or we have enough
    while (bLoopInfinite || (nMessages > 0)) {
        
        // big
        cMessage.resize(cAttr.mq_msgsize);
        
        // get message
        unsigned nMsgPriority = 0;
        int64_t nRead = mq_receive(nMQD, (char *)cMessage.get(), cAttr.mq_msgsize, &nMsgPriority);
        if (nRead == -1) {
            std::cerr << "failed to get message from MQ: " << strerror(errno) << std::endl;
            break;
        }
        
        // fix size (if necessary);
        cMessage.resize(nRead);
        
        // output
        if (bHexOutput) std::cout << cMessage.as_hex() << std::endl;
        else fwrite(cMessage.get(), nRead, 1, stdout);
        
        // check if we have enough
        if (nMessages) nMessages--;
    }
    
    // close the reading mq again
    mq_close(nMQD);
}


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    // create the command line header
    std::string sApplication = std::string("q3p-mq-reader - AIT Q3P Message Queue Reader Tool V") + VERSION;
    std::string sDescription = std::string("\nThis extracts keys from a Q3P message queue.\n\nCopyright 2012-2015 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] [MQ]";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("number,n", boost::program_options::value<uint64_t>(), "number of keys to withdraw from queue");
    cOptions.add_options()("scan,s", "scan system for available message queues");
    cOptions.add_options()("version,v", "print version string");
    cOptions.add_options()("hex,x", "convert keys data to ascii hex strings");
    
    // final arguments
    boost::program_options::options_description cArgs("Arguments");
    cArgs.add_options()("MQ", "MQ is the operating system's name of the message queue to open.\nMandatory if not started in scanning mode.");
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("MQ", 1);
    
    // construct overall options
    boost::program_options::options_description cCmdLineOptions("Command Line");
    cCmdLineOptions.add(cOptions);
    cCmdLineOptions.add(cArgs);

    // option variable map
    boost::program_options::variables_map cVariableMap;
    
    try {
        // parse action
        boost::program_options::command_line_parser cParser(argc, argv);
        boost::program_options::store(cParser.options(cCmdLineOptions).positional(cPositionalDescription).run(), cVariableMap);
        boost::program_options::notify(cVariableMap);        
    }
    catch (std::exception & cException) {
        std::cerr << "error parsing command line: " << cException.what() << "\ntype '--help' for help" << std::endl;        
        return 1;
    }
    
    // check for "help" set
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        std::cout << cArgs.find("MQ", false).description() << "\n" << std::endl;      
        return 0;
    }
    
    // check for "version" set
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }
    
    QString sMQ;
    bool bHexOutput = false;
    bool bScanMQs = false;
    uint64_t nMessages = 0;
    
    // check for "scan" set
    if (cVariableMap.count("scan")) bScanMQs = true;
    
    // check for "hex" set
    if (cVariableMap.count("hex")) bHexOutput = true;
    
    // check for "messages" count
    if (cVariableMap.count("number")) nMessages = cVariableMap["number"].as<uint64_t>();

    // we need a MQ, if not scan
    if ((bScanMQs) && cVariableMap.count("MQ")) {
        std::cerr << "going to scan system, but message queue is given." << "\ntype '--help' for help"<< std::endl;
        return 1;
    }
    if ((!bScanMQs) && (cVariableMap.count("MQ") != 1)) {
        std::cerr << "need excactly one MQ argument, if not scanning" << "\ntype '--help' for help"<< std::endl;
        return 1;
    }
    
    // extract the MQ
    if (cVariableMap.count("MQ") == 1) sMQ = QString::fromStdString(cVariableMap["MQ"].as<std::string>());
    
    // start up Qt subsystem
    QCoreApplication cApp(argc, argv);
    cApp.processEvents();

    if (bScanMQs) {
        // scan for MQs and show them
        mq_list cList = scan_dbus();
        show_list(cList);
    }
    else {
        // dump the mq output to stdout
        dump(sMQ, bHexOutput, nMessages);
    }
    
    return 0;
}


/**
 * scan the DBus for any serviced message queues
 * 
 * @return  a list of found message queues
 */
mq_list scan_dbus() {
    
    mq_list cList;
    
    // get all registered service names
    const QStringList sServiceNames = qkd::utility::dbus::qkd_dbus().interface()->registeredServiceNames();
    for (int i = 0; i < sServiceNames.size(); i++) {
        
        // got a Q3P node?
        if (sServiceNames[i].startsWith("at.ac.ait.q3p.node-")) {
            
            // collect message queue names
            mq_list cMQNodes = scan_node(sServiceNames[i]);
            cList.insert(cList.end(), cMQNodes.begin(), cMQNodes.end());
        }
    }
    
    return cList;
}


/**
 * scan a single node
 * 
 * @param   sNode       node service name
 * @return  a list of found message queues
 */
mq_list scan_node(QString const & sNode) {
    
    mq_list cList;
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    
    // iterate trough the links of the given node
    QDBusMessage cMessage = QDBusMessage::createMethodCall(sNode, "/Node", "at.ac.ait.q3p.node", "links");
    QDBusMessage cReply = cDBus.call(cMessage);
    if (cReply.type() == QDBusMessage::ReplyMessage) {
        
        // iterate over the links of the node
        QStringList cLinkList = cReply.arguments()[0].toStringList();
        for (int i = 0; i < cLinkList.size(); i++) {
            
            QString sLinkService = "/Link/" + cLinkList[i] + "/MQ";
            QDBusMessage cMessage = QDBusMessage::createMethodCall(sNode, sLinkService, "org.freedesktop.DBus.Properties", "Get");
            cMessage << "at.ac.ait.q3p.messagequeue";
            cMessage << "name";
            QDBusMessage cReply = cDBus.call(cMessage);
            if (cReply.type() == QDBusMessage::ReplyMessage) {
                
                mq cItem;
                cItem.sNode = sNode;
                cItem.sLink = cLinkList[i];
                cItem.sMQ = cReply.arguments()[0].value<QDBusVariant>().variant().value<QString>();
                
                cList.push_back(cItem);
            }
        }
    }
    
    return cList;        
}


/**
 * show the list to the user
 * 
 * @param   cList   list of known message queues
 */
void show_list(mq_list const & cList) {
    
    // something to show at all?
    if (cList.empty()) {
        
        std::cout << "No message queues serviced by Q3P links detected on the system." << std::endl;
        return;
    }
    
    // show them
    std::cout << "found nodes, links and message queues:" << std::endl;
    for (auto & iter : cList) {
        
        // formater
        boost::format cFormater = boost::format("node: %|-30| link: %|-30| mq: %|-30|");
        std::cout << cFormater % iter.sNode.toStdString() % iter.sLink.toStdString() % iter.sMQ.toStdString() << std::endl;
    }
}
