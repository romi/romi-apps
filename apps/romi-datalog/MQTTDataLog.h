/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#ifndef __ROMI_MQTTDATALOG_H
#define __ROMI_MQTTDATALOG_H

#include <memory>
#include <vector>
#include <thread>
#include <mosquitto.h>
#include <api/IDataLog.h>

namespace romi {

        class MQTTDataLog : public IDataLog
        {
        protected:

                static constexpr const char * kHost = "test.mosquitto.org";
                static const int kPort = 8883;
                static const int kKeepAlive = 60;
                static constexpr const char *kTopic = "romi/topic";
                static constexpr const char *kCAFile = "/tmp/crt.pem";

                std::string topic_;
                struct mosquitto *mosq_;
                bool connected_;
                std::unique_ptr<std::thread> thread_;
                bool quitting_;
                
                MQTTDataLog(const MQTTDataLog& other) = delete;
                MQTTDataLog& operator=(const MQTTDataLog& other) = delete;

                void create_cafile();
                void check_network_events();
                
        public:
                MQTTDataLog(const std::string& topic);
                ~MQTTDataLog() override;

                void store(double time, const std::string& topic,
                           const std::string& name, double value) override;
                void store(double time, const std::string& name, double value) override;
        };
}

#endif // __ROMI_MQTTDATALOG_H
