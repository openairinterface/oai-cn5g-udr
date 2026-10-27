/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef _UDR_SBI_HELPER_HPP
#define _UDR_SBI_HELPER_HPP

#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

#include "Snssai.h"
#include "conversions.hpp"
#include "udr_config.hpp"
#include "sbi_helper.hpp"

using namespace oai::common::sbi;

extern oai::udr::config::udr_config udr_cfg;

namespace oai::udr::api {

class udr_sbi_helper : public sbi_helper {
 public:
  static inline const std::string UdrDataRepositoryServiceBase =
      sbi_helper::UdrDataRepositoryBase +
      udr_cfg.nudr.api_version.value_or(kDefaultSbiApiVersion);

  static inline const std::string UdrDataRepositoryServiceBaseV1 =
      sbi_helper::UdrDataRepositoryBase + "v1";

  static inline const std::string UdrDataRepositoryServiceBaseV2 =
      sbi_helper::UdrDataRepositoryBase + "v2";

  static inline const std::string UdrConfigurationServiceBase =
      sbi_helper::UdrConfBase +
      udr_cfg.nudr.api_version.value_or(kDefaultSbiApiVersion);

  // Parses an S-NSSAI query parameter: a percent-encoded JSON Snssai
  // [TS 29.571]. Returns false with the reason when it isn't valid JSON or
  // fails Snssai validation (SST 0-255, SD of 6 hexadecimal digits). The model
  // also normalizes the SD to lower case.
  static bool parse_snssai_query(
      const std::string& value, oai::_3gpp::model::Snssai& snssai,
      std::string& error) {
    try {
      nlohmann::json::parse(oai::utils::conv::url_decode(value)).get_to(snssai);
    } catch (const std::exception& e) {
      error = e.what();
      return false;
    }
    std::stringstream msg;
    if (!snssai.validate(msg)) {
      error = msg.str();
      return false;
    }
    return true;
  }
};

}  // namespace oai::udr::api

#endif
