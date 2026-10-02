/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "udr_profile.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "logger.hpp"
#include "string.hpp"

using namespace oai::udr::app;

//------------------------------------------------------------------------------
udr_profile::udr_profile() : oai::sba::nf_profile(), udr_info() {
  nf_type = "UDR";
}

//------------------------------------------------------------------------------
udr_profile::udr_profile(const std::string& id)
    : oai::sba::nf_profile(id), udr_info() {
  nf_type = "UDR";
}

//------------------------------------------------------------------------------
udr_profile::udr_profile(const udr_profile& other)
    : oai::sba::nf_profile(), udr_info() {
  *this = other;
}

//------------------------------------------------------------------------------
udr_profile& udr_profile::operator=(const udr_profile& other) {
  if (this == &other) return *this;

  nf_instance_id   = other.nf_instance_id;
  nf_instance_name = other.nf_instance_name;
  nf_type          = other.nf_type;
  nf_status        = other.nf_status;
  heartBeat_timer  = other.heartBeat_timer;
  plmn_list        = other.plmn_list;
  snssais          = other.snssais;
  fqdn             = other.fqdn;
  ipv4_addresses   = other.ipv4_addresses;
  ipv6_addresses   = other.ipv6_addresses;
  priority         = other.priority;
  capacity         = other.capacity;
  json_data        = other.json_data;
  nf_services      = other.nf_services;
  custom_info      = other.custom_info;
  is_updated       = other.is_updated;
  udr_info         = other.udr_info;
  return *this;
}

//------------------------------------------------------------------------------
void udr_profile::set_udr_info(const oai::common::sbi::udr_info_t& s) {
  udr_info = s;
}

//------------------------------------------------------------------------------
void udr_profile::get_udr_info(oai::common::sbi::udr_info_t& s) const {
  s = udr_info;
}

//------------------------------------------------------------------------------
void udr_profile::display() {
  oai::sba::nf_profile::display();

  Logger::udr_app().debug("\tUDR Info");
  Logger::udr_app().debug("\t\tGroupId: %s", udr_info.groupid.c_str());
  for (auto supi : udr_info.supi_ranges) {
    Logger::udr_app().debug(
        "\t\t SupiRanges: Start - %s, End - %s, Pattern - %s",
        supi.supi_range.start.c_str(), supi.supi_range.end.c_str(),
        supi.supi_range.pattern.c_str());
  }
  for (auto gpsi : udr_info.gpsi_ranges) {
    Logger::udr_app().debug(
        "\t\t GpsiRanges: Start - %s, End - %s, Pattern - %s",
        gpsi.identity_range.start.c_str(), gpsi.identity_range.end.c_str(),
        gpsi.identity_range.pattern.c_str());
  }
  for (auto ext_grp_id : udr_info.ext_grp_id_ranges) {
    Logger::udr_app().debug(
        "\t\t externalGroupIdentifiersRanges: Start - %s, "
        "End - %s, Pattern - %s",
        ext_grp_id.identity_range.start.c_str(),
        ext_grp_id.identity_range.end.c_str(),
        ext_grp_id.identity_range.pattern.c_str());
  }
  for (auto data_set_Id : udr_info.data_set_id) {
    Logger::udr_app().debug("\t\t Data Set Id: %s", data_set_Id.c_str());
  }
}

//------------------------------------------------------------------------------
void udr_profile::to_json(nlohmann::json& data) const {
  oai::sba::nf_profile::to_json(data);

  // Preserve the UDR registration payload produced before using the common
  // NF profile implementation.
  data.erase("json_data");
  data.erase("nfServices");
  if (snssais.empty()) data["sNssais"] = nlohmann::json::array();
  data["fqdn"] = fqdn;

  // UDR Info
  data["udrInfo"]["groupId"]                        = udr_info.groupid;
  data["udrInfo"]["supiRanges"]                     = nlohmann::json::array();
  data["udrInfo"]["gpsiRanges"]                     = nlohmann::json::array();
  data["udrInfo"]["externalGroupIdentifiersRanges"] = nlohmann::json::array();
  data["udrInfo"]["DataSetId"]                      = nlohmann::json::array();
  for (auto supi : udr_info.supi_ranges) {
    nlohmann::json tmp = {};
    tmp["start"]       = supi.supi_range.start;
    tmp["end"]         = supi.supi_range.end;
    tmp["pattern"]     = supi.supi_range.pattern;
    data["udrInfo"]["supiRanges"].push_back(tmp);
  }
  for (auto gpsi : udr_info.gpsi_ranges) {
    nlohmann::json tmp = {};
    tmp["start"]       = gpsi.identity_range.start;
    tmp["end"]         = gpsi.identity_range.end;
    tmp["pattern"]     = gpsi.identity_range.pattern;
    data["udrInfo"]["gpsiRanges"].push_back(tmp);
  }
  for (auto ext_grp_id : udr_info.ext_grp_id_ranges) {
    nlohmann::json tmp = {};
    tmp["start"]       = ext_grp_id.identity_range.start;
    tmp["end"]         = ext_grp_id.identity_range.end;
    tmp["pattern"]     = ext_grp_id.identity_range.pattern;
    data["udrInfo"]["externalGroupIdentifiersRanges"].push_back(tmp);
  }
  for (auto data_set_Id : udr_info.data_set_id) {
    std::string tmp = data_set_Id;
    data["udrInfo"]["DataSetId"].push_back(data_set_Id);
  }

  Logger::udr_app().debug("udr profile to JSON:\n %s", data.dump().c_str());
}

//------------------------------------------------------------------------------
void udr_profile::from_json(const nlohmann::json& data) {
  snssais.clear();
  ipv4_addresses.clear();
  udr_info = {};

  if (data.find("nfInstanceId") != data.end()) {
    nf_instance_id = data["nfInstanceId"].get<std::string>();
  }

  if (data.find("nfInstanceName") != data.end()) {
    nf_instance_name = data["nfInstanceName"].get<std::string>();
  }

  if (data.find("nfType") != data.end()) {
    nf_type = data["nfType"].get<std::string>();
  }

  if (data.find("nfStatus") != data.end()) {
    nf_status = data["nfStatus"].get<std::string>();
  }

  if (data.find("heartBeatTimer") != data.end()) {
    heartBeat_timer = data["heartBeatTimer"].get<int>();
  }
  // sNssais
  if (data.find("sNssais") != data.end()) {
    for (auto it : data["sNssais"]) {
      snssai_t s = {};
      s.sst      = it["sst"].get<int>();
      s.sd       = it["sd"].get<std::string>();
      snssais.push_back(s);
    }
  }

  if (data.find("fqdn") != data.end()) {
    fqdn = data["fqdn"].get<std::string>();
  }

  if (data.find("ipv4Addresses") != data.end()) {
    for (const auto& item : data["ipv4Addresses"]) {
      struct in_addr address = {};
      auto value             = item.get<std::string>();
      if (inet_pton(AF_INET, oai::utils::trim(value).c_str(), &address) != 1) {
        Logger::udr_app().warn(
            "Address conversion: Bad value %s", oai::utils::trim(value));
        continue;
      }
      ipv4_addresses.push_back(address);
    }
  }

  if (data.find("priority") != data.end()) {
    priority = data["priority"].get<int>();
  }

  if (data.find("capacity") != data.end()) {
    capacity = data["capacity"].get<int>();
  }

  // UDR info
  if (data.find("udrInfo") != data.end()) {
    nlohmann::json info = data["udrInfo"];
    if (info.find("groupId") != info.end()) {
      udr_info.groupid = info["groupId"].get<std::string>();
    }
    if (info.find("DataSetId") != info.end()) {
      nlohmann::json data_set_id_list = data["udrInfo"]["DataSetId"];
      for (auto d : data_set_id_list) {
        udr_info.data_set_id.push_back(d);
      }
    }
    if (info.find("supiRanges") != info.end()) {
      nlohmann::json supi_ranges = data["udrInfo"]["supiRanges"];
      for (auto d : supi_ranges) {
        oai::common::sbi::supi_range_info_item_t supi;
        supi.supi_range.start   = d["start"];
        supi.supi_range.end     = d["end"];
        supi.supi_range.pattern = d["pattern"];
        udr_info.supi_ranges.push_back(supi);
      }
    }
    if (info.find("gpsiRanges") != info.end()) {
      nlohmann::json gpsi_ranges = data["udrInfo"]["gpsiRanges"];
      for (auto d : gpsi_ranges) {
        oai::common::sbi::identity_range_info_item_t gpsi;
        gpsi.identity_range.start   = d["start"];
        gpsi.identity_range.end     = d["end"];
        gpsi.identity_range.pattern = d["pattern"];
        udr_info.gpsi_ranges.push_back(gpsi);
      }
    }
    if (info.find("externalGroupIdentifiersRanges") != info.end()) {
      nlohmann::json ext_grp_id_ranges =
          data["udrInfo"]["externalGroupIdentifiersRanges"];
      for (auto d : ext_grp_id_ranges) {
        oai::common::sbi::identity_range_info_item_t ext_grp_id;
        ext_grp_id.identity_range.start   = d["start"];
        ext_grp_id.identity_range.end     = d["end"];
        ext_grp_id.identity_range.pattern = d["pattern"];
        udr_info.ext_grp_id_ranges.push_back(ext_grp_id);
      }
    }
  }
  display();
}

//------------------------------------------------------------------------------
void udr_profile::handle_heartbeart_timeout(uint64_t ms) {
  Logger::udr_app().info(
      "Handle heartbeart timeout profile %s, time %d", nf_instance_id.c_str(),
      ms);
  set_nf_status("SUSPENDED");
}
