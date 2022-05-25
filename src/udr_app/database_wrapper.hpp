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

  /*
   * Initialize a connection to the DB
   * @param void
   * @return true if successful, otherwise return false
   */
  bool initialize() override {
    Logger::udr_app().debug("Initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    return derived->initialize();
    return true;
  }

  /*
   * Close the connection established to the DB
   * @param void
   * @return true if successful, otherwise return false
   */
  bool close_connection() override {
    Logger::udr_app().debug("Initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    return derived->close_connection();
  }

  /*
   * Insert a new item to the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool insert_authentication_subscription(
      const std::string& id, const nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Query an item from the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_authentication_subscription(
      const std::string& id, nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Update an item from the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool update_authentication_subscription(
      const std::string& id, const nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Delete an item from the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @return true if successful, otherwise return false
   */
  bool delete_authentication_subscription(const std::string& id) override {
    return true;
  }

  /*
   *  Query an item from the DB for AccessandMobilitySubscriptionData
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string& ] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_am_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Insert an item into DB for AMF3GPPAccessRegistration Context
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool create_amf_context_3gpp(
      const std::string& ue_id, const nlohmann::json& json_data) override {
    return true;
  }

  /*
   *  Query for an item from the DB for AMF3GPPAccessRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @param [long code] code: HTTP response code
   * @return true if successful, otherwise return false
   */
  bool query_amf_context_3gpp(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  /*
   *  Insert a new item into the DB for AuthenticationStatus
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool insert_authentication_status(
      const std::string& ue_id, const nlohmann::json& json_data) override {
    return true;
  }

  /*
   *  Delete an item from the DB for AuthenticationStatus
   * @param [const std::string&] ue_id: UE Identity
   * @return true if successful, otherwise return false
   */
  bool delete_authentication_status(const std::string& ue_id) override {
    return true;
  }

  /*
   * Query for an item from the DB for AuthenticationStatus
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Query an item from the DB for SDMSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Delete an item from the DB for SDMSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @return true if successful, otherwise return false
   */
  bool delete_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id) override {
    return true;
  }

  /*
   * Update an item from the DB for SDMSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool update_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Insert a new item into the DB for SDMSubscriptions
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool create_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Query an item from the DB for SDMSubscriptions
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Query an item from the DB for SessionManagementSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data, oai::udr::model::Snssai snssai = {},
      std::string dnn = {}) override {
    return true;
  }

  /*
   * Insert an item into the DB for SMFRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool insert_smf_context_non_3gpp(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Delete an item from the DB for SMFRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @return true if successful, otherwise return false
   */
  bool delete_smf_context(
      const std::string& ue_id, const int32_t& pdu_session_id) override {
    return true;
  }

  /*
   * Query an item from the DB SMFRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_smf_registration(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Query an item from the DB for  a request to retrieve
   * SMFRegistrationsCollection (SMFRegistrationsCollectionApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_smf_reg_list(
      const std::string& ue_id, nlohmann::json& json_data) override {
    return true;
  }

  /*
   * Query an item from the DB for SMFSelectionSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_smf_select_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) override {
    return true;
  }

 protected:
};
}  // namespace oai::udr::app

#endif
