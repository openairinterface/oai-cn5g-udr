/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_UDR_PROFILE_HPP_SEEN
#define FILE_UDR_PROFILE_HPP_SEEN

#include <nlohmann/json.hpp>

#include "nf_profile.hpp"
#include "udr.h"

namespace oai {
namespace udr {
namespace app {

class udr_profile : public oai::sba::nf_profile {
 public:
  udr_profile();
  explicit udr_profile(const std::string& id);
  udr_profile(const udr_profile& other);
  udr_profile& operator=(const udr_profile& other);
  ~udr_profile() override = default;

  void set_udr_info(const oai::common::sbi::udr_info_t& info);
  void get_udr_info(oai::common::sbi::udr_info_t& info) const;

  void display() override;
  void to_json(nlohmann::json& data) const override;
  void from_json(const nlohmann::json& data);

  void handle_heartbeart_timeout(uint64_t ms);

 private:
  oai::common::sbi::udr_info_t udr_info;
};

}  // namespace app
}  // namespace udr
}  // namespace oai

#endif
