# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: ait-backtrace
#
# Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
#
# Copyright (C) 2012-2015 AIT Austrian Institute of Technology
# AIT Austrian Institute of Technology GmbH
# Donau-City-Strasse 1 | 1220 Vienna | Austria
# http://www.ait.ac.at
#
# This file is part of the AIT QKD Software Suite.
#
# The AIT QKD Software Suite is free software: you can redistribute 
# it and/or modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation, either version 3 of 
# the License, or (at your option) any later version.
# 
# The AIT QKD Software Suite is distributed in the hope that it will 
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the AIT QKD Software Suite. 
# If not, see <http://www.gnu.org/licenses/>.
# ------------------------------------------------------------


# ------------------------------------------------------------
# sources and linkage

# additional includes
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}) 

# define the sources
set(QKD_SRC 

    # basic QKD key
    key/key.cpp
    key/key_ring.cpp
    
    # crypto engine
    crypto/association.cpp
    crypto/context.cpp
    crypto/evhash.cpp
    crypto/gf2.cpp
    crypto/crypto_evhash.cpp
    crypto/crypto_null.cpp
    crypto/crypto_xor.cpp
    crypto/engine.cpp
    crypto/scheme.cpp
    
    # old and dirty crypto stuff
    crypto/old/evhash/evhash-32.cpp
    crypto/old/evhash/evhash-64.cpp
    crypto/old/evhash/evhash-96.cpp
    crypto/old/evhash/evhash-128.cpp
    crypto/old/evhash/evhash-256.cpp
    crypto/old/simple-state.cpp
    crypto/old/utility.cpp

    # atof - locale "C" version enforced
    utility/atof/atof.cpp

    # moving average
    utility/average/average.cpp
    utility/average/average_data.cpp
    utility/average/average_time.cpp
    utility/average/average_value.cpp

    # backtrace on SIGNALs
    utility/backtrace/backtrace.cpp
    
    # bigint implementation
    utility/bigint/bigint.cpp
    
    # buffer implementation
    utility/buffer/buffer.cpp
    
    # checksum on memory blocks
    utility/checksum/checksum.cpp
    utility/checksum/checksum_crc32.cpp
    utility/checksum/checksum_md5.cpp
    utility/checksum/checksum_sha1.cpp
    
    # DBus helpers
    utility/dbus/dbus.cpp
    
    # general debug flag
    utility/debug/debug.cpp
    
    # environment paths (home, share, ...)
    utility/environment/environment.cpp

    # QKD system investigation
    utility/investigation/investigation.cpp
    utility/investigation/investigation_dbus.cpp
    
    # save memory handling
    utility/memory/memory.cpp
    
    # thread-safe queue
    utility/queue/queue.cpp
    
    # properties
    utility/properties/properties.cpp
    
    # random number sources
    utility/random/random.cpp
    utility/random/random_c_api.cpp
    utility/random/random_cbc_aes.cpp
    utility/random/random_file.cpp
    utility/random/random_hmac_sha.cpp
    
    # shannon handy functions
    utility/shannon/shannon.cpp
    
    # si-units utility
    utility/si_units/si_units.cpp
    
    # syslog interface
    utility/syslog/syslog.cpp
    
    # compress/decompress
    utility/zip/zip.cpp
    
    # Q3P
    q3p/db/db.cpp
    q3p/db/db_file.cpp
    q3p/db/db_null.cpp
    q3p/db/db_ram.cpp
    q3p/engine/channel.cpp
    q3p/engine/engine.cpp
    q3p/engine/message.cpp
    q3p/engine/mq.cpp
    q3p/engine/nic.cpp
    q3p/engine/protocol/data.cpp
    q3p/engine/protocol/handshake.cpp
    q3p/engine/protocol/key_move.cpp
    q3p/engine/protocol/load.cpp
    q3p/engine/protocol/load_request.cpp
    q3p/engine/protocol/protocol.cpp
    q3p/engine/protocol/store.cpp
    
    # Q3P IPSec implementation
    q3p/engine/ipsec/CipherValidator.cpp
    q3p/engine/ipsec/IPAddress.cpp
    q3p/engine/ipsec/IPsecConnectionManager.cpp
    q3p/engine/ipsec/NetlinkDeleteSAMessage.cpp
    q3p/engine/ipsec/NetlinkDeleteSPMessage.cpp
    q3p/engine/ipsec/NetlinkIPsecManager.cpp
    q3p/engine/ipsec/NetlinkMessage.cpp
    q3p/engine/ipsec/NetlinkNewSAMessage.cpp
    q3p/engine/ipsec/NetlinkNewSPMessage.cpp
    q3p/engine/ipsec/NetlinkSAConfig.cpp
    q3p/engine/ipsec/NetlinkSPConfig.cpp
    q3p/engine/ipsec/TunTapManager.cpp
    q3p/engine/ipsec/Utils.cpp
    
    # QKD module
    module/communicator.cpp
    module/message.cpp
    module/module.cpp
    module/module_internal.cpp
    
    # widgets
    widget/lcd.cpp
    widget/led.cpp
    widget/module_frame.cpp
    widget/plot.cpp
    widget/res.cpp
    
    # versioning
    version/version.cpp
)

# Qt UIC
QT4_WRAP_UI(${CMAKE_CURRENT_BINARY_DIR}/ui_module_frame.h widget/module_frame.ui)
set(QKD_UIC
    ui_module_frame.h
)


# Qt MOC
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/engine/protocol/data.h         ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine_protocol_data.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/engine/protocol/handshake.h    ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine_protocol_handshake.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/engine/protocol/key_move.h     ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine_protocol_key_move.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/engine/protocol/load.h         ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine_protocol_load.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/engine/protocol/load_request.h ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine_protocol_load_request.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/engine/protocol/protocol.h     ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine_protocol_protocol.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/engine/protocol/store.h        ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine_protocol_store.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/q3p/db.h                   ${CMAKE_CURRENT_BINARY_DIR}/q3p_db.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/db/db_file.h                   ${CMAKE_CURRENT_BINARY_DIR}/q3p_db_file.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/db/db_null.h                   ${CMAKE_CURRENT_BINARY_DIR}/q3p_db_null.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/lib/q3p/db/db_ram.h                    ${CMAKE_CURRENT_BINARY_DIR}/q3p_db_ram.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/q3p/engine.h               ${CMAKE_CURRENT_BINARY_DIR}/q3p_engine.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/q3p/mq.h                   ${CMAKE_CURRENT_BINARY_DIR}/q3p_mq.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/q3p/nic.h                  ${CMAKE_CURRENT_BINARY_DIR}/q3p_nic.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/module/module.h            ${CMAKE_CURRENT_BINARY_DIR}/qkd_module.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/widget/lcd.h               ${CMAKE_CURRENT_BINARY_DIR}/qkd_widget_lcd.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/widget/led.h               ${CMAKE_CURRENT_BINARY_DIR}/qkd_widget_led.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/widget/module_frame.h      ${CMAKE_CURRENT_BINARY_DIR}/qkd_widget_module_frame.moc.cpp) 
QT4_GENERATE_MOC(${CMAKE_SOURCE_DIR}/include/qkd/widget/plot.h              ${CMAKE_CURRENT_BINARY_DIR}/qkd_widget_plot.moc.cpp) 
set(QKD_MOC
    
    q3p_engine_protocol_data.moc.cpp
    q3p_engine_protocol_handshake.moc.cpp
    q3p_engine_protocol_key_move.moc.cpp
    q3p_engine_protocol_load.moc.cpp
    q3p_engine_protocol_load_request.moc.cpp
    q3p_engine_protocol_protocol.moc.cpp
    q3p_engine_protocol_store.moc.cpp
    
    q3p_db.moc.cpp
    q3p_db_file.moc.cpp
    q3p_db_null.moc.cpp
    q3p_db_ram.moc.cpp
    
    q3p_engine.moc.cpp
    q3p_mq.moc.cpp
    q3p_nic.moc.cpp
    
    qkd_module.moc.cpp
    
    qkd_widget_lcd.moc.cpp
    qkd_widget_led.moc.cpp
    qkd_widget_module_frame.moc.cpp
    qkd_widget_plot.moc.cpp
)


# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(${CMAKE_SOURCE_DIR}/include/qkd/q3p/db.h            ${CMAKE_CURRENT_BINARY_DIR}/db.xml)
QT4_GENERATE_DBUS_INTERFACE(${CMAKE_SOURCE_DIR}/include/qkd/module/module.h     ${CMAKE_CURRENT_BINARY_DIR}/module.xml)
QT4_GENERATE_DBUS_INTERFACE(${CMAKE_SOURCE_DIR}/include/qkd/q3p/engine.h        ${CMAKE_CURRENT_BINARY_DIR}/engine.xml)
QT4_GENERATE_DBUS_INTERFACE(${CMAKE_SOURCE_DIR}/include/qkd/q3p/mq.h            ${CMAKE_CURRENT_BINARY_DIR}/mq.xml)
QT4_GENERATE_DBUS_INTERFACE(${CMAKE_SOURCE_DIR}/include/qkd/q3p/nic.h           ${CMAKE_CURRENT_BINARY_DIR}/nic.xml)
QT4_ADD_DBUS_ADAPTOR(DB_DBUS_SRC 
    ${CMAKE_CURRENT_BINARY_DIR}/db.xml 
    ${CMAKE_SOURCE_DIR}/include/qkd/q3p/db.h
    qkd::q3p::db
    db_dbus)
QT4_ADD_DBUS_ADAPTOR(MODULE_DBUS_SRC 
    ${CMAKE_CURRENT_BINARY_DIR}/module.xml 
    ${CMAKE_SOURCE_DIR}/include/qkd/module/module.h
    qkd::module::module
    module_dbus)
QT4_ADD_DBUS_ADAPTOR(ENGINE_DBUS_SRC 
    ${CMAKE_CURRENT_BINARY_DIR}/engine.xml 
    ${CMAKE_SOURCE_DIR}/include/qkd/q3p/engine.h
    qkd::q3p::engine_instance
    engine_dbus)
QT4_ADD_DBUS_ADAPTOR(MQ_DBUS_SRC 
    ${CMAKE_CURRENT_BINARY_DIR}/mq.xml 
    ${CMAKE_SOURCE_DIR}/include/qkd/q3p/mq.h
    qkd::q3p::mq_instance
    mq_dbus)
QT4_ADD_DBUS_ADAPTOR(NIC_DBUS_SRC 
    ${CMAKE_CURRENT_BINARY_DIR}/nic.xml 
    ${CMAKE_SOURCE_DIR}/include/qkd/q3p/nic.h
    qkd::q3p::nic_instance
    nic_dbus)


# a macro to convert any binary to a source file
macro(CONVERT_TO_HEADER infile outfile)

    # ensure full absolute paths
    get_filename_component(abs_infile ${infile} ABSOLUTE)
    get_filename_component(name_infile ${infile} NAME)
    set(abs_outfile "${outfile}")
    if (NOT IS_ABSOLUTE "${outfile}")
        set(abs_outfile "${CMAKE_CURRENT_BINARY_DIR}/${outfile}")
    endif(NOT IS_ABSOLUTE "${outfile}")
   
    # this is the command
    add_custom_command(OUTPUT ${abs_outfile}
        COMMAND ln
        ARGS -s ${abs_infile} ${name_infile}
        COMMAND xxd
        ARGS -i ${name_infile} ${abs_outfile}
        COMMAND rm
        ARGS ${name_infile}
        DEPENDS ${abs_infile} VERBATIM)
   
    set_source_files_properties(${abs_outfile} PROPERTIES HEADER_FILE_ONLY (true))
    
endmacro(CONVERT_TO_HEADER)


# collect resources as binary

CONVERT_TO_HEADER(widget/res/alice.png                          ${CMAKE_CURRENT_BINARY_DIR}/res_alice.png.h)
CONVERT_TO_HEADER(widget/res/bob.png                            ${CMAKE_CURRENT_BINARY_DIR}/res_bob.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_green.png             ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_green.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_green_small.png       ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_green_small.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_grey.png              ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_grey.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_grey_small.png        ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_grey_small.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_red.png               ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_red.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_red_small.png         ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_red_small.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_yellow.png            ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_yellow.png.h)
CONVERT_TO_HEADER(widget/res/glass_button_yellow_small.png      ${CMAKE_CURRENT_BINARY_DIR}/res_glass_button_yellow_small.png.h)
CONVERT_TO_HEADER(widget/res/media_eject.png                    ${CMAKE_CURRENT_BINARY_DIR}/res_media_eject.png.h)
CONVERT_TO_HEADER(widget/res/media_playback_pause.png           ${CMAKE_CURRENT_BINARY_DIR}/res_media_playback_pause.png.h)
CONVERT_TO_HEADER(widget/res/media_playback_start.png           ${CMAKE_CURRENT_BINARY_DIR}/res_media_playback_start.png.h)
CONVERT_TO_HEADER(widget/res/media_playback_stop.png            ${CMAKE_CURRENT_BINARY_DIR}/res_media_playback_stop.png.h)
CONVERT_TO_HEADER(widget/res/media_record.png                   ${CMAKE_CURRENT_BINARY_DIR}/res_media_record.png.h)
CONVERT_TO_HEADER(widget/res/module_confirmation.png            ${CMAKE_CURRENT_BINARY_DIR}/res_module_confirmation.png.h)
CONVERT_TO_HEADER(widget/res/module_error_correction.png        ${CMAKE_CURRENT_BINARY_DIR}/res_module_error_correction.png.h)
CONVERT_TO_HEADER(widget/res/module_error_estimation.png        ${CMAKE_CURRENT_BINARY_DIR}/res_module_error_estimation.png.h)
CONVERT_TO_HEADER(widget/res/module_keystore.png                ${CMAKE_CURRENT_BINARY_DIR}/res_module_keystore.png.h)
CONVERT_TO_HEADER(widget/res/module_other.png                   ${CMAKE_CURRENT_BINARY_DIR}/res_module_other.png.h)
CONVERT_TO_HEADER(widget/res/module_peer.png                    ${CMAKE_CURRENT_BINARY_DIR}/res_module_peer.png.h)
CONVERT_TO_HEADER(widget/res/module_pipe_in.png                 ${CMAKE_CURRENT_BINARY_DIR}/res_module_pipe_in.png.h)
CONVERT_TO_HEADER(widget/res/module_pipe_out.png                ${CMAKE_CURRENT_BINARY_DIR}/res_module_pipe_out.png.h)
CONVERT_TO_HEADER(widget/res/module_presifting.png              ${CMAKE_CURRENT_BINARY_DIR}/res_module_presifting.png.h)
CONVERT_TO_HEADER(widget/res/module_privacy_amplification.png   ${CMAKE_CURRENT_BINARY_DIR}/res_module_privacy_amplification.png.h)
CONVERT_TO_HEADER(widget/res/module_sifting.png                 ${CMAKE_CURRENT_BINARY_DIR}/res_module_sifting.png.h)
CONVERT_TO_HEADER(widget/res/module.png                         ${CMAKE_CURRENT_BINARY_DIR}/res_module.png.h)
CONVERT_TO_HEADER(widget/res/wwDigital.ttf                      ${CMAKE_CURRENT_BINARY_DIR}/res_wwDigital.ttf.h)
set(QKD_RES
    res_alice.png.h
    res_bob.png.h
    res_glass_button_green.png.h
    res_glass_button_green_small.png.h
    res_glass_button_grey.png.h
    res_glass_button_grey_small.png.h
    res_glass_button_red.png.h
    res_glass_button_red_small.png.h
    res_glass_button_yellow.png.h
    res_glass_button_yellow_small.png.h
    res_media_eject.png.h
    res_media_playback_pause.png.h
    res_media_playback_start.png.h
    res_media_playback_stop.png.h
    res_media_record.png.h
    res_module_confirmation.png.h
    res_module_error_correction.png.h
    res_module_error_estimation.png.h
    res_module_keystore.png.h
    res_module_other.png.h
    res_module_peer.png.h
    res_module_peer.png.h
    res_module_pipe_in.png.h
    res_module_pipe_out.png.h
    res_module_presifting.png.h
    res_module_privacy_amplification.png.h
    res_module_sifting.png.h
    res_module.png.h
    res_wwDigital.ttf.h
)
    
    
# define executables
add_library(qkd SHARED ${QKD_SRC} ${QKD_UIC} ${QKD_MOC} ${QKD_RES} ${MODULE_DBUS_SRC} ${ENGINE_DBUS_SRC} ${DB_DBUS_SRC} ${MQ_DBUS_SRC} ${NIC_DBUS_SRC})
target_link_libraries(qkd ${CMAKE_REQUIRED_LIBRARIES})
set_target_properties(qkd PROPERTIES VERSION ${VERSION})


# ------------------------------------------------------------
# installation

# install
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/qkd DESTINATION include COMPONENT header)
install(TARGETS qkd LIBRARY DESTINATION lib COMPONENT library)

