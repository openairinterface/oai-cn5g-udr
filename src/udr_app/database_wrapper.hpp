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

#ifndef DATABASE_WRAPPER_HPP
#define DATABASE_WRAPPER_HPP

#include "database_wrapper_abstraction.hpp"
#include "udr.h"
#include "Snssai.h"
#include "logger.hpp"

#include <nlohmann/json.hpp>

namespace oai::udr::app {

template<class DerivedT>
class database_wrapper : public database_wrapper_abstraction {
 public:
  database_wrapper(){};
  virtual ~database_wrapper(){};

  /*std::unique_ptr<database_wrapper_abstraction> clone() const override {
          return std::make_unique<DerivedT>(static_cast<DerivedT
     const&>(*this));
      }
  */

  bool initialize() override {
    Logger::udr_app().debug("Initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    return derived->initialize();
    return true;
  }

  bool close_connection() override {
    Logger::udr_app().debug("Initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    return derived->close_connection();
  }

  bool insert_authentication_subscription(
      const std::string& id,
      const oai::udr::model::AuthenticationSubscription&
          authentication_subscription,
      nlohmann::json& json_data) override {
    return true;
  }

  bool delete_authentication_subscription(const std::string& id) override {
    return true;
  }

  bool query_authentication_subscription(
      const std::string& id, nlohmann::json& json_data) override {
    return true;
  }

  bool update_authentication_subscription(
      const std::string& id,
      const std::vector<oai::udr::model::PatchItem>& patchItem,
      nlohmann::json& json_data) override {
    return true;
  }

  bool query_am_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) override {
    return true;
  }

  bool create_amf_context_3gpp(
      const std::string& ue_id,
      oai::udr::model::Amf3GppAccessRegistration& amf3GppAccessRegistration)
      override {
    return true;
  }

  bool query_amf_context_3gpp(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  bool insert_authentication_status(
      const std::string& ue_id, const oai::udr::model::AuthEvent& authEvent,
      nlohmann::json& json_data) override {
    return true;
  }

  bool delete_authentication_status(const std::string& ue_id) override {
    return true;
  }

  bool query_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  bool query_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) override {
    return true;
  }

  bool delete_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id) override {
    return true;
  }

  bool update_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data) override {
    return true;
  }

  bool create_sdm_subscriptions(
      const std::string& ue_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data) override {
    return true;
  }

  bool query_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  bool query_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data, const oai::udr::model::Snssai& snssai = {},
      const std::string& dnn = {}) override {
    return true;
  }

  bool insert_smf_context_non_3gpp(
      const std::string& ue_id, const int32_t& pdu_session_id,
      const oai::udr::model::SmfRegistration& smfRegistration,
      nlohmann::json& json_data) override {
    return true;
  }

  bool delete_smf_context(
      const std::string& ue_id, const int32_t& pdu_session_id) override {
    return true;
  }

  bool query_smf_registration(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) override {
    return true;
  }

  bool query_smf_reg_list(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  bool query_smf_select_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) override {
    return true;
  }

 protected:
};
}  // namespace oai::udr::app

#endif
