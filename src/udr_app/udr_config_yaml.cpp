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

#include "udr_config_yaml.hpp"

#include "conversions.hpp"
#include "logger.hpp"
#include <boost/algorithm/string.hpp>

namespace oai::config {

//------------------------------------------------------------------------------
udr_support_features::udr_support_features() {
  m_set = true;
}

//------------------------------------------------------------------------------
void udr_support_features::from_yaml(const YAML::Node& node) {
  if (node[UDR_CONFIG_DATABASE_TYPE]) {
    m_database_type.from_yaml(node[UDR_CONFIG_DATABASE_TYPE]);
  }
}

//------------------------------------------------------------------------------
std::string udr_support_features::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());

  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, UDR_CONFIG_DATABASE_TYPE_LABEL,
      inner_width, m_database_type.get_value()));
  return out;
}

//------------------------------------------------------------------------------
udr::udr(
    const std::string& name, const std::string& host, const sbi_interface& sbi)
    : nf(name, host, sbi) {}

void udr::from_yaml(const YAML::Node& node) {
  nf::from_yaml(node);

  // Load UDR specified parameter
  for (const auto& elem : node) {
    auto key = elem.first.as<std::string>();

    if (key == UDR_CONFIG_INSTANCE_ID) {
      m_instance_id.from_yaml(elem.second);
    }

    if (key == UDR_CONFIG_PID_DIRECTORY) {
      m_pid_directory.from_yaml(elem.second);
    }

    if (key == UDR_CONFIG_UDR_NAME) {
      m_udr_name.from_yaml(elem.second);
    }

    if (key == UDR_CONFIG_SUPPORT_FEATURES) {
      m_udr_support_features.from_yaml(elem.second);
    }
  }
}

//------------------------------------------------------------------------------
std::string udr::to_string(const std::string& indent) const {
  std::string out;
  std::string inner_indent = indent + indent;
  unsigned int inner_width = get_inner_width(inner_indent.length());

  out.append(indent).append(nf::to_string(indent));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, UDR_CONFIG_INSTANCE_ID_LABEL,
          inner_width, m_instance_id.get_value()));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, UDR_CONFIG_PID_DIRECTORY_LABEL,
          inner_width, m_pid_directory.get_value()));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, UDR_CONFIG_UDR_NAME_LABEL,
          inner_width, m_udr_name.get_value()));

  out.append(inner_indent)
      .append(fmt::format(
          "{} {}\n", OUTER_LIST_ELEM, UDR_CONFIG_SUPPORT_FEATURES_LABEL));
  out.append(m_udr_support_features.to_string(inner_indent + indent));

  return out;
}

//------------------------------------------------------------------------------
const uint32_t udr::get_instance_id() const {
  return m_instance_id.get_value();
}
//------------------------------------------------------------------------------
const std::string udr::get_pid_directory() const {
  return m_pid_directory.get_value();
}
//------------------------------------------------------------------------------
const std::string udr::get_udr_name() const {
  return m_udr_name.get_value();
}

//------------------------------------------------------------------------------
udr_support_features udr::get_support_features() const {
  return m_udr_support_features;
}

//------------------------------------------------------------------------------
udr_config_yaml::udr_config_yaml(
    const std::string& config_path, bool log_stdout, bool log_rot_file)
    : oai::config::config(
          config_path, oai::config::UDR_CONFIG_NAME, log_stdout, log_rot_file) {
  m_used_sbi_values    = {oai::config::UDR_CONFIG_NAME,
                       oai::config::NRF_CONFIG_NAME};
  m_used_config_values = {oai::config::LOG_LEVEL_CONFIG_NAME,
                          oai::config::REGISTER_NF_CONFIG_NAME,
                          NF_CONFIG_HTTP_NAME,
                          oai::config::NF_LIST_CONFIG_NAME,
                          oai::config::UDR_CONFIG_NAME,
                          oai::config::DATABASE_CONFIG};

  // TODO with NF_Type and switch
  // TODO: Still we need to add default NFs even we don't use this in all_in_one
  // use case
  auto m_udr = std::make_shared<udr>(
      "UDR", "oai-udr", sbi_interface("SBI", "oai-udr1", 80, "v1", "eth0"));
  add_nf("udr", m_udr);

  auto m_nrf = std::make_shared<nf>(
      "NRF", "oai-nrf", sbi_interface("SBI", "oai-nrf", 80, "v1", "eth0"));
  add_nf("nrf", m_nrf);

  update_used_nfs();
}

//------------------------------------------------------------------------------
udr_config_yaml::~udr_config_yaml() {}

void udr_config_yaml::pre_process() {
  // Process configuration information to display only the appropriate
  // information
  // TODO
}

//------------------------------------------------------------------------------
void udr_config_yaml::to_udr_config(oai::udr::config::udr_config& cfg) {
  std::shared_ptr<udr> udr_local = std::static_pointer_cast<udr>(get_local());
  cfg.instance                   = udr_local->get_instance_id();
  cfg.pid_dir                    = udr_local->get_pid_directory();
  cfg.udr_name                   = udr_local->get_udr_name();
  cfg.log_level                  = spdlog::level::from_str(log_level());
  cfg.register_nrf               = register_nrf();

  // TODO:
  // cfg.support_features.use_fqdn_dns = true;
  if (get_http_version() == 2) cfg.use_http2 = true;

  // Database
  if (get_database_config().is_set()) {
    cfg.mysql.mysql_server = get_database_config().get_host();
    cfg.mysql.mysql_user   = get_database_config().get_user();
    cfg.mysql.mysql_pass   = get_database_config().get_pass();
    cfg.mysql.mysql_db     = get_database_config().get_database_name();
    // cfg.mysql.       = get_database_config().get_random();
    cfg.mysql.connection_timeout =
        get_database_config().get_connection_timeout();
    cfg.db_type = DB_TYPE_MYSQL;  // Set to MySQL by default

    if (boost::iequals(get_database_config().get_database_type(), "mysql")) {
      cfg.db_type = DB_TYPE_MYSQL;
    } else if (boost::iequals(
                   get_database_config().get_database_type(), "cassandra")) {
      cfg.db_type = DB_TYPE_CASSANDRA;
    }
  }

  cfg.nudr.api_version = local().get_sbi().get_api_version();
  cfg.nudr_http2_port  = local().get_sbi().get_port();
  cfg.nudr.port        = local().get_sbi().get_port();
  cfg.nudr.addr4       = local().get_sbi().get_addr4();
  cfg.nudr.if_name     = local().get_sbi().get_if_name();

  if (get_nf(oai::config::NRF_CONFIG_NAME)) {
    cfg.nrf_addr.api_version = get_nf("nrf")->get_sbi().get_api_version();
    cfg.nrf_addr.uri_root    = get_nf(oai::config::NRF_CONFIG_NAME)->get_url();
  }
}
}  // namespace oai::config
