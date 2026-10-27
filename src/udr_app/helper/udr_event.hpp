/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_UDR_EVENT_HPP_SEEN
#define FILE_UDR_EVENT_HPP_SEEN

#include <boost/signals2.hpp>
namespace bs2 = boost::signals2;

#include "nf_event.hpp"
#include "udr.h"

namespace oai::udr::app {
class udr_event : public oai::sba::nf_event {
 public:
  udr_event() {};
  udr_event(udr_event const&)      = delete;
  void operator=(udr_event const&) = delete;

  static udr_event& get_instance() {
    static udr_event instance;
    return instance;
  }

  // class register/handle event
  friend class udr_app;
  friend class udr_client;

  /*
   * Subscribe to the task db connection reset event
   * @param [const db_connection_sig_t::slot_type &] sig
   * @param [uint64_t] period: interval between two events
   * @param [uint64_t] start:
   * @return void
   */
  // bs2::connection subscribe_task_db_connection_reset(
  //    const db_connection_sig_t::slot_type& sig, uint64_t period,
  //    uint64_t start = 0);
};
}  // namespace oai::udr::app
#endif
