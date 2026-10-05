/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_UDR_CLIENT_SEEN
#define FILE_UDR_CLIENT_SEEN

#include "nf_service.hpp"
#include "udr_event.hpp"
#include "udr_profile.hpp"

namespace oai::udr::app {

class udr_client : public oai::sba::nf_service {
 public:
  udr_client(
      const std::shared_ptr<udr_event>& ev,
      const std::shared_ptr<oai::sba::sbi_http_client>& client_inst);
  udr_client(udr_client const&)     = delete;
  ~udr_client() override            = default;
  void operator=(udr_client const&) = delete;

  void start();
  void stop();

  void generate_udr_profile();
  std::string get_nf_instance_id() const;
  bool register_to_nrf();
  bool deregister_to_nrf();

 protected:
  bool nrf_registration_enabled() const override;
  uint64_t nrf_registration_retry_seconds() const override;
  void on_registration_outcome(
      bool success, const oai::sba::sbi_http_response& resp) override;

 private:
  udr_profile udr_nf_profile;
};

}  // namespace oai::udr::app

#endif /* FILE_UDR_CLIENT_SEEN */
