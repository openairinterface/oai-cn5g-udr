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

/*! \file udr_nrf.cpp
 \brief
 \author
 \company Eurecom
 \date 2020
 \email: contact@openairinterface.org
 */

#include "udr_nrf.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "3gpp_29.500.h"
#include "logger.hpp"
#include "udr.h"
#include "udr_app.hpp"
#include "udr_client.hpp"
#include "udr_profile.hpp"

using namespace oai::udr::config;
using namespace oai::udr::app;
using namespace boost::placeholders;

using json = nlohmann::json;

extern udr_config udr_cfg;
extern udr_nrf* udr_nrf_inst;
udr_client* udr_client_inst = nullptr;

//------------------------------------------------------------------------------
udr_nrf::udr_nrf(udr_event& ev) : m_event_sub(ev) {
  // generate UUID
  udr_instance_id = to_string(boost::uuids::random_generator()());
  // Generate UDR profile
  generate_udr_profile();
}

//------------------------------------------------------------------------------
void udr_nrf::start() {
  // Register to NRF
  if (udr_cfg.register_nrf) {
    Logger::udr_app().info("NRF TASK Created ");
    register_to_nrf();
  }
}

//------------------------------------------------------------------------------
void udr_nrf::stop() {
  // Deregister to NRF
  if (udr_cfg.register_nrf) {
    deregister_to_nrf();
  }
}

//---------------------------------------------------------------------------------------------
void udr_nrf::get_nrf_api_root(std::string& api_root) {
  api_root =
      udr_cfg.nrf_addr.uri_root + NNRF_NFM_BASE + udr_cfg.nrf_addr.api_version;
}

//---------------------------------------------------------------------------------------------
void udr_nrf::generate_udr_profile() {
  // TODO: remove hardcoded values
  udr_nf_profile.set_nf_instance_id(udr_instance_id);
  udr_nf_profile.set_nf_instance_name("OAI-UDR");
  udr_nf_profile.set_nf_type("UDR");
  udr_nf_profile.set_nf_status("REGISTERED");
  udr_nf_profile.set_nf_heartBeat_timer(50);
  udr_nf_profile.set_nf_priority(1);
  udr_nf_profile.set_nf_capacity(100);
  // udr_nf_profile.set_fqdn(udr_cfg.fqdn);
  udr_nf_profile.add_nf_ipv4_addresses(udr_cfg.nudr.addr4);

  // UDR info (Hardcoded for now)
  // ToDo:-If none of these parameters are provided, the UDR can serve any
  // external group and any SUPI or GPSI managed by the PLMN of the UDR
  // instance. If "supiRanges", "gpsiRanges" and
  // "externalGroupIdentifiersRanges" attributes are absent, and "groupId" is
  // present, the SUPIs / GPSIs / ExternalGroups served by this UDR instance is
  // determined by the NRF (see 3GPP TS 23.501 [2], clause 6.2.6.2).
  udr_info_t udr_info_item;
  supi_range_udr_info_item_t supi_ranges;
  udr_info_item.groupid = "oai-udr-testgroupid";
  udr_info_item.data_set_id.push_back("0210");
  udr_info_item.data_set_id.push_back("9876");
  supi_ranges.supi_range.start   = "208950000000031";
  supi_ranges.supi_range.pattern = "^imsi-20895[31-131]{6}$";
  supi_ranges.supi_range.start   = "208950000000131";
  udr_info_item.supi_ranges.push_back(supi_ranges);
  identity_range_udr_info_item_t gpsi_ranges;
  gpsi_ranges.identity_range.start   = "752740000";
  gpsi_ranges.identity_range.pattern = "^gpsi-75274[0-9]{4}$";
  gpsi_ranges.identity_range.end     = "752749999";
  udr_info_item.gpsi_ranges.push_back(gpsi_ranges);
  udr_nf_profile.set_udr_info(udr_info_item);
  // ToDo:- Add remaining fields
  // UDR info item end

  udr_nf_profile.display();
}
//---------------------------------------------------------------------------------------------
void udr_nrf::register_to_nrf() {
  nlohmann::json response_data = {};

  // Send NF registration request
  std::string nrf_api_root = {};
  std::string response     = {};
  std::string method       = {"PUT"};
  long response_code       = {0};
  get_nrf_api_root(nrf_api_root);
  std::string remote_uri = nrf_api_root + UDR_NF_REGISTER_URL + udr_instance_id;
  nlohmann::json json_data = {};
  udr_nf_profile.to_json(json_data);

  Logger::udr_nrf().info("Sending NF Registration request");
  bool is_registration_success = false;

  if (udr_client_inst->curl_http_client(
          remote_uri, method, json_data.dump().c_str(), response,
          response_code)) {
    try {
      response_data = nlohmann::json::parse(response);
      // TODO: use Heart-beart timer interval returned from NRF
      if (response.find("REGISTERED") != 0) {
        stop_nrf_registration_retry();
        start_event_nf_heartbeat(remote_uri);
        is_registration_success = true;
      }
    } catch (nlohmann::json::exception& e) {
      Logger::udr_nrf().info(
          "NF Registration procedure failed - cannot parse the response");
    }
  } else {
    Logger::udr_app().debug("NF registration procedure failed, try again ...");
  }

  if (!is_registration_success) start_nrf_registration_retry();
}

//---------------------------------------------------------------------------------------------
void udr_nrf::deregister_to_nrf() {
  nlohmann::json response_data = {};
  // Send NFs deregistration request
  std::string nrf_api_root = {};
  std::string response     = {};
  std::string method       = {"DELETE"};
  long response_code       = {0};

  get_nrf_api_root(nrf_api_root);
  std::string nrf_uri = nrf_api_root + UDR_NF_REGISTER_URL + udr_instance_id;

  Logger::udr_nrf().info("Sending NF Deregistration request");

  bool deregistration_result = false;
  deregistration_result      = udr_client_inst->curl_http_client(
      nrf_uri, method, "", response, response_code);

  if (deregistration_result and (response_code == 204)) {
    // TODO:
  } else {
    Logger::udr_nrf().info("NF Deregistration procedure failed.");
    // TODO: should we retry?
  }
}

//---------------------------------------------------------------------------------------------
void udr_nrf::start_event_nf_heartbeat(std::string& remote_uri) {
  // get current time
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  struct itimerspec its;
  its.it_value.tv_sec  = HEART_BEAT_TIMER;  // seconds
  its.it_value.tv_nsec = 0;                 // 100 * 1000 * 1000; //100ms
  const uint64_t interval =
      its.it_value.tv_sec * 1000 +
      its.it_value.tv_nsec / 1000000;  // convert sec, nsec to msec

  task_connection = m_event_sub.subscribe_task_nf_heartbeat(
      boost::bind(&udr_nrf::trigger_nf_heartbeat_procedure, this, _1), interval,
      ms + interval);
}
//---------------------------------------------------------------------------------------------
void udr_nrf::trigger_nf_heartbeat_procedure(uint64_t ms) {
  _unused(ms);
  oai::model::common::PatchItem patch_item = {};
  std::vector<oai::model::common::PatchItem> patch_items;
  //{"op":"replace","path":"/nfStatus", "value": "REGISTERED"}
  oai::model::common::PatchOperation op;
  op.setEnumValue(
      oai::model::common::PatchOperation_anyOf::ePatchOperation_anyOf::REPLACE);
  patch_item.setOp(op);
  patch_item.setPath("/nfStatus");
  patch_item.setValue("REGISTERED");
  patch_items.push_back(patch_item);
  Logger::udr_nrf().info("Sending NF Heartbeat Request");

  std::string response     = {};
  long response_code       = {0};
  std::string method       = {"PATCH"};
  nlohmann::json json_data = nlohmann::json::array();
  for (auto i : patch_items) {
    nlohmann::json item = {};
    to_json(item, i);
    json_data.push_back(item);
  }

  std::string nrf_api_root = {};
  get_nrf_api_root(nrf_api_root);
  std::string remote_uri = nrf_api_root + UDR_NF_REGISTER_URL + udr_instance_id;

  bool is_heartbeat_success = false;

  if (udr_client_inst->curl_http_client(
          remote_uri, method, json_data.dump().c_str(), response,
          response_code)) {
    if (response_code == HTTP_STATUS_CODE_200_OK or
        response_code == HTTP_STATUS_CODE_201_CREATED or
        response_code == HTTP_STATUS_CODE_204_NO_CONTENT) {
      is_heartbeat_success = true;
      // TODO: process the response
    }
  }

  if (!is_heartbeat_success) {
    Logger::udr_nrf().info(
        "NF Heartbeat procedure failed, try to register again");
    if (task_connection.connected()) task_connection.disconnect();
    register_to_nrf();
  }
}

//---------------------------------------------------------------------------------------------
void udr_nrf::start_nrf_registration_retry() {
  if (!retry_nrf_registration_task_connection.connected()) {
    // get current time
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
    const uint64_t interval =
        NRF_REGISTRATION_RETRY_TIMER * 1000;  // convert sec to msec

    Logger::udr_nrf().debug("Start NRF registration retry task");
    retry_nrf_registration_task_connection =
        m_event_sub.subscribe_task_nf_heartbeat(
            boost::bind(
                &udr_nrf::trigger_nrf_registration_retry_procedure, this, _1),
            interval, ms + interval);
  }
}

//---------------------------------------------------------------------------------------------
void udr_nrf::trigger_nrf_registration_retry_procedure(uint64_t ms) {
  _unused(ms);
  register_to_nrf();
}

//---------------------------------------------------------------------------------------------
void udr_nrf::stop_nrf_registration_retry() {
  // get current time
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  if (retry_nrf_registration_task_connection.connected()) {
    Logger::udr_nrf().debug("Stop NRF registration retry task");
    retry_nrf_registration_task_connection.disconnect();
  }
}
