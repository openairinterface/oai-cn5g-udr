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

#include "cassandra_db.hpp"

#include "udr_config.hpp"
#include "AuthenticationSubscription.h"
#include "logger.hpp"

using namespace oai::udr::app;
using namespace oai::udr::model;
using namespace oai::udr::config;
extern udr_config udr_cfg;

//------------------------------------------------------------------------------
cassandra_db::cassandra_db() : database_wrapper<cassandra_db>() {
  // initialize();
}

//------------------------------------------------------------------------------
cassandra_db::~cassandra_db() {}

bool cassandra_db::initialize() {
  Logger::udr_app().debug("Initialize CassandraDB");
  Logger::udr_app().debug("CassandraDB is not supported!");
  return false;
}

bool cassandra_db::close_connection() {
  return true;
}

bool cassandra_db::insert_authentication_subscription(
    const std::string& id, const nlohmann::json& json_data) {
  return true;
}

bool cassandra_db::query_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  Logger::udr_app().debug("CassandraDB is not supported!");
  return true;
}

bool cassandra_db::update_authentication_subscription(
    const std::string& id, const nlohmann::json& json_data) {
  return true;
}
bool cassandra_db::delete_authentication_subscription(const std::string& id) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::query_am_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::create_amf_context_3gpp(
    const std::string& ue_id, const nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::query_amf_context_3gpp(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::cassandra_db::insert_authentication_status(
    const std::string& ue_id, const nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::cassandra_db::delete_authentication_status(
    const std::string& ue_id) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::cassandra_db::query_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::cassandra_db::query_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::cassandra_db::delete_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::update_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::create_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::query_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::query_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data, oai::udr::model::Snssai, std::string dnn) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::insert_smf_context_non_3gpp(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::delete_smf_context(
    const std::string& ue_id, const int32_t& pdu_session_id) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::query_smf_registration(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::query_smf_reg_list(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool cassandra_db::query_smf_select_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  return true;
}
