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

/*! \file db_connection_manager.hpp
 \author  Tien-Thinh NGUYEN
 \company Eurecom
 \date 2022
 \email:
 */

#ifndef FILE_DB_CONNECTION_MANAGER_SEEN
#define FILE_DB_CONNECTION_MANAGER_SEEN

#include "database_wrapper_abstraction.hpp"
#include "udr_event.hpp"

namespace oai {
namespace udr {
namespace app {

class db_connection_manager {
 private:
 public:
  db_connection_manager(udr_event& ev);
  db_connection_manager(db_connection_manager const&) = delete;
  void operator=(db_connection_manager const&) = delete;

  /*
   * Start event connection initialization procedure
   * @param [void]
   * @return void
   */
  void start_event_connection_initialization(
      std::shared_ptr<database_wrapper_abstraction>& db_connector);

  /*
   * Trigger connection reset procedure  (NF Heartbeat)
   * @param [void]
   * @return void
   */
  void trigger_connection_reset_procedure(
      uint64_t ms, std::shared_ptr<database_wrapper_abstraction>& db_connector);

 private:
  udr_event& m_event_sub;
  bs2::connection task_connection;
  bs2::connection db_connection;
};
}  // namespace app
}  // namespace udr
}  // namespace oai
#endif /* FILE_DB_CONNECTION_MANAGER_SEEN */
