/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the
 * License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file udr_app.hpp
 \brief
 \author  Tien-Thinh NGUYEN
 \company Eurecom
 \date 2020
 \email: Tien-Thinh.Nguyen@eurecom.fr
 */

#ifndef FILE_UDR_APP_HPP_SEEN
#define FILE_UDR_APP_HPP_SEEN

#include <string>
#include <pistache/http.h>
#include <map>
#include <shared_mutex>
#include <nlohmann/json.hpp>

namespace oai {
namespace udr {
namespace app {

// class ausf_config;
class udr_app {
 public:
  explicit udr_app(const std::string& config_file);
  udr_app(udr_app const&) = delete;
  void operator=(udr_app const&) = delete;

  virtual ~udr_app();

  void handle_access_mobility_subscription_data_document(
		  const std::string &ue_id, const std::string &serving_plmn_id, nlohmann::json& response_data,
      Pistache::Http::Code& code);

 private:
};
}  // namespace app
}  // namespace udr
}  // namespace oai
#include "udr_config.hpp"

#endif /* FILE_UDR_APP_HPP_SEEN */
