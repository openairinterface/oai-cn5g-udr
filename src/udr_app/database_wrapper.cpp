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

#include "database_wrapper.hpp"
#include "logger.hpp"
using namespace oai::udr::app;
/*
//------------------------------------------------------------------------------
template <class DerivedT> database_wrapper<DerivedT>::database_wrapper():
database_wrapper_abstraction() {
        //db_type = DB_TYPE_UNKNOWN;
}

//------------------------------------------------------------------------------
template <class DerivedT> database_wrapper<DerivedT>::~database_wrapper() {}
*/

/*
template <class DerivedT> bool database_wrapper<DerivedT>:: initialize() {

    Logger::udr_app().debug("initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    //return derived->initialize();
        return true;
}
*/
/*
template <class DerivedT> bool database_wrapper<DerivedT>:: close_connection() {
        return true;
}
*/

/*
template<class DerivedT>
bool database_wrapper<DerivedT>::insert_authentication_subscription(
    const std::string& id, const nlohmann::json& json_data) {
  Logger::udr_app().debug(
      "insert_authentication_subscription from database_wrapper");
  auto derived = static_cast<DerivedT*>(this);
  return derived->insert_authentication_subscription(id, json_data);

  return true;
}

template<class DerivedT>
bool database_wrapper<DerivedT>::query_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  return true;
}
template<class DerivedT>
bool database_wrapper<DerivedT>::update_authentication_subscription(
    const std::string& id, const nlohmann::json& json_data) {
  return true;
}
template<class DerivedT>
bool database_wrapper<DerivedT>::delete_authentication_subscription(
    const std::string& id) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::query_am_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::create_amf_context_3gpp(
    const std::string& ue_id, const nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::query_amf_context_3gpp(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::database_wrapper::insert_authentication_status(
    const std::string& ue_id, const nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::database_wrapper::delete_authentication_status(
    const std::string& ue_id) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::database_wrapper::query_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::database_wrapper::query_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::database_wrapper::delete_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::update_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::create_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::query_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::query_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data, oai::udr::model::Snssai, std::string dnn) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::insert_smf_context_non_3gpp(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::delete_smf_context(
    const std::string& ue_id, const int32_t& pdu_session_id) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::query_smf_registration(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::query_smf_reg_list(
    const std::string& ue_id, nlohmann::json& json_data) {
  return true;
}

//------------------------------------------------------------------------------
template<class DerivedT>
bool database_wrapper<DerivedT>::query_smf_select_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  return true;
}

*/
