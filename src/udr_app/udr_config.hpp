#ifndef _UDR_CONFIG_H_
#define _UDR_CONFIG_H_

#include <libconfig.h++>
#include <string>

#define UDR_CONFIG_STRING_UDR_CONFIG "UDR"
#define UDR_CONFIG_STRING_INSTANCE_ID "INSTANCE_ID"
#define UDR_CONFIG_STRING_PID_DIRECTORY "PID_DIRECTORY"
#define UDR_CONFIG_STRING_INTERFACES "INTERFACES"
#define UDR_CONFIG_STRING_INTERFACE_NUDR "NUDR"
#define UDR_CONFIG_STRING_INTERFACE_NAME "INTERFACE_NAME"
#define UDR_CONFIG_STRING_IPV4_ADDRESS "IPV4_ADDRESS"
#define UDR_CONFIG_STRING_PORT "PORT"

#define UDR_CONFIG_STRING_MYSQL "MYSQL"
#define UDR_CONFIG_STRING_MYSQL_SERVER "MYSQL_SERVER"
#define UDR_CONFIG_STRING_MYSQL_USER "MYSQL_USER"
#define UDR_CONFIG_STRING_MYSQL_PASS "MYSQL_PASS"
#define UDR_CONFIG_STRING_MYSQL_DB "MYSQL_DB"

using namespace libconfig;

namespace config {

typedef struct {
  std::string mysql_server;
  std::string mysql_user;
  std::string mysql_pass;
  std::string mysql_db;
} mysql_conf_t;

typedef struct interface_cfg_s {
  std::string if_name;
  std::string addr4;
  unsigned int port;
} interface_cfg_t;

class udr_config {
public:
  udr_config();
  ~udr_config();

  int load(const std::string &config_file);
  int load_interface(const Setting &if_cfg, interface_cfg_t &cfg);
  void display();

  unsigned int instance;
  std::string pid_dir;
  interface_cfg_t nudr;
  mysql_conf_t mysql;
};
} // namespace config

#endif
