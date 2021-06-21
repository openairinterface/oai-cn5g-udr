#include "udr_server.hpp"
#include "logger.hpp"

#ifdef __linux__
static void sigHandler [[noreturn]] (int sig) {
  switch (sig) {
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
    case SIGHUP:
    default:
      //                m_httpEndpoint->shutdown();
      break;
  }
  exit(0);
}

static void setUpUnixSignals(std::vector<int> quitSignals) {
  sigset_t blocking_mask;
  sigemptyset(&blocking_mask);
  for (auto sig : quitSignals) sigaddset(&blocking_mask, sig);

  struct sigaction sa;
  sa.sa_handler = sigHandler;
  sa.sa_mask = blocking_mask;
  sa.sa_flags = 0;

  for (auto sig : quitSignals) sigaction(sig, &sa, nullptr);
}
#endif

using namespace org::openapitools::server::api;

UDRApiServer::UDRApiServer(Pistache::Address address, MYSQL *mysql)
    : m_httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(address)) {
  m_router = std::make_shared<Pistache::Rest::Router>();

  m_AuthenticationSubscriptionDocumentApiserver =
      std::make_shared<AuthenticationSubscriptionDocumentApiImpl>(m_router,
                                                                  mysql);
  m_AuthenticationStatusDocumentApiserver =
      std::make_shared<AuthenticationStatusDocumentApiImpl>(m_router, mysql);
  m_AccessAndMobilitySubscriptionDataDocumentApiserver =
      std::make_shared<AccessAndMobilitySubscriptionDataDocumentApiImpl>(
          m_router, mysql);
  m_SMFSelectionSubscriptionDataDocumentApiserver =
      std::make_shared<SMFSelectionSubscriptionDataDocumentApiImpl>(m_router,
                                                                    mysql);
  m_SessionManagementSubscriptionDataApiserver =
      std::make_shared<SessionManagementSubscriptionDataApiImpl>(m_router,
                                                                 mysql);
  m_AMF3GPPAccessRegistrationDocumentApiserver =
      std::make_shared<AMF3GPPAccessRegistrationDocumentApiImpl>(m_router,
                                                                 mysql);
  m_SMFRegistrationDocumentApiserver =
      std::make_shared<SMFRegistrationDocumentApiImpl>(m_router, mysql);
  m_SMFRegistrationsCollectionApiserver =
      std::make_shared<SMFRegistrationsCollectionApiImpl>(m_router, mysql);
  m_SDMSubscriptionDocumentApiserver =
      std::make_shared<SDMSubscriptionDocumentApiImpl>(m_router, mysql);
  m_SDMSubscriptionsCollectionApiserver =
      std::make_shared<SDMSubscriptionsCollectionApiImpl>(m_router, mysql);
}
void UDRApiServer::init(size_t thr) {
#ifdef __linux__
  std::vector<int> sigs{SIGQUIT, SIGINT, SIGTERM, SIGHUP};
  setUpUnixSignals(sigs);
#endif

  auto opts = Pistache::Http::Endpoint::options().threads(thr);
  opts.flags(Pistache::Tcp::Options::ReuseAddr);
  opts.maxRequestSize(PISTACHE_SERVER_MAX_REQUEST_SIZE);
  opts.maxResponseSize(PISTACHE_SERVER_MAX_RESPONSE_SIZE);
  m_httpEndpoint->init(opts);

  m_AuthenticationSubscriptionDocumentApiserver->init();
  m_AuthenticationStatusDocumentApiserver->init();
  m_AccessAndMobilitySubscriptionDataDocumentApiserver->init();
  m_SMFSelectionSubscriptionDataDocumentApiserver->init();
  m_SessionManagementSubscriptionDataApiserver->init();
  m_AMF3GPPAccessRegistrationDocumentApiserver->init();
  m_SMFRegistrationDocumentApiserver->init();
  m_SMFRegistrationsCollectionApiserver->init();
  m_SDMSubscriptionDocumentApiserver->init();
  m_SDMSubscriptionsCollectionApiserver->init();
  Logger::udr_server().debug("Initiate UDR server endpoints done!");
}
void UDRApiServer::start() {
  if (m_AuthenticationSubscriptionDocumentApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for AuthenticationSubscriptionDocumentApiImpl");
  if (m_AuthenticationStatusDocumentApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for AuthenticationStatusDocumentApiImpl");
  if (m_AccessAndMobilitySubscriptionDataDocumentApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for AccessAndMobilitySubscriptionDataDocumentApiImpl");
  if (m_SMFSelectionSubscriptionDataDocumentApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for SMFSelectionSubscriptionDataDocumentApiImpl");
  if (m_SessionManagementSubscriptionDataApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for SessionManagementSubscriptionDataApiImpl");
  if (m_AMF3GPPAccessRegistrationDocumentApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for AMF3GPPAccessRegistrationDocumentApiImpl");
  if (m_SMFRegistrationDocumentApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for SMFRegistrationsCollectionApiImpl");
  if (m_SMFRegistrationsCollectionApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for SDMSubscriptionDocumentApiImpl");
  if (m_SDMSubscriptionDocumentApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for SDMSubscriptionsCollectionApiImpl");
  if (m_SDMSubscriptionsCollectionApiserver != nullptr)
    Logger::udr_server().debug(
        "UDR handler for AuthenticationSubscriptionDocumentApiImpl");

  m_httpEndpoint->setHandler(m_router->handler());
  m_httpEndpoint->serve();
}
void UDRApiServer::shutdown() { m_httpEndpoint->shutdown(); }
