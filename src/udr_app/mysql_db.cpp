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

#include "mysql_db.hpp"

#include "udr_config.hpp"
#include "AuthenticationSubscription.h"
#include "logger.hpp"

using namespace oai::udr::app;
using namespace oai::udr::model;
using namespace oai::udr::config;
extern udr_config udr_cfg;

//------------------------------------------------------------------------------
mysql_db::mysql_db() : database_wrapper<mysql_db>() {}

//------------------------------------------------------------------------------
mysql_db::~mysql_db() {}

bool mysql_db::initialize() {
  Logger::udr_app().debug("Initialize MySQL DB");
  if (!mysql_init(&mysql_connector)) {
    Logger::udr_app().error("Cannot initialize MySQL");
    throw std::runtime_error("Cannot initialize MySQL");
  }

  if (!mysql_real_connect(
          &mysql_connector, udr_cfg.mysql.mysql_server.c_str(),
          udr_cfg.mysql.mysql_user.c_str(), udr_cfg.mysql.mysql_pass.c_str(),
          udr_cfg.mysql.mysql_db.c_str(), 0, 0, 0)) {
    Logger::udr_app().error(
        "An error occurred while connecting to MySQL DB: %s",
        mysql_error(&mysql_connector));
    throw std::runtime_error("Cannot connect to MySQL DB");
  }
  return true;
}

bool mysql_db::close_connection() {
  mysql_close(&mysql_connector);
  return true;
}

bool mysql_db::insert_authentication_subscription(
    const std::string& id, const nlohmann::json& json_data) {
  return true;
}

bool mysql_db::query_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  Logger::udr_server().info("Query Authentication Subscription");
  MYSQL_RES* res     = nullptr;
  MYSQL_ROW row      = {};
  MYSQL_FIELD* field = nullptr;
  nlohmann::json j   = {};

  AuthenticationSubscription authentication_subscription = {};
  const std::string query =
      "SELECT * FROM AuthenticationSubscription WHERE ueid='" + id + "'";
  Logger::udr_server().info("MySQL Query: %s", query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_server().error(
        "mysql_real_query failure！ SQL Query: %s", query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_server().error(
        "mysql_store_result failure！ SQL Query: %s", query.c_str());
    return false;
  }

  row = mysql_fetch_row(res);

  if (row != nullptr) {
    for (int i = 0; field = mysql_fetch_field(res); i++) {
      Logger::udr_server().debug("Row [%d]: %s ", i, field->name);
      if (!strcmp("authenticationMethod", field->name)) {
        authentication_subscription.setAuthenticationMethod(row[i]);
      } else if (!strcmp("encPermanentKey", field->name) && row[i] != nullptr) {
        authentication_subscription.setEncPermanentKey(row[i]);
      } else if (
          !strcmp("protectionParameterId", field->name) && row[i] != nullptr) {
        authentication_subscription.setProtectionParameterId(row[i]);
      } else if (!strcmp("sequenceNumber", field->name) && row[i] != nullptr) {
        SequenceNumber sequencenumber = {};
        nlohmann::json::parse(row[i]).get_to(sequencenumber);
        authentication_subscription.setSequenceNumber(sequencenumber);
      } else if (
          !strcmp("authenticationManagementField", field->name) &&
          row[i] != nullptr) {
        authentication_subscription.setAuthenticationManagementField(row[i]);
      } else if (!strcmp("algorithmId", field->name) && row[i] != nullptr) {
        authentication_subscription.setAlgorithmId(row[i]);
      } else if (!strcmp("encOpcKey", field->name) && row[i] != nullptr) {
        authentication_subscription.setEncOpcKey(row[i]);
      } else if (!strcmp("encTopcKey", field->name) && row[i] != nullptr) {
        authentication_subscription.setEncTopcKey(row[i]);
      } else if (
          !strcmp("vectorGenerationInHss", field->name) && row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          authentication_subscription.setVectorGenerationInHss(true);
        else
          authentication_subscription.setVectorGenerationInHss(false);
      } else if (!strcmp("n5gcAuthMethod", field->name) && row[i] != nullptr) {
        authentication_subscription.setN5gcAuthMethod(row[i]);
      } else if (
          !strcmp("rgAuthenticationInd", field->name) && row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          authentication_subscription.setRgAuthenticationInd(true);
        else
          authentication_subscription.setRgAuthenticationInd(false);
      } else if (!strcmp("supi", field->name) && row[i] != nullptr) {
        authentication_subscription.setSupi(row[i]);
      }
    }

    to_json(json_data, authentication_subscription);
  } else {
    Logger::udr_server().error(
        "AuthenticationSubscription no data！ SQL Query: %s", query.c_str());
  }

  mysql_free_result(res);
  return true;
}

bool mysql_db::update_authentication_subscription(
    const std::string& id, const nlohmann::json& json_data) {
  return true;
}
bool mysql_db::delete_authentication_subscription(const std::string& id) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_am_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::create_amf_context_3gpp(
    const std::string& ue_id, const nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_amf_context_3gpp(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::insert_authentication_status(
    const std::string& ue_id, const nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::delete_authentication_status(
    const std::string& ue_id) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::query_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::query_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::delete_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::update_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::create_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data, oai::udr::model::Snssai, std::string dnn) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::insert_smf_context_non_3gpp(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::delete_smf_context(
    const std::string& ue_id, const int32_t& pdu_session_id) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_smf_registration(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_smf_reg_list(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_smf_select_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  return true;
}
