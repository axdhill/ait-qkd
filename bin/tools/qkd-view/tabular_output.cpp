/*
 * tabular_output.cpp
 *
 * This is shows the current QKD system snapshot
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

#include <ostream>
#include <sstream>

#include <boost/format.hpp>

#include "tabular_output.h"

/**
 * Initializes the output format according to the provided runtime options.
 * @param  cProgramOptions  formatting-related runtime options
 */
void tabular_output::initialize(configuration_options const &cProgramOptions) {
    bPrintHeader = ! cProgramOptions.bOmitHeader;
    bPrintModuleIO = cProgramOptions.bOnlyModuleIO;
    bPrintShort = cProgramOptions.bOutputShort;
}

/**
 * Writes investigation results to the specified output stream.
 * @param  cOut            the stream to write to.
 * @param  cInvestigation  the particular investigation instance to format.
 */
void tabular_output::write(std::ostream &cOut, qkd::utility::investigation &cInvestigation) {
    if (!bPrintHeader)
        dump_investigation_details(cOut, cInvestigation);
    if (!bPrintModuleIO) {
        dump_nodes(cOut, cInvestigation.nodes());
        dump_links(cOut, cInvestigation.links());
    }
    dump_modules(cOut, cInvestigation.modules());
}

/**
 * Set the maximum column width for a given set of properties.
 *
 * @param   cColumnWidth   the column width map.
 * @param   cProperties    the properties to inspect.
 */
void tabular_output::set_column_width(column_width &cColumnWidth, qkd::utility::properties const &cProperties) {
    // set the column names as initial width
    for (auto const &cColumn : cProperties) {
        // if this is the first entry with this id, take the length of the name as initial width
        if (cColumnWidth.find(cColumn.first) == cColumnWidth.end())
            cColumnWidth[cColumn.first] = cColumn.first.length();

        // set the new column to the maximum
        cColumnWidth[cColumn.first] = std::max(cColumnWidth[cColumn.first], cColumn.second.length());
    }
}

/**
 * Writes the basic investigation details to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cInvestigation  the particular investigation instance to format.
 */
void tabular_output::dump_investigation_details(std::ostream &cOut, qkd::utility::investigation &cInvestigation) const {
    std::time_t cTimestamp = std::chrono::system_clock::to_time_t(cInvestigation.timestamp());
    cOut << "QKD system investigation results from " << std::ctime(&cTimestamp);
    cOut << "QKD system investigation took " <<
    std::chrono::duration_cast<std::chrono::milliseconds>(cInvestigation.duration()).count() << "ms" << std::endl;
}

/**
 * Writes link information to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cLinkMap        the particular link data to format.
 */
void tabular_output::dump_links(std::ostream &cOut, const std::map<std::string, qkd::utility::properties> &cLinkMap) {
    // something to show at all?
    if (cLinkMap.empty()) return;

    // set the column width for links
    cColumnWidth.cLink.clear();
    for (auto const &cLink : cLinkMap) {
        set_column_width(cColumnWidth.cLink, cLink.second);
    }

    // construct the format string: "link:" - id - node - dbus - state - connected - db_opened - uri_local - uri_peer - master - slave - mq - nic
    std::stringstream ss;
    ss << "%-5s    "
    << "%-" << cColumnWidth.cLink["id"] << "s    "
    << "%-" << cColumnWidth.cLink["node"] << "s    "
    << "%-" << cColumnWidth.cLink["dbus"] << "s    "
    << "%-" << cColumnWidth.cLink["state"] << "s    "
    << "%-" << cColumnWidth.cLink["connected"] << "s    "
    << "%-" << cColumnWidth.cLink["db_opened"] << "s    "
    << "%-" << cColumnWidth.cLink["uri_local"] << "s    "
    << "%-" << cColumnWidth.cLink["uri_peer"] << "s    "
    << "%-" << cColumnWidth.cLink["master"] << "s    "
    << "%-" << cColumnWidth.cLink["slave"] << "s    "
    << "%-" << cColumnWidth.cLink["mq"] << "s    "
    << "%-" << cColumnWidth.cLink["nic"] << "s";

    // the column format
    boost::format cFormat(ss.str());

    // print the heading
    if (bPrintHeader) {
        boost::format cHeading(cFormat);
        cHeading
        % "type"
        % "id"
        % "node"
        % "dbus"
        % "state"
        % "connected"
        % "db_opened"
        % "uri_local"
        % "uri_peer"
        % "master"
        % "slave"
        % "mq"
        % "nic";

        cOut << cHeading.str() << std::endl;
    }

    // go over the found links
    for (auto const &cLink : cLinkMap) {
        boost::format cLinkFormat(cFormat);

        cLinkFormat % "link:";
        cLinkFormat % cLink.second.at("id");
        cLinkFormat % cLink.second.at("node");
        cLinkFormat % cLink.second.at("dbus");
        cLinkFormat % cLink.second.at("state");
        cLinkFormat % cLink.second.at("connected");
        cLinkFormat % cLink.second.at("db_opened");
        cLinkFormat % cLink.second.at("uri_local");
        cLinkFormat % cLink.second.at("uri_peer");
        cLinkFormat % cLink.second.at("master");
        cLinkFormat % cLink.second.at("slave");
        cLinkFormat % cLink.second.at("mq");
        cLinkFormat % cLink.second.at("nic");

        cOut << cLinkFormat.str() << std::endl;
    }

    // add an artificial empty line for nice viewing if headers are on
    if (bPrintHeader) cOut << std::endl;
}

/**
 * Writes module information to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cModuleMap      the particular module data to format.
 */
void tabular_output::dump_modules(std::ostream &cOut, const std::map<std::string, qkd::utility::properties> &cModuleMap) {
    // something to show at all?
    if (cModuleMap.empty()) return;

    // set the column width for modules
    cColumnWidth.cModule.clear();
    for (auto const &cModule : cModuleMap) {
        set_column_width(cColumnWidth.cModule, cModule.second);
    }

    // construct the format string: "module:" id - dbus - ...
    std::stringstream ss;
    if (bPrintShort) {
        // short version
        ss << "%-7s    "
        << "%-" << cColumnWidth.cModule["id"] << "s    "
        << "%-" << cColumnWidth.cModule["dbus"] << "s    "
        << "%-" << cColumnWidth.cModule["keys_incoming"] << "s    "
        << "%-" << cColumnWidth.cModule["keys_outgoing"] << "s    "
        << "%-" << cColumnWidth.cModule["key_bits_incoming"] << "s    "
        << "%-" << cColumnWidth.cModule["key_bits_outgoing"] << "s    "
        << "%-" << cColumnWidth.cModule["pipeline"] << "s    "
        << "%-" << cColumnWidth.cModule["process_id"] << "s    "
        << "%-" << cColumnWidth.cModule["state_name"] << "s    "
        << "%-" << cColumnWidth.cModule["role_name"] << "s    "
        << "%-" << cColumnWidth.cModule["url_pipe_in"] << "s    "
        << "%-" << cColumnWidth.cModule["url_pipe_out"] << "s    "
        << "%-" << cColumnWidth.cModule["url_listen"] << "s    "
        << "%-" << cColumnWidth.cModule["url_peer"] << "s    "
        << "%-" << cColumnWidth.cModule["idle"] << "s";
    } else if (bPrintModuleIO) {
        // short version
        ss << "%-" << cColumnWidth.cModule["id"] << "s    "
        << "%-" << cColumnWidth.cModule["url_pipe_in"] << "s    "
        << "%-" << cColumnWidth.cModule["url_pipe_out"] << "s    "
        << "%-" << cColumnWidth.cModule["url_listen"] << "s    "
        << "%-" << cColumnWidth.cModule["url_peer"] << "s";
    } else {
        // full version
        ss << "%-7s    "
        << "%-" << cColumnWidth.cModule["id"] << "s    "
        << "%-" << cColumnWidth.cModule["dbus"] << "s    "
        << "%-" << cColumnWidth.cModule["pipeline"] << "s    "
        << "%-" << cColumnWidth.cModule["process_id"] << "s    "
        << "%-" << cColumnWidth.cModule["type"] << "s    "
        << "%-" << cColumnWidth.cModule["type_name"] << "s    "
        << "%-" << cColumnWidth.cModule["start_time"] << "s    "
        << "%-" << cColumnWidth.cModule["state"] << "s    "
        << "%-" << cColumnWidth.cModule["state_name"] << "s    "
        << "%-" << cColumnWidth.cModule["role"] << "s    "
        << "%-" << cColumnWidth.cModule["role_name"] << "s    "
        << "%-" << cColumnWidth.cModule["url_pipe_in"] << "s    "
        << "%-" << cColumnWidth.cModule["url_pipe_out"] << "s    "
        << "%-" << cColumnWidth.cModule["url_listen"] << "s    "
        << "%-" << cColumnWidth.cModule["url_peer"] << "s    "
        << "%-" << cColumnWidth.cModule["idle"] << "s    "
        << "%-" << cColumnWidth.cModule["random_url"] << "s    "
        << "%-" << cColumnWidth.cModule["keys_incoming"] << "s    "
        << "%-" << cColumnWidth.cModule["keys_outgoing"] << "s    "
        << "%-" << cColumnWidth.cModule["key_bits_incoming"] << "s    "
        << "%-" << cColumnWidth.cModule["key_bits_outgoing"] << "s    "
        << "%-" << cColumnWidth.cModule["disclosed_bits_incoming"] << "s    "
        << "%-" << cColumnWidth.cModule["disclosed_bits_outgoing"] << "s    "
        << "%-" << cColumnWidth.cModule["debug"] << "s    "
        << "%-" << cColumnWidth.cModule["description"] << "s    "
        << "%-" << cColumnWidth.cModule["organisation"] << "s    "
        << "%-" << cColumnWidth.cModule["process_image"] << "s";
    }

    // the column format
    boost::format cFormat(ss.str());

    // print the heading
    if (bPrintHeader) {
        boost::format cHeading(cFormat);
        if (bPrintShort) {
            // short version
            cHeading
            % "type"
            % "id"
            % "dbus"
            % "keys_incoming"
            % "keys_outgoing"
            % "key_bits_incoming"
            % "key_bits_outgoing"
            % "pipeline"
            % "process_id"
            % "state_name"
            % "role_name"
            % "url_pipe_in"
            % "url_pipe_out"
            % "url_listen"
            % "url_peer"
            % "idle";
        } else if (bPrintModuleIO) {
            // module IO
            cHeading
            % "id"
            % "url_pipe_in"
            % "url_pipe_out"
            % "url_listen"
            % "url_peer";
        } else {
            // full version
            cHeading
            % "type"
            % "id"
            % "dbus"
            % "pipeline"
            % "process_id"
            % "type"
            % "type_name"
            % "start_time"
            % "state"
            % "state_name"
            % "role"
            % "role_name"
            % "url_pipe_in"
            % "url_pipe_out"
            % "url_listen"
            % "url_peer"
            % "idle"
            % "random_url"
            % "keys_incoming"
            % "keys_outgoing"
            % "key_bits_incoming"
            % "key_bits_outgoing"
            % "disclosed_bits_incoming"
            % "disclosed_bits_outgoing"
            % "debug"
            % "description"
            % "organisation"
            % "process_image";
        }
        cOut << cHeading.str() << std::endl;
    }

    // go over the found modules
    for (auto const &cModule : cModuleMap) {
        boost::format cModuleFormat(cFormat);
        if (bPrintShort) {
            // short version
            cModuleFormat % "module:";
            cModuleFormat % cModule.second.at("id");
            cModuleFormat % cModule.second.at("dbus");
            cModuleFormat % cModule.second.at("keys_incoming");
            cModuleFormat % cModule.second.at("keys_outgoing");
            cModuleFormat % cModule.second.at("key_bits_incoming");
            cModuleFormat % cModule.second.at("key_bits_outgoing");
            cModuleFormat % cModule.second.at("pipeline");
            cModuleFormat % cModule.second.at("process_id");
            cModuleFormat % cModule.second.at("state_name");
            cModuleFormat % cModule.second.at("role_name");
            cModuleFormat % cModule.second.at("url_pipe_in");
            cModuleFormat % cModule.second.at("url_pipe_out");
            cModuleFormat % cModule.second.at("url_listen");
            cModuleFormat % cModule.second.at("url_peer");
            cModuleFormat % cModule.second.at("idle");
        }
        else if (bPrintModuleIO) {
            // short version
            cModuleFormat % cModule.second.at("id");
            cModuleFormat % cModule.second.at("url_pipe_in");
            cModuleFormat % cModule.second.at("url_pipe_out");
            cModuleFormat % cModule.second.at("url_listen");
            cModuleFormat % cModule.second.at("url_peer");
        }
        else {
            // full version
            cModuleFormat % "module:";
            cModuleFormat % cModule.second.at("id");
            cModuleFormat % cModule.second.at("dbus");
            cModuleFormat % cModule.second.at("pipeline");
            cModuleFormat % cModule.second.at("process_id");
            cModuleFormat % cModule.second.at("type");
            cModuleFormat % cModule.second.at("type_name");
            cModuleFormat % cModule.second.at("start_time");
            cModuleFormat % cModule.second.at("state");
            cModuleFormat % cModule.second.at("state_name");
            cModuleFormat % cModule.second.at("role");
            cModuleFormat % cModule.second.at("role_name");
            cModuleFormat % cModule.second.at("url_pipe_in");
            cModuleFormat % cModule.second.at("url_pipe_out");
            cModuleFormat % cModule.second.at("url_listen");
            cModuleFormat % cModule.second.at("url_peer");
            cModuleFormat % cModule.second.at("idle");
            cModuleFormat % cModule.second.at("random_url");
            cModuleFormat % cModule.second.at("keys_incoming");
            cModuleFormat % cModule.second.at("keys_outgoing");
            cModuleFormat % cModule.second.at("key_bits_incoming");
            cModuleFormat % cModule.second.at("key_bits_outgoing");
            cModuleFormat % cModule.second.at("disclosed_bits_incoming");
            cModuleFormat % cModule.second.at("disclosed_bits_outgoing");
            cModuleFormat % cModule.second.at("debug");
            cModuleFormat % cModule.second.at("description");
            cModuleFormat % cModule.second.at("organisation");
            cModuleFormat % cModule.second.at("process_image");
        }

        cOut << cModuleFormat.str() << std::endl;
    }

    // add an artificial empty line for nice viewing if headers are on
    if (bPrintHeader) cOut << std::endl;
}

/**
 * Writes node information to the specified stream.
 * @param  cOut            the stream to write to.
 * @param  cNodeMap        the particular node data to format.
 */
void tabular_output::dump_nodes(std::ostream &cOut, const std::map<std::string, qkd::utility::properties> &cNodeMap) {
    // something to show at all?
    if (cNodeMap.empty()) return;

    // set the column width for nodes
    cColumnWidth.cNode.clear();
    for (auto const & cNode : cNodeMap) {
        set_column_width(cColumnWidth.cNode, cNode.second);
    }

    // construct the format string: "node:" - id - dbus - start_time - process_id - process_image - config_file - random_url - debug
    std::stringstream ss;
    ss << "%-5s    "
    << "%-" << cColumnWidth.cNode["id"] << "s    "
    << "%-" << cColumnWidth.cNode["dbus"] << "s    "
    << "%-" << cColumnWidth.cNode["start_time"] << "s    "
    << "%-" << cColumnWidth.cNode["process_id"] << "s    "
    << "%-" << cColumnWidth.cNode["process_image"] << "s    "
    << "%-" << cColumnWidth.cNode["config_file"] << "s    "
    << "%-" << cColumnWidth.cNode["random_url"] << "s    "
    << "%-" << cColumnWidth.cNode["debug"] << "s";

    // the column format
    boost::format cFormat(ss.str());

    // print the heading
    if (bPrintHeader) {
        boost::format cHeading(cFormat);
        cHeading
        % "type"
        % "id"
        % "dbus"
        % "start_time"
        % "process_id"
        % "process_image"
        % "config_file"
        % "random_url"
        % "debug";

        cOut << cHeading.str() << std::endl;
    }

    // go over the found nodes
    for (auto const & cNode : cNodeMap) {
        boost::format cNodeFormat(cFormat);

        cNodeFormat % "node:";
        cNodeFormat % cNode.second.at("id");
        cNodeFormat % cNode.second.at("dbus");
        cNodeFormat % cNode.second.at("start_time");
        cNodeFormat % cNode.second.at("process_id");
        cNodeFormat % cNode.second.at("process_image");
        cNodeFormat % cNode.second.at("config_file");
        cNodeFormat % cNode.second.at("random_url");
        cNodeFormat % cNode.second.at("debug");

        cOut << cNodeFormat.str() << std::endl;
    }

    // add an artificial empty line for nice viewing if headers are on
    if (bPrintHeader) cOut << std::endl;
}