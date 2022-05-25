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
   * Initialize the DB and establish the connection between UDR and the DB
   * @param void
   * @return true if successful, otherwise return false
   */
  bool initialize() override {
    /*	    Logger::udr_app().debug("initialize from database_wrapper");
                auto derived = static_cast<DerivedT*>(this);
                return derived->initialize();
                */
    return true;
  }

  bool close_connection() {
    Logger::udr_app().debug("initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    return derived->close_connection();
  }

  bool insert_authentication_subscription(
      const std::string& id, const nlohmann::json& json_data) {
    return true;
  }

  bool query_authentication_subscription(
      const std::string& id, nlohmann::json& json_data) {
    return true;
  }
  bool update_authentication_subscription(
      const std::string& id, const nlohmann::json& json_data) {
    return true;
  }
  bool delete_authentication_subscription(const std::string& id) {
    return true;
  }

  /*
   * Handle a query for AccessandMobilitySubscriptionData
   * (AccessAndMobilitySubscriptionDataDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string& ] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_am_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) {
    return true;
  }

  /*
   * Handle a request to insert AMF3GPPAccessRegistration Context
   * (AMF3GPPAccessRegistrationDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool create_amf_context_3gpp(
      const std::string& ue_id, const nlohmann::json& json_data) {
    return true;
  }

  /*
   * Handle a query for AMF3GPPAccessRegistration
   * (AMF3GPPAccessRegistrationDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @param [long code] code: HTTP response code
   * @return true if successful, otherwise return false
   */
  bool query_amf_context_3gpp(
      const std::string& ue_id, nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to insert AuthenticationStatus
   * (AuthenticationStatusDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool insert_authentication_status(
      const std::string& ue_id, const nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to delete AuthenticationStatus
   * (AuthenticationStatusDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @return true if successful, otherwise return false
   */
  bool delete_authentication_status(const std::string& ue_id) { return true; }

  /*
   * Handle a request to retrieve AuthenticationStatus
   * (AuthenticationStatusDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to retrieve SDMSubscription
   * (SDMSubscriptionDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to remove SDMSubscription (SDMSubscriptionDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @param [long code] code: HTTP response code
   * @return true if successful, otherwise return false
   */
  bool delete_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id) {
    return true;
  }

  /*
   * Handle a request to update SDMSubscription (SDMSubscriptionDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @param [SdmSubscription&] sdmSubscription: subscription information
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool update_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to create SDMSubscriptions
   * (SDMSubscriptionDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool create_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to retrieve SDMSubscriptions
   * (SDMSubscriptionDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to retrieve SessionManagementSubscription
   * (SessionManagementSubscriptionDataApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data, oai::udr::model::Snssai snssai = {},
      std::string dnn = {}) {
    return true;
  }
  /*
   * Handle a request to create SMFRegistration (SMFRegistrationDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool insert_smf_context_non_3gpp(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to delete SMFRegistration (SMFRegistrationDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @return true if successful, otherwise return false
   */
  bool delete_smf_context(
      const std::string& ue_id, const int32_t& pdu_session_id) {
    return true;
  }
  /*
   * Handle a request to retrieve SMFRegistration
   * (SMFRegistrationDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_smf_registration(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to retrieve SMFRegistrationsCollection
   * (SMFRegistrationsCollectionApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_smf_reg_list(const std::string& ue_id, nlohmann::json& json_data) {
    return true;
  }
  /*
   * Handle a request to retrieve SMFSelectionSubscription
   * (SMFSelectionSubscriptionDataDocumentApiImpl)
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  bool query_smf_select_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) {
    return true;
  }

 protected:
};
}  // namespace oai::udr::app

#endif
