#include "udr_config.hpp"

#include <iostream>
#include <libconfig.h++>

#include "logger.hpp"

using namespace libconfig;

namespace config {
udr_config::udr_config() {}
udr_config::~udr_config() {}

int udr_config::load(const std ::string &config_file) {
  Logger::udr_app().debug("\nLoad UDR system configuration file(%s)",
                          config_file.c_str());

  Config cfg;
  try {
    cfg.readFile(config_file.c_str());
  } catch (const FileIOException &fioex) {
    Logger::udr_app().error("I/O error while reading file %s - %s",
                            config_file.c_str(), fioex.what());
    throw;
  } catch (const ParseException &pex) {
    Logger::udr_app().error("Parse error at %s:%d - %s", pex.getFile(),
                            pex.getLine(), pex.getError());
    throw;
  }
  const Setting &root = cfg.getRoot();
  try {
    const Setting &udr_cfg = root[UDR_CONFIG_STRING_UDR_CONFIG];
  } catch (const SettingNotFoundException &nfex) {
    Logger::udr_app().error("%s : %s", nfex.what(), nfex.getPath());
    return -1;
  }
  const Setting &udr_cfg = root[UDR_CONFIG_STRING_UDR_CONFIG];
  try {
    udr_cfg.lookupValue(UDR_CONFIG_STRING_INSTANCE_ID, instance);
  } catch (const SettingNotFoundException &nfex) {
    Logger::udr_app().error("%s : %s, using defaults", nfex.what(),
                            nfex.getPath());
  }
  try {
    udr_cfg.lookupValue(UDR_CONFIG_STRING_PID_DIRECTORY, pid_dir);
  } catch (const SettingNotFoundException &nfex) {
    Logger::udr_app().error("%s : %s, using defaults", nfex.what(),
                            nfex.getPath());
  }

  try {
    const Setting &new_if_cfg = udr_cfg[UDR_CONFIG_STRING_INTERFACES];
    const Setting &nudr_cfg = new_if_cfg[UDR_CONFIG_STRING_INTERFACE_NUDR];
    load_interface(nudr_cfg, nudr);
  } catch (const SettingNotFoundException &nfex) {
    Logger::udr_app().error("%s : %s, using defaults", nfex.what(),
                            nfex.getPath());
    return -1;
  }
  try {
    const Setting &mysql_cfg = udr_cfg[UDR_CONFIG_STRING_MYSQL];
    mysql_cfg.lookupValue(UDR_CONFIG_STRING_MYSQL_SERVER, mysql.mysql_server);
    mysql_cfg.lookupValue(UDR_CONFIG_STRING_MYSQL_USER, mysql.mysql_user);
    mysql_cfg.lookupValue(UDR_CONFIG_STRING_MYSQL_PASS, mysql.mysql_pass);
    mysql_cfg.lookupValue(UDR_CONFIG_STRING_MYSQL_DB, mysql.mysql_db);
  } catch (const SettingNotFoundException &nfex) {
    Logger::udr_app().error("%s : %s, using defaults", nfex.what(),
                            nfex.getPath());
    return -1;
  }
}

int udr_config::load_interface(const libconfig::Setting &if_cfg,
                               interface_cfg_t &cfg) {
  if_cfg.lookupValue(UDR_CONFIG_STRING_INTERFACE_NAME, cfg.if_name);
  if_cfg.lookupValue(UDR_CONFIG_STRING_IPV4_ADDRESS, cfg.addr4);
  if_cfg.lookupValue(UDR_CONFIG_STRING_PORT, cfg.port);
}

void udr_config::display() {
  Logger::config().info(
      "======================    UDR   =====================");
  Logger::config().info("Configuration UDR:");
  Logger::config().info(
      "- Instance ...........................................: %d", instance);
  Logger::config().info(
      "- PID dir ............................................: %s",
      pid_dir.c_str());
  Logger::config().info(
      "- MYSQL Server Addr...................................: %s",
      mysql.mysql_server.c_str());
  Logger::config().info(
      "- MYSQL user .........................................: %s",
      mysql.mysql_user.c_str());
  Logger::config().info(
      "- MYSQL pass .........................................: %s",
      mysql.mysql_pass.c_str());
  Logger::config().info(
      "- MYSQL db ...........................................: %s",
      mysql.mysql_db.c_str());

  Logger::config().info("- Nudr Networking:");
  Logger::config().info("    iface ................: %s", nudr.if_name.c_str());
  Logger::config().info("    ip ...................: %s", nudr.addr4.c_str());
  Logger::config().info("    port .................: %d", nudr.port);
}

} // namespace config
