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

#include "util/Logger.h"
#include "util/ImageIO.h"
#include "rpc/RemoteCamera.h"
#include "rpc/MethodsCamera.h"
#include "rpc/MethodsPowerDevice.h"

namespace romi {
        
        RemoteCamera::RemoteCamera(std::unique_ptr<rcom::IRPCClient>& client)
                : RemoteStub(client), output_()
        {
        }
        
        bool RemoteCamera::set_value(const std::string& name, double value)
        {
                nlohmann::json params;
                params[MethodsCamera::kSettingName] = name;
                params[MethodsCamera::kSettingValue] = value;
                return execute_with_params(MethodsCamera::kSetValue, params);
        }
        
        bool RemoteCamera::select_option(const std::string& name,
                                         const std::string& value)
        {
                nlohmann::json params;
                params[MethodsCamera::kOptionName] = name;
                params[MethodsCamera::kOptionValue] = value;
                return execute_with_params(MethodsCamera::kSelectOption, params);
        }
        
        bool RemoteCamera::grab(Image &image) 
        {
                bool success = false;
                rcom::MemBuffer& jpeg = grab_jpeg();
                if (jpeg.size() > 0) {
                        success = ImageIO::load_from_buffer(image, jpeg.data());
                }
                return success;
        }
        
        rcom::MemBuffer& RemoteCamera::grab_jpeg()
        {
                nlohmann::json params;
                rcom::RPCError error;

                output_.clear();
                
                client_->execute("", MethodsCamera::kGrabJpegBinary,
                                 params, output_, error);
                
                if (error.code != 0) {
                        r_warn("RemoteCamera::grab_jpeg: %s", error.message.c_str());
                }

                return output_;
        }
        
        bool RemoteCamera::power_up()
        {
                r_debug("RemoteCamera::power_up");
                return execute_simple_request(MethodsPowerDevice::power_up);
        }
        
        bool RemoteCamera::power_down()
        {
                r_debug("RemoteCamera::power_down");
                return execute_simple_request(MethodsPowerDevice::power_down);
        }
        
        bool RemoteCamera::stand_by()
        {
                r_debug("RemoteCamera::stand_by");
                return execute_simple_request(MethodsPowerDevice::stand_by);
        }
        
        bool RemoteCamera::wake_up()
        {
                r_debug("RemoteCamera::wake_up");
                return execute_simple_request(MethodsPowerDevice::wake_up);
        }

        const ICameraSettings& RemoteCamera::get_settings()
        {
                throw std::runtime_error("RemoteCamera::get_settings: not implemented");
        }
}
