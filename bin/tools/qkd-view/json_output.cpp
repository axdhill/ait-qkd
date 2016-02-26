/*
 * json_output.cpp
 *
 * This is shows the current QKD system snapshot
 *
 * Author: Manuel Warum, <manuel.warum@ait.ac.at>
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

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "json_output.h"

/**
 * Initializes the output format according to the provided runtime options.
 * @param  cProgramOptions  formatting-related runtime options
 */
void json_output::initialize(configuration_options const &cProgramOptions) {
    bPrintModuleIO = cProgramOptions.bOnlyModuleIO;
}

/**
 * Writes investigation results to the specified output stream.
 * @param  cOut            the stream to write to.
 * @param  cInvestigation  the particular investigation instance to format.
 */
void json_output::write(std::ostream &cOut, qkd::utility::investigation &cInvestigation) {
    cOut << "{ \"details\": ";
    dump_investigation_details(cOut, cInvestigation);
    if (bPrintModuleIO) {
        cOut << ", ";
        dump_nodes(cOut, cInvestigation.nodes());
        cOut << ", ";
        dump_links(cOut, cInvestigation.links());
    }
    cOut << ", ";
    dump_modules(cOut, cInvestigation.modules());
    cOut << "}" << std::endl;
}

/**
 * Utility method that provides a format instance for the specified fields.
 *
 * @param  cFields         a vector of all the fields we are interested in.
 * @return a format instance that concatenates all fields into a JSON object.
 */
inline boost::format json_output::simple_format(std::vector<std::string> const &cFields) const {
    std::stringstream ss;
    ss << "{";
    for (uint32_t i = 0; i < cFields.size(); i++) {
        if (i > 0) ss << ",";
        ss << "\"" << cFields[i] << "\":\"%" << std::to_string(i + 1) << "%\"";
    }
    ss << "}";
    try {
        return boost::format(ss.str());
    } catch (const std::exception &ex) { // TODO: Remove exception handling when no more issues are found
        std::cerr << ss.str() << std::endl;
        throw ex;
    }
}

/**
 * Writes the basic investigation details to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cInvestigation  the particular investigation instance to format.
 */
void json_output::dump_investigation_details(std::ostream &cOut, qkd::utility::investigation &cInvestigation) const {
    std::time_t cTimestamp = std::chrono::system_clock::to_time_t(cInvestigation.timestamp());
    cOut << "{ \"time\":\"" << std::ctime(&cTimestamp) << "\", \"investigation_time\":"
    << std::chrono::duration_cast<std::chrono::milliseconds>(cInvestigation.duration()).count() << " }";
}

/**
 * Writes node information to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cNodeMap        the particular node data to format.
 */
void json_output::dump_nodes(std::ostream &cOut,
                             const std::map<std::string, qkd::utility::properties> &cNodeMap) const {
    const std::vector<std::string> fields = {"id", "dbus", "start_time", "process_id", "process_image", "config_file",
                                             "random_url", "debug"};
    cOut << "\"nodes\": [";
    dump_json_array(cOut, fields, cNodeMap);
    cOut << "]";
}

/**
 * Writes link information to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cLinkMap        the particular link data to format.
 */
void json_output::dump_links(std::ostream &cOut,
                             const std::map<std::string, qkd::utility::properties> &cNodeMap) const {
    const std::vector<std::string> fields = {"id", "node", "dbus", "state", "connected", "db_opened", "uri_local",
                                             "uri_peer", "master", "slave", "mq", "nic"};
    cOut << "\"links\": [";
    dump_json_array(cOut, fields, cNodeMap);
    cOut << "]";
}

/**
 * Writes module information to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cModuleMap      the particular module data to format.
 */
void json_output::dump_modules(std::ostream &cOut,
                               const std::map<std::string, qkd::utility::properties> &cModuleMap) const {
    std::vector<std::string> fields;
    if (bPrintModuleIO)
        fields = {"id", "url_pipe_in", "url_pipe_out", "url_listen", "url_peer"};
    else
        fields = {
                "id", "dbus", "pipeline", "process_id", "type", "type_name", "start_time", "state", "state_name",
                "role", "role_name", "url_pipe_in", "url_pipe_out", "url_listen", "url_peer", "idle", "random_url",
                "keys_incoming", "keys_outgoing", "key_bits_incoming", "key_bits_outgoing", "disclosed_bits_incoming",
                "disclosed_bits_outgoing", "debug", "description", "organisation", "process_image"
        };

    cOut << "\"modules\": [";
    dump_json_array(cOut, fields, cModuleMap);
    cOut << "]";
}

/**
 * Utility method that dumps a specified set of properties to the specified
 * output stream.
 *
 * @param  cOut            the stream to write to.
 * @param  cFields         a vector of all the fields we are interested in.
 * @param  source          the investigation's data container.
 */
inline void json_output::dump_json_array(std::ostream &cOut,
                                         const std::vector<std::string> &cFields,
                                         const std::map<std::string, qkd::utility::properties> &source) const {
    const boost::format formatPrototype = simple_format(cFields);
    bool pastFirst = false;

    for (auto const &cNode : source) {
        boost::format nodeFormat(formatPrototype);
        for (auto const &field : cFields)
            nodeFormat % cNode.second.at(field);

        if (pastFirst)
            cOut << ",";
        else
            pastFirst = true;
        cOut << nodeFormat.str();
    }
}