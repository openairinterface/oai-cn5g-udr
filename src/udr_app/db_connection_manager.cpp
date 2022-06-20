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

/*! \file db_connection_manager.cpp
 \brief
 \author  Tien-Thinh NGUYEN
 \company Eurecom
 \date 2022
 \email:
 */

#include "db_connection_manager.hpp"

#include "logger.hpp"
#include "udr.h"
#include "udr_app.hpp"

using namespace oai::udr::config;
using namespace oai::udr::app;

using json = nlohmann::json;

extern udr_config udr_cfg;

//------------------------------------------------------------------------------
db_connection_manager::db_connection_manager(udr_event& ev) : m_event_sub(ev) {}

//---------------------------------------------------------------------------------------------
void db_connection_manager::start_event_connection_initialization(
    std::shared_ptr<database_wrapper_abstraction>& db_connector) {
  // get current time
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  struct itimerspec its;
  its.it_value.tv_sec  = DB_CONNECTION_TIMER;  // seconds
  its.it_value.tv_nsec = 0;                    // 100 * 1000 * 1000; //100ms
  const uint64_t interval =
      its.it_value.tv_sec * 1000 +
      its.it_value.tv_nsec / 1000000;  // convert sec, nsec to msec

  db_connection = m_event_sub.subscribe_task_db_connection_reset(
      boost::bind(&udr_nrf::trigger_connection_reset_procedure, this, _1, _2),
      db_connector, interval, ms + interval);
}
//---------------------------------------------------------------------------------------------
void db_connection_manager::trigger_connection_reset_procedure(
    std::shared_ptr<database_wrapper_abstraction>& db_connector, uint64_t ms) {
  _unused(ms);

  // Close the current connection
  db_connector.close_connection();
  // and start a new one
  db_connector.initialize();
}
