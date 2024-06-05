/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under
 *one or more contributor license agreements.  See the NOTICE file distributed
 *with this work for additional information regarding copyright ownership. The
 *OpenAirInterface Software Alliance licenses this file to You under the OAI
 *Public License, Version 1.1  (the "License"); you may not use this file except
 *in compliance with the License. You may obtain a copy of the License at
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

#include "udr_config.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <iostream>
#include <libconfig.h++>

#include "common_defs.h"
#include "if.hpp"
#include "logger.hpp"
#include "string.hpp"

using namespace libconfig;

namespace oai::udr::config {

//------------------------------------------------------------------------------
udr_config::udr_config()
    : db_conf(), instance(), udr_name(), pid_dir(), nudr() {
  nudr_http2_port  = 8080;
  nudr.api_version = "v1";
  db_type          = DB_TYPE_MYSQL;
  use_http2        = false;
  register_nrf     = false;
  use_fqdn_dns     = false;
  log_level        = spdlog::level::debug;
}

//------------------------------------------------------------------------------
udr_config::~udr_config() {}

//------------------------------------------------------------------------------
void udr_config::display() {
  Logger::config().info(
      "==== OAI-CN5G %s v%s ====", PACKAGE_NAME, PACKAGE_VERSION);
  Logger::config().info(
      "======================    UDR   =====================");
  Logger::config().info("Configuration UDR:");
  Logger::config().info("- Instance ................: %d", instance);
  Logger::config().info("- PID dir .................: %s", pid_dir.c_str());
  Logger::config().info("- UDR Name ................: %s", udr_name.c_str());

  Logger::config().info("- Nudr:");
  Logger::config().info(
      "    Interface name ........: %s", nudr.if_name.c_str());
  Logger::config().info(
      "    IPv4 Addr .............: %s", inet_ntoa(nudr.addr4));
  Logger::config().info("    HTTP1 Port ............: %d", nudr.port);
  Logger::config().info("    HTTP2 port ............: %d", nudr_http2_port);
  Logger::config().info(
      "    API version ...........: %s", nudr.api_version.c_str());
  Logger::config().info("- Supported Features:");
  Logger::config().info(
      "    Register NRF ..........: %s", register_nrf ? "Yes" : "No");
  Logger::config().info(
      "    Use FQDN ..............: %s", use_fqdn_dns ? "Yes" : "No");
  Logger::config().info(
      "    Use HTTP2 .............: %s", use_http2 ? "Yes" : "No");
  Logger::config().info(
      "    Database ..............: %s", db_type_e2str[db_type].c_str());
  if (register_nrf) {
    Logger::config().info("- NRF:");
    Logger::config().info(
        "    URI root ...............: %s", nrf_addr.uri_root);
    Logger::config().info(
        "    IPv4 Addr .............: %s",
        inet_ntoa(*((struct in_addr*) &nrf_addr.ipv4_addr)));
    Logger::config().info("    Port ..................: %lu  ", nrf_addr.port);
    Logger::config().info(
        "    API version ...........: %s", nrf_addr.api_version.c_str());
    if (use_fqdn_dns)
      Logger::config().info(
          "    FQDN ..................: %s", nrf_addr.fqdn.c_str());
  }

  if (db_type == DB_TYPE_CASSANDRA) {
    Logger::config().info("- Cassandra:");
    Logger::config().info(
        "    Cassandra DB ..........: not "
        "supported!");
  } else if (db_type == DB_TYPE_UNKNOWN) {
    Logger::config().info("- Unknown DB:");
    Logger::config().info(
        "    ..........: not "
        "supported!");
  } else if (db_type == DB_TYPE_MYSQL or db_type == DB_TYPE_MONGO) {
    if (db_type == DB_TYPE_MYSQL) {
      Logger::config().info("- MySQL:");
    } else {
      Logger::config().info("- Mongo:");
    }
    Logger::config().info(
        "    Server Addr ...........: %s", db_conf.server.c_str());
    Logger::config().info("    Server Port ...........: %d", db_conf.port);
    Logger::config().info(
        "    Username ..............: %s", db_conf.user.c_str());
    Logger::config().info(
        "    Password ..............: %s", db_conf.pass.c_str());
    Logger::config().info(
        "    Database ..............: %s", db_conf.db_name.c_str());
    Logger::config().info(
        "    DB Timeout ............: %d (seconds)",
        db_conf.connection_timeout);
  }

  Logger::config().info(
      "- Log Level will be .......: %s",
      spdlog::level::to_string_view(log_level));
}

}  // namespace oai::udr::config
