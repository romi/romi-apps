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
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <util/Logger.h>
#include <util/StringUtils.h>
#include <util/ClockAccessor.h>
#include "crt.h"
#include "MQTTDataLog.h"

namespace romi {

        MQTTDataLog::MQTTDataLog(const std::string& device)
                : device_(device),
                  mosq_(nullptr),
                  connected_(false),
                  thread_(nullptr),
                  quitting_(false) 
        {
                create_cafile();

                mosquitto_lib_init();

                mosq_ = mosquitto_new("romi-cablebot-001", true, nullptr);
                if (!mosq_){
                        r_err("MQTTDataLog: mosquitto_new failed");
                        mosquitto_lib_cleanup();
                        throw std::runtime_error("MQTTDataLog: mosquitto_new failed");
                }

                // TLS setup
                if (mosquitto_tls_set(mosq_, kCAFile, nullptr,
                                      nullptr, nullptr, nullptr) != MOSQ_ERR_SUCCESS) {
                        r_err("MQTTDataLog: Failed to set TLS options.");
                        mosquitto_destroy(mosq_);
                        mosquitto_lib_cleanup();
                        throw std::runtime_error("MQTTDataLog: Failed to set TLS options");
                }

                // Require valid cert and enforce TLS v1.2+
                mosquitto_tls_opts_set(mosq_, 1, "tlsv1.2", nullptr);

                //mosquitto_connect_callback_set(mosq_, on_connect);

                int rc = mosquitto_connect(mosq_, kHost, kPort, kKeepAlive);
                if (rc != MOSQ_ERR_SUCCESS) {
                        r_err("MQTTDataLog: Connect failed: %s", mosquitto_strerror(rc));
                        mosquitto_destroy(mosq_);
                        mosquitto_lib_cleanup();
                        throw std::runtime_error("MQTTDataLog: Connect failed");
                }

                thread_ = std::make_unique<std::thread>([this]() {
                        this->check_network_events();
                });
        }
        
        MQTTDataLog::~MQTTDataLog()
        {
                quitting_ = true;
                if (thread_ != nullptr) {
                        thread_->join();
                }
                if (mosq_) {
                        mosquitto_destroy(mosq_);
                }
                mosquitto_lib_cleanup();
        }

        void MQTTDataLog::check_network_events()
        {
                auto clock = romi::ClockAccessor::GetInstance();
                
                try {
                        while (!quitting_) {
                                int ret = mosquitto_loop(mosq_, 0, 1);
                                if (ret != MOSQ_ERR_SUCCESS) {
                                        r_err("MQTTDataLog: loop failed: %s",
                                              mosquitto_strerror(ret));
                                }
                                clock->sleep(0.1);
                        }
                } catch (...) {
                        r_err("MQTTDataLog::check_network_events: caught exception");
                }
        }
        
        void MQTTDataLog::store(double time, const std::string& topic,
                                const std::string& name, double value)
        {
                char msg[256];
                snprintf(msg, sizeof(msg) - 1,
                         "{\"time\"=%.3f,\"value\"=%.3f}", time, value);
                msg[255] = 0;

                char mqtt_topic[256];
                snprintf(mqtt_topic, sizeof(mqtt_topic) - 1,
                         "romi/%s/%s/%s", device_.c_str(), topic.c_str(), name.c_str());
                mqtt_topic[255] = 0;

                // romi/<device-id>/<node-topic>/<name> {'time'=time,'value'=value}
                
                int ret = mosquitto_publish(mosq_, nullptr, mqtt_topic,
                                            (int) strlen(msg), msg,
                                            0, false);
                if (ret == MOSQ_ERR_SUCCESS) {
                        r_debug("MQTTDataLog: %s -> %s", msg, mqtt_topic);
                } else {
                        r_err("MQTTDataLog: Publish failed: %s", mosquitto_strerror(ret));
                }
        }

        void MQTTDataLog::create_cafile()
        {
                std::ofstream out(kCAFile);
                out << kMQTTCertificate;
                out.close();
        }
}

