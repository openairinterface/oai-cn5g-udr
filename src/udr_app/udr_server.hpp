#ifndef _UDR_SERVER_H_
#define _UDR_SERVER_H_

#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/router.h"
#ifdef __linux__
#include <vector>
#include <signal.h>
#include <unistd.h>
#endif

#include "AuthenticationSubscriptionDocumentApiImpl.h"
#include "AuthenticationStatusDocumentApiImpl.h"
#include "AccessAndMobilitySubscriptionDataDocumentApiImpl.h"
#include "SMFSelectionSubscriptionDataDocumentApiImpl.h"
#include "SessionManagementSubscriptionDataApiImpl.h"
#include "AMF3GPPAccessRegistrationDocumentApiImpl.h"
#include "SMFRegistrationDocumentApiImpl.h"
#include "SMFRegistrationsCollectionApiImpl.h"
#include "SDMSubscriptionDocumentApiImpl.h"
#include "SDMSubscriptionsCollectionApiImpl.h"

#include <mysql/mysql.h>

#define PISTACHE_SERVER_THREADS     2
#define PISTACHE_SERVER_MAX_REQUEST_SIZE 32768
#define PISTACHE_SERVER_MAX_RESPONSE_SIZE 32768

using namespace org::openapitools::server::api;

class UDRApiServer{

public:
    UDRApiServer(Pistache::Address address,MYSQL *mysql);
    void init(size_t thr = 1);
    void start();
    void shutdown();

private:

    std::shared_ptr<Pistache::Http::Endpoint> m_httpEndpoint;
    std::shared_ptr<Pistache::Rest::Router> m_router;
    std::shared_ptr<AuthenticationSubscriptionDocumentApiImpl> m_AuthenticationSubscriptionDocumentApiserver;
    std::shared_ptr<AuthenticationStatusDocumentApiImpl> m_AuthenticationStatusDocumentApiserver;
    std::shared_ptr<AccessAndMobilitySubscriptionDataDocumentApiImpl> m_AccessAndMobilitySubscriptionDataDocumentApiserver;
    std::shared_ptr<SMFSelectionSubscriptionDataDocumentApiImpl> m_SMFSelectionSubscriptionDataDocumentApiserver;
    std::shared_ptr<SessionManagementSubscriptionDataApiImpl> m_SessionManagementSubscriptionDataApiserver;
    std::shared_ptr<AMF3GPPAccessRegistrationDocumentApiImpl> m_AMF3GPPAccessRegistrationDocumentApiserver;
    std::shared_ptr<SMFRegistrationDocumentApiImpl> m_SMFRegistrationDocumentApiserver;
    std::shared_ptr<SMFRegistrationsCollectionApiImpl> m_SMFRegistrationsCollectionApiserver;
    std::shared_ptr<SDMSubscriptionDocumentApiImpl> m_SDMSubscriptionDocumentApiserver;
    std::shared_ptr<SDMSubscriptionsCollectionApiImpl> m_SDMSubscriptionsCollectionApiserver;
};

#endif