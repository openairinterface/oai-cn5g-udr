#include "options.hpp"
#include "logger.hpp"
#include "udr_config.hpp"
#include "udr_server.hpp"

using namespace config;

udr_config udr_cfg;

int main(int argc, char **argv) {

    if  (!Options::parse(argc, argv))
    {
        std::cout<<"Options::parse() failed"<<std::endl;
        return 1;
    }

    Logger::init( "UDR" , Options::getlogStdout() , Options::getlogRotFilelog());
    Logger::udr_app().startup("Options parsed!");

    //add config file
    udr_cfg.load(Options::getlibconfigConfig());
    udr_cfg.display();

    Logger::udr_app().debug("Initiating UDR server endpoints");
    //Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));
    Pistache::Address addr(udr_cfg.nudr.addr4 , Pistache::Port(udr_cfg.nudr.port));

    MYSQL mysql;
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, udr_cfg.mysql.mysql_server.c_str(),udr_cfg.mysql.mysql_user.c_str(), udr_cfg.mysql.mysql_pass.c_str(), udr_cfg.mysql.mysql_db.c_str(), 0, 0, 0))
    {
        Logger::udr_app().error("An error occurred while connecting to db: %s",mysql_error(&mysql));
        return 0;
    }

    UDRApiServer udrApiServer(addr,&mysql);
    udrApiServer.init(PISTACHE_SERVER_THREADS);
    std::thread udr_api_manager(&UDRApiServer::start, udrApiServer);
    Logger::udr_app().debug("Initiating Done!");

    pause();
    mysql_close(&mysql);
    return 0;
}

