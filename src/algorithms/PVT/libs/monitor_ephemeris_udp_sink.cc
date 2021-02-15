/*!
 * \file monitor_pvt_udp_sink.cc
 * \brief Implementation of a class that sends serialized Monitor_Pvt
 * objects over udp to one or multiple endpoints
 * \author Álvaro Cebrián Juan, 2019. acebrianjuan(at)gmail.com
 *
 * -----------------------------------------------------------------------------
 *
 * GNSS-SDR is a Global Navigation Satellite System software-defined receiver.
 * This file is part of GNSS-SDR.
 *
 * Copyright (C) 2010-2020  (see AUTHORS file for a list of contributors)
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * -----------------------------------------------------------------------------
 */

#include "monitor_ephemeris_udp_sink.h"
#include <boost/archive/binary_oarchive.hpp>
#include <iostream>
#include <sstream>


Monitor_Ephemeris_Udp_Sink::Monitor_Ephemeris_Udp_Sink(const std::vector<std::string>& addresses, const uint16_t& port, bool protobuf_enabled) : socket{io_context}
{
    for (const auto& address : addresses)
        {
            boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(address, error), port);
            endpoints.push_back(endpoint);
        }

    use_protobuf = protobuf_enabled;
    if (use_protobuf)
        {
            serdes_gal = Serdes_Galileo_Eph();
            serdes_gps = Serdes_Gps_Eph();
        }
}


bool Monitor_Ephemeris_Udp_Sink::write_galileo_ephemeris(const Galileo_Ephemeris* const monitor_gal_eph)
{
    std::string outbound_data;
    if (use_protobuf == false)
        {
            std::ostringstream archive_stream;
            boost::archive::binary_oarchive oa{archive_stream};
            oa << *monitor_gal_eph;
            outbound_data = archive_stream.str();
        }
    else
        {
            outbound_data = "E";
            outbound_data.append(serdes_gal.createProtobuffer(monitor_gal_eph));
        }

    for (const auto& endpoint : endpoints)
        {
            socket.open(endpoint.protocol(), error);
            socket.connect(endpoint, error);

            try
                {
                    if (socket.send(boost::asio::buffer(outbound_data)) == 0)
                        {
                            return false;
                        }
                }
            catch (boost::system::system_error const& e)
                {
                    return false;
                }
        }
    return true;
}

bool Monitor_Ephemeris_Udp_Sink::write_gps_ephemeris(const std::shared_ptr<Gps_Ephemeris> monitor_gps_eph)
{
    std::string outbound_data;
    if (use_protobuf == false)
        {
            std::ostringstream archive_stream;
            boost::archive::binary_oarchive oa{archive_stream};
            oa << *monitor_gps_eph;
            outbound_data = archive_stream.str();
        }
    else
        {
            outbound_data = "G";
            outbound_data.append(serdes_gps.createProtobuffer(monitor_gps_eph));
        }

    for (const auto& endpoint : endpoints)
        {
            socket.open(endpoint.protocol(), error);
            socket.connect(endpoint, error);

            try
                {
                    if (socket.send(boost::asio::buffer(outbound_data)) == 0)
                        {
                            return false;
                        }
                }
            catch (boost::system::system_error const& e)
                {
                    return false;
                }
        }
    return true;
}
