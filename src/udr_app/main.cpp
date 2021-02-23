/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 *file except in compliance with the License. You may obtain a copy of the
 *License at
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

/*! file
brief
author  Jian Yang,Fengjiao He,Hongxin Wang
date 2020
email: contact@openairinterface.org
*/

#include "logger.hpp"
#include "options.hpp"
#include "udr_config.hpp"
#include "udr_server.hpp"

using namespace config;

udr_config udr_cfg;

int main(int argc, char **argv) {
  if (!Options::parse(argc, argv)) {
    std::cout << "Options::parse() failed" << std::endl;
    return 1;
  }

  Logger::init("UDR", Options::getlogStdout(), Options::getlogRotFilelog());
  Logger::udr_app().startup("Options parsed!");

  // add config file
  udr_cfg.load(Options::getlibconfigConfig());
  udr_cfg.display();

  Logger::udr_app().debug("Initiating UDR server endpoints");
  // Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));
  Pistache::Address addr(udr_cfg.nudr.addr4, Pistache::Port(udr_cfg.nudr.port));

  MYSQL mysql;
  mysql_init(&mysql);
  if (!mysql_real_connect(&mysql, udr_cfg.mysql.mysql_server.c_str(),
                          udr_cfg.mysql.mysql_user.c_str(),
                          udr_cfg.mysql.mysql_pass.c_str(),
                          udr_cfg.mysql.mysql_db.c_str(), 0, 0, 0)) {
    Logger::udr_app().error("An error occurred while connecting to db: %s",
                            mysql_error(&mysql));
    return 0;
  }

  UDRApiServer udrApiServer(addr, &mysql);
  udrApiServer.init(PISTACHE_SERVER_THREADS);
  std::thread udr_api_manager(&UDRApiServer::start, udrApiServer);
  Logger::udr_app().debug("Initiating Done!");

  pause();
  mysql_close(&mysql);
  return 0;
}
