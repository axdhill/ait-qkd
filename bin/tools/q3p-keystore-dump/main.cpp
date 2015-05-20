/*
 * main.cpp
 * 
 * This is the Q3P KeyStore Dump main startup file.
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

#include <chrono>

#include <boost/program_options.hpp>

// Qt
#include <QtCore/QString>

// ait
#include <qkd/key/key.h>
#include <qkd/q3p/db.h>
#include <qkd/utility/environment.h>


// ------------------------------------------------------------
// fwd


void dump(QString sURL);


// ------------------------------------------------------------
// code


/**
 * dump the content of a database of the given URL
 * 
 * @param   sURL        url defining the key-DB
 */
void dump(QString sURL) {
    
    // measure time
    auto nStart = std::chrono::high_resolution_clock::now();
    qkd::q3p::key_db cDB;
    
    try {
        
        // if we don't have a ':' in the url then we might lack a scheme
        // so we go for file:// in the current folder
        if (sURL.indexOf(':') == -1) {
            std::stringstream sNewURL;
            sNewURL << "file://";
            sNewURL << qkd::utility::environment::current_path().string();
            sNewURL << "/";
            sNewURL << sURL.toStdString();
            sURL = QString::fromStdString(sNewURL.str());
        }
        
        // open the DB
        cDB = qkd::q3p::db::open(sURL);
    }
    catch (qkd::q3p::db::db_url_scheme_unknown & cDBUrlSchemeUnknown) {
        std::cerr << "failed to open key DB - unknown URL scheme: \"" << sURL.toStdString() << "\"" << std::endl;
        return;
    }
    catch (qkd::q3p::db::db_init_error & cDBInitError) {
        std::cerr << "failed to open key DB - init error: \"" << sURL.toStdString() << "\"" << std::endl;
        return;
    }
    
    // header
    std::cout << "dumping Q3P keystore DB" << std::endl;
    std::cout << "url: " << sURL.toStdString() << std::endl;
    std::cout << "description: " << cDB->describe().toStdString() << std::endl;
    std::cout << "keys in db: " << cDB->count() << std::endl;
    
    // space needed for the key-bits:
    // we have quantum() key bytes: each byte neads 2 space
    // plus the space in between for every 8th byte
    unsigned int nFill = cDB->quantum() * 2 + cDB->quantum() / 8;
    
    // construct a proper header format strings
    std::stringstream sFormat;
    sFormat << "%-10s ";                    // key-id
    sFormat << "%-5s ";                     // flags
    sFormat << "%-" << nFill << "s ";       // key-data
    sFormat << "%s\n";                      // ascii

    // print the header
    fprintf(stdout, sFormat.str().c_str(), "key-id", "flags", "key-data", "ascii");

    // walk over all keys
    for (qkd::key::key_id nID = cDB->min_id(); nID < cDB->max_id(); nID++) {
        
        // don't dump invalid keys
        if (!cDB->valid(nID)) continue;
        
        // get the key
        qkd::key::key cKey = cDB->get(nID);
        fprintf(stdout, "%010u ", cKey.id());
        
        // key flags
        if (cDB->injected(nID)) fprintf(stdout, "I");
        else fprintf(stdout, " ");
        if (cDB->eventual_sync(nID)) fprintf(stdout, "E");
        else fprintf(stdout, " ");
        if (cDB->real_sync(nID)) fprintf(stdout, "R");
        else fprintf(stdout, " ");

        fprintf(stdout, "   ");
        
        // key bits
        for (uint64_t i = 0; i < cKey.size(); i++) {
            fprintf(stdout, "%02x", cKey.data()[i]);
            if ((i % 8) == 7) fprintf(stdout, " ");
        }
            
        fprintf(stdout, " |");
        
        // key chars
        for (uint64_t i = 0; i < cKey.size(); i++) {
            char c = (char)cKey.data()[i];
            if ((c < ' ') || (c > 'z')) c = '.';
            fprintf(stdout, "%c", c);
        }

        fprintf(stdout, "|\n");
    }

    // measure time
    auto nStop = std::chrono::high_resolution_clock::now();
    auto nTimeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(nStop - nStart);

    std::cout << "dumping took " << nTimeDiff.count() << " millisec" << std::endl;
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
    std::string sApplication = std::string("q3p-keystore-dump - AIT Q3P KeyStore Dump Tool V") + VERSION;
    std::string sDescription = std::string("\nThis prints the content of an AIT Q3P KeyStore.\n\nCopyright 2012-2015 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] URL";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("version,v", "print version string");
    
    // final arguments
    boost::program_options::options_description cArgs("Arguments");
    cArgs.add_options()("URL", "URL is the url of database to access.");
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("URL", 1);
    
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
        std::cout << cArgs.find("URL", false).description() << "\n" << std::endl;      
        std::cout << "The colums are this:\n\n";
        std::cout << "\tkey-id:     id of the key in the DB\n";
        std::cout << "\tflags:      flags of a key:\n";
        std::cout << "\t                I = injected\n";
        std::cout << "\t                E = eventual sync\n";
        std::cout << "\t                R = real sync\n";
        std::cout << "\tkey-data:   Key bits\n";
        std::cout << "\tascii:      Ascii of the key value\n";
        std::cout << std::endl;
        return 0;
    }
    
    // check for "version" set
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }
    
    // we need a url
    if (cVariableMap.count("URL") != 1) {
        std::cerr << "need excactly one URL argument" << "\ntype '--help' for help" << std::endl;
        return 1;
    }
    
    // extract the URL
    QString sURL = QString::fromStdString(cVariableMap["URL"].as<std::string>());

    // dump the url
    dump(sURL);
    
    return 0;
}
