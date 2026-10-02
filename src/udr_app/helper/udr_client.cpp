/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "udr_client.hpp"

#include <nlohmann/json.hpp>

#include "logger.hpp"
#include "udr.h"
#include "udr_config.hpp"

using namespace oai::udr::app;

extern oai::udr::config::udr_config udr_cfg;

//------------------------------------------------------------------------------
udr_client::udr_client(
    const std::shared_ptr<udr_event>& ev,
    const std::shared_ptr<oai::sba::sbi_http_client>& client_inst)
    : oai::sba::nf_service(ev, client_inst) {
  generate_udr_profile();
}

//------------------------------------------------------------------------------
void udr_client::start() {
  if (udr_cfg.register_nrf) {
    Logger::udr_app().info("NRF TASK Created ");
    register_to_nrf();
  }
}

//------------------------------------------------------------------------------
void udr_client::stop() {
  if (udr_cfg.register_nrf) deregister_to_nrf();
}

//------------------------------------------------------------------------------
void udr_client::generate_udr_profile() {
  // TODO: remove hardcoded values
  udr_nf_profile.set_nf_instance_id(nf_instance_id);
  udr_nf_profile.set_nf_instance_name("OAI-UDR");
  udr_nf_profile.set_nf_type("UDR");
  udr_nf_profile.set_nf_status("REGISTERED");
  udr_nf_profile.set_nf_heartBeat_timer(HEART_BEAT_TIMER);
  udr_nf_profile.set_nf_priority(1);
  udr_nf_profile.set_nf_capacity(100);
  udr_nf_profile.set_fqdn(udr_cfg.udr_name);
  udr_nf_profile.add_nf_ipv4_addresses(udr_cfg.nudr.addr4);

  // UDR info (hardcoded for now)
  oai::common::sbi::udr_info_t udr_info_item;
  oai::common::sbi::supi_range_info_item_t supi_ranges;
  udr_info_item.groupid = "oai-udr-testgroupid";
  udr_info_item.data_set_id.push_back("0210");
  udr_info_item.data_set_id.push_back("9876");
  supi_ranges.supi_range.start   = "208950000000031";
  supi_ranges.supi_range.pattern = "^imsi-20895[31-131]{6}$";
  supi_ranges.supi_range.start   = "208950000000131";
  udr_info_item.supi_ranges.push_back(supi_ranges);

  oai::common::sbi::identity_range_info_item_t gpsi_ranges;
  gpsi_ranges.identity_range.start   = "752740000";
  gpsi_ranges.identity_range.pattern = "^gpsi-75274[0-9]{4}$";
  gpsi_ranges.identity_range.end     = "752749999";
  udr_info_item.gpsi_ranges.push_back(gpsi_ranges);
  udr_nf_profile.set_udr_info(udr_info_item);

  udr_nf_profile.display();
}

//------------------------------------------------------------------------------
std::string udr_client::get_nf_instance_id() const {
  return nf_instance_id;
}

//------------------------------------------------------------------------------
bool udr_client::register_to_nrf() {
  nlohmann::json json_data = {};
  udr_nf_profile.to_json(json_data);
  Logger::udr_client().info("Sending NF registration request");
  return oai::sba::nf_service::register_to_nrf(udr_cfg.nrf_addr, json_data);
}

//------------------------------------------------------------------------------
bool udr_client::deregister_to_nrf() {
  Logger::udr_client().info("Sending NF deregistration request");
  return oai::sba::nf_service::deregister_to_nrf();
}

//------------------------------------------------------------------------------
bool udr_client::nrf_registration_enabled() const {
  return udr_cfg.register_nrf;
}

//------------------------------------------------------------------------------
uint64_t udr_client::nrf_registration_retry_seconds() const {
  return NRF_REGISTRATION_RETRY_TIMER;
}

//------------------------------------------------------------------------------
void udr_client::on_registration_outcome(
    bool success, const oai::sba::sbi_http_response& resp) {
  if (success) {
    Logger::udr_client().info(
        "NF registration procedure successful (status %d)", resp.status_code);
    start_event_nf_heartbeat(HEART_BEAT_TIMER);
    stop_nrf_registration_retry();
  } else {
    Logger::udr_client().info("NF registration procedure failed, try again");
    start_nrf_registration_retry();
  }
}
