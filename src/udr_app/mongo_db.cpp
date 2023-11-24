#include "mongo_db.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <spdlog/spdlog.h>

#include "AccessAndMobilitySubscriptionData.h"
#include "AuthenticationSubscription.h"
#include "ProblemDetails.h"
#include "SdmSubscription.h"
#include "logger.hpp"
#include "udr_config.hpp"

#include <boost/algorithm/string.hpp>

using namespace oai::udr::app;
using namespace oai::udr::model;
using namespace oai::udr::config;
using namespace oai::model::common;

extern udr_config udr_cfg;

mongo_db::mongo_db(udr_event& ev)
    : database_wrapper<mongo_db>(), m_event_sub(ev), m_db_connection_status() {
  is_db_connection_active = false;
  start_event_connection_handling();
}

//------------------------------------------------------------------------------
mongo_db::~mongo_db() {
  if (db_connection_event.connected()) db_connection_event.disconnect();
  close_connection();
}

//------------------------------------------------------------------------------
bool mongo_db::initialize() {
  return true;
}

//------------------------------------------------------------------------------
bool mongo_db::connect(uint32_t num_retries) {
  Logger::udr_db().debug("Connecting to MongoDB");

  int i = 0;
  while (i < num_retries) {
    try {
      // Try to connect to MongoDB

      mongo_client = mongocxx::client{mongocxx::uri{
          "mongodb://" + udr_cfg.db_conf.user + ":" + udr_cfg.db_conf.pass +
          "@" + udr_cfg.db_conf.server + ":" +
          std::to_string(udr_cfg.db_conf.port)}};

      // Check if connection to MongoDB works
      bsoncxx::builder::stream::document ping;
      ping << "ping" << 1;
      auto db = mongo_client[udr_cfg.db_conf.db_name.c_str()];
      db.run_command(ping.view());

      Logger::udr_db().info("Connected to MongoDB");
      set_db_connection_status(true);
      Logger::udr_db().info("Mongo client created successfully");

      return true;
    } catch (const mongocxx::exception& ex) {
      std::cout << "Mongo client URI: " << mongo_client.uri().to_string()
                << std::endl;
      Logger::udr_db().error(
          "An error occurred when connecting to MongoDB (%s), retry ...",
          ex.what());
      i++;
      set_db_connection_status(false);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  Logger::udr_db().error(
      "Failed to connect to MongoDB after %d retries", num_retries);
  return false;
}

//------------------------------------------------------------------------------
bool mongo_db::close_connection() {
  Logger::udr_db().debug("Close the connection with MongoDB");
  // No need to explicitly close the MongoDB connection
  set_db_connection_status(false);
  return true;
}

//------------------------------------------------------------------------------
void mongo_db::set_db_connection_status(bool status) {
  std::unique_lock lock(m_db_connection_status);
  is_db_connection_active = status;
}

//------------------------------------------------------------------------------
bool mongo_db::get_db_connection_status() const {
  std::shared_lock lock(m_db_connection_status);
  return is_db_connection_active;
}

//---------------------------------------------------------------------------------------------
void mongo_db::start_event_connection_handling() {
  // create a time point representing the current time
  auto now = std::chrono::system_clock::now();

  // convert the time point to milliseconds
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

  struct itimerspec its;
  its.it_value.tv_sec  = udr_cfg.db_conf.connection_timeout;  // seconds
  its.it_value.tv_nsec = 0;  // 100 * 1000 * 1000; //100ms
  const uint64_t interval =
      its.it_value.tv_sec * 1000 +
      its.it_value.tv_nsec / 1000000;  // convert sec, nsec to msec

  db_connection_event = m_event_sub.subscribe_task_nf_heartbeat(
      std::bind(
          &mongo_db::trigger_connection_handling_procedure, this,
          std::placeholders::_1),
      interval, ms + interval);
}
//---------------------------------------------------------------------------------------------
void mongo_db::trigger_connection_handling_procedure(uint64_t ms) {
  _unused(ms);
  std::time_t current_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  Logger::udr_db().debug(
      "DB Connection handling, current time: %s", std::ctime(&current_time));

  try {
    if (mongo_client) {
      bsoncxx::builder::stream::document ping;
      ping << "ping" << 1;
      auto db = mongo_client[udr_cfg.db_conf.db_name.c_str()];
      db.run_command(ping.view());
      return;
    }
  } catch (const std::exception& e) {
    set_db_connection_status(false);
    Logger::udr_db().warn(
        "Could not establish the connection to the DB, reason: %s", e.what());
  }

  // If couldn't connect to the DB
  // Reset the connection and try again
  close_connection();
  initialize();
  if (!connect(MAX_CONNECTION_RETRY))
    Logger::udr_db().warn("Could not establish the connection to the DB");
}

//------------------------------------------------------------------------------
bool mongo_db::insert_authentication_subscription(
    const std::string& id,
    const oai::udr::model::AuthenticationSubscription& auth_subscription,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    return false;
  }
  // Select the appropriate database and collection
  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["AuthenticationSubscription"];
  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << id;

  mongocxx::options::find opts{};
  opts.limit(1);

  try {
    auto cursor = coll.find_one(filter_builder.view(), opts);

    if (cursor) {
      Logger::udr_db().error("AuthenticationSubscription existed!");
      // Existed
      return false;
    }

    to_json(json_data, auth_subscription);

    json_data["ueid"] = id;
    coll.insert_one(bsoncxx::from_json(json_data.dump()));

    Logger::udr_db().debug(
        "AuthenticationSubscription POST: %s", json_data.dump().c_str());

    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while insert authentication subscription in MongoDB: %s",
        e.what());
    return false;
  }
}

bool mongo_db::delete_authentication_subscription(const std::string& id) {
  // Select the appropriate database and collection
  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["AuthenticationSubscription"];

  try {
    // Perform the delete operation
    auto result = coll.delete_one(bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("ueid", id)));

    if (!result) {
      Logger::udr_db().error("Failed to delete document from MongoDB");
      return false;
    }

    if (result->deleted_count() == 0) {
      Logger::udr_db().error("No document found with the given ID");
      return false;
    }

    Logger::udr_db().debug(
        "Deleted %i document(s) from MongoDB", result->deleted_count());

    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while delete authentication subscription from MongoDB: %s",
        e.what());
    return false;
  }
}

bool mongo_db::query_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  // Check the connection with DB first

  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    return false;
  }

  Logger::udr_db().info("Query Authentication Subscription");

  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AuthenticationSubscription"];

  // Build the query
  auto query = bsoncxx::builder::stream::document{}
               << "ueid" << id << bsoncxx::builder::stream::finalize;

  try {
    // Execute the query and get the result
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        coll.find_one(query.view());

    if (result) {
      bsoncxx::document::view view = result->view();

      AuthenticationSubscription authentication_subscription = {};
      from_json(
          nlohmann::json::parse(bsoncxx::to_json(view)),
          authentication_subscription);

      to_json(json_data, authentication_subscription);

      return true;
    } else {
      Logger::udr_db().error(
          "AuthenticationSubscription no data！ Query filter: %s",
          bsoncxx::to_json(query.view()).c_str());

      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query authentication subscription from MongoDB: %s",
        e.what());
    return false;
  }
}

//------------------------------------------------------------------------------

bool mongo_db::update_authentication_subscription(
    const std::string& ue_id,
    const std::vector<oai::model::common::PatchItem>& patchItem,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AuthenticationSubscription"];

  auto filter = bsoncxx::builder::stream::document{}
                << "ueid" << ue_id << bsoncxx::builder::stream::finalize;

  try {
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        coll.find_one(filter.view());

    if (result) {
      bsoncxx::document::view view = result->view();

      for (const auto& item : patchItem) {
        if (item.getOp().getEnumValue() ==
                PatchOperation_anyOf::ePatchOperation_anyOf::REPLACE &&
            item.valueIsSet()) {
          bsoncxx::document::value sequenceNumberValue =
              bsoncxx::from_json(item.getValue());
          bsoncxx::document::view sequenceNumberView =
              sequenceNumberValue.view();

          bsoncxx::builder::stream::document updateBuilder{};
          updateBuilder << "$set" << bsoncxx::builder::stream::open_document;
          updateBuilder << "sequenceNumber" << sequenceNumberView;
          updateBuilder << bsoncxx::builder::stream::close_document;
          auto update = updateBuilder << bsoncxx::builder::stream::finalize;

          auto updateResult = coll.update_one(filter.view(), update.view());
          if (!updateResult) {
            Logger::udr_db().error(
                "Failed to update AuthenticationSubscription");
            return false;
          }
        }

        nlohmann::json tmp_j;
        to_json(tmp_j, item);
        json_data += tmp_j;
      }

      Logger::udr_db().info(
          "AuthenticationSubscription PATCH: %s", json_data.dump().c_str());
      bool query_result = query_authentication_subscription(ue_id, json_data);
      if (!query_result) {
        Logger::udr_db().error(
            "Failed to retrieve updated authentication subscription data");
        return false;
      }

      return true;
    } else {
      Logger::udr_db().error(
          "AuthenticationSubscription not found for ueid: %s", ue_id.c_str());
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while update authentication subscription in MongoDB: %s",
        e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_am_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  // Establish MongoDB connection
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    return false;
  }

  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AccessAndMobilitySubscriptionData"];

  // Construct the MongoDB query
  bsoncxx::builder::stream::document query_builder{};
  query_builder << "ueid" << ue_id << "servingPlmnid" << serving_plmn_id;
  auto query = query_builder.view();

  try {
    auto result = coll.find_one(query);
    if (result) {
      oai::udr::model::AccessAndMobilitySubscriptionData subscription_data = {};

      const bsoncxx::document::view row = result.value().view();

      from_json(
          nlohmann::json::parse(bsoncxx::to_json(row)), subscription_data);
      to_json(json_data, subscription_data);

      Logger::udr_db().debug(
          "AccessAndMobilitySubscriptionData Get: %s",
          json_data.dump().c_str());
    } else {
      // Handle query failure
      Logger::udr_db().error("Failed to query AM Data from MongoDB");
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query AM Data from MongoDB: %s", e.what());
    return false;
  } catch (const std::exception& e) {
    Logger::udr_db().error(
        "Exception while query AM Data from MongoDB: %s", e.what());
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
bool mongo_db::create_amf_context_3gpp(
    const std::string& ue_id,
    Amf3GppAccessRegistration& amf3GppAccessRegistration) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  // Select the appropriate database and collection
  mongocxx::database db = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto collection       = db["Amf3GppAccessRegistration"];
  try {
    auto document = collection.find_one(filter_builder.view());

    nlohmann::json json_data;
    to_json(json_data, amf3GppAccessRegistration);

    bsoncxx::document::value update_doc = bsoncxx::from_json(json_data.dump());

    if (document) {
      auto result = collection.update_one(
          filter_builder.view(),
          bsoncxx::builder::basic::make_document(
              bsoncxx::builder::basic::kvp("$set", update_doc.view())));
      if (!result) {
        Logger::udr_db().error(
            "MongoDB update failure! Query: %s",
            bsoncxx::to_json(update_doc.view()).c_str());
        return false;
      }

      Logger::udr_db().debug(
          "MongoDB Document %s is updated successfully with following values: "
          "%s",
          bsoncxx::to_json(filter_builder.view()),
          bsoncxx::to_json(update_doc.view()));
      return true;
    } else {
      Logger::udr_db().error(
          "MongoDB Document %s not found! Query could not be submitted: %s",
          bsoncxx::to_json(filter_builder.view()),
          bsoncxx::to_json(update_doc.view()));
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while create AMF context in MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_amf_context_3gpp(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  auto db         = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto collection = db["Amf3GppAccessRegistration"];
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;
  auto filter = filter_builder.view();

  try {
    auto cursor = collection.find_one(filter);

    if (cursor) {
      oai::udr::model::Amf3GppAccessRegistration amf3gppaccessregistration = {};
      const bsoncxx::document::view row = cursor.value().view();

      from_json(
          nlohmann::json::parse(bsoncxx::to_json(row)),
          amf3gppaccessregistration);
      to_json(json_data, amf3gppaccessregistration);

      return true;
    } else {
      Logger::udr_db().info(
          "AMF 3GPP Access Registration for UE ID %s not found in MongoDB",
          ue_id.c_str());
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query AMF context from MongoDB: %s", e.what());
    return false;
  } catch (const std::exception& e) {
    Logger::udr_db().error(
        "Exception while query sm data from MongoDB: %s", e.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool mongo_db::insert_authentication_status(
    const std::string& ue_id, const oai::udr::model::AuthEvent& authEvent,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  // Get the database and collection
  auto db         = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto collection = db["AuthenticationStatus"];
  auto filter     = bsoncxx::builder::stream::document{}
                << "ueid" << ue_id << bsoncxx::builder::stream::finalize;

  try {
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        collection.find_one(filter.view());

    nlohmann::json tmp_json;
    to_json(tmp_json, authEvent);
    tmp_json["ueid"] = ue_id;
    auto document    = bsoncxx::from_json(tmp_json.dump());

    if (result) {
      auto updateResult = collection.update_one(
          filter.view(),
          bsoncxx::builder::basic::make_document(
              bsoncxx::builder::basic::kvp("$set", document.view())));
      if (!updateResult) {
        Logger::udr_db().error("Failed to update AuthenticationStatus");
        return false;
      }
    } else {
      auto insertResult = collection.insert_one(document.view());
      if (!insertResult) {
        Logger::udr_db().error("Failed to insert AuthenticationStatus");
        return false;
      }
    }

    Logger::udr_db().info(
        "AuthenticationStatus PUT: %s", tmp_json.dump().c_str());

    return true;

  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while insert authentication status in MongoDB: %s",
        e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::delete_authentication_status(const std::string& ue_id) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["AuthenticationStatus"];

  bsoncxx::builder::stream::document filter_builder;
  filter_builder
      << "ueid"
      << ue_id;  // Create a filter document for matching the "ueid" field

  try {
    auto result = coll.delete_one(
        filter_builder.view());  // Delete the document matching the filter
    if (result) {
      if (result->deleted_count() > 0) {
        Logger::udr_db().debug("AuthenticationStatus DELETE - successful");

        return true;
      } else {
        Logger::udr_db().info(
            "No document found with ueid '%s'", ue_id.c_str());
        return false;
      }
    } else {
      Logger::udr_db().error("Failed to delete document in MongoDB");
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while delete authentication status from MongoDB: %s",
        e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    return false;
  }

  Logger::udr_db().info("Query Authentication Status");

  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AuthenticationStatus"];

  // Build the query
  auto query = bsoncxx::builder::stream::document{}
               << "ueid" << ue_id << bsoncxx::builder::stream::finalize;

  try {
    // Execute the query and get the result
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        coll.find_one(query.view());

    // Check if the result is not empty
    if (result) {
      // Convert the result object to a JSON string
      std::string result_str = bsoncxx::to_json(result->view());

      // Log the result using the Logger::udr_db().info() method
      Logger::udr_db().info("MongoDB Result: %s", result_str.c_str());

      AuthEvent authentication_status = {};
      from_json(nlohmann::json::parse(result_str), authentication_status);
      to_json(json_data, authentication_status);
      return true;
    } else {
      Logger::udr_db().error(
          "AuthenticationStatus no data！ Query filter: %s",
          bsoncxx::to_json(query.view()).c_str());
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query authentication data from MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  // Select the appropriate database and collection
  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["SdmSubscriptions"];
  bsoncxx::builder::stream::document filter;

  // TODO Error Handling
  filter << "ueid" << ue_id << "subsId" << std::stoi(subs_id);

  try {
    auto result = coll.find_one(filter.view());
    Logger::udr_db().debug(
        "SdmSubscription Query Filter: %s", bsoncxx::to_json(filter.view()));
    if (result) {
      auto str_result = bsoncxx::to_json(result->view());
      oai::udr::model::SdmSubscription sdmSubscriptions = {};

      from_json(nlohmann::json::parse(str_result), sdmSubscriptions);

      // Convert SdmSubscriptions to json
      to_json(json_data, sdmSubscriptions);

      Logger::udr_db().debug(
          "SdmSubscription Query Result: %s", json_data.dump());

      Logger::udr_db().debug(
          "Successfully queried SDM subscription from MongoDB: UE ID=%s, "
          "Subscription ID=%s",
          ue_id, subs_id);
      return true;
    } else {
      Logger::udr_db().info(
          "Failed to query SDM subscription from MongoDB: UE ID=%s, "
          "Subscription "
          "ID=%s",
          ue_id, subs_id);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query sdm subscription from MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::delete_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    return false;
  }

  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];
  bsoncxx::builder::stream::document filter;

  // TODO Error Handling
  filter << "ueid" << ue_id << "subsId" << stoi(subs_id);

  try {
    auto result = coll.delete_one(filter.view());

    if (!result || result->deleted_count() == 0) {
      Logger::udr_db().error(
          "Failed to delete document from MongoDB: ueid='%s' subsId='%s'",
          ue_id.c_str(), subs_id.c_str());
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while delete sdm subscription from MongoDB: %s", e.what());
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
bool mongo_db::update_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    oai::udr::model::SdmSubscription& sdmSubscription,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];

  // Prepare filter for update
  bsoncxx::builder::stream::document filter;
  filter << "ueid" << ue_id << "subsId" << subs_id;

  nlohmann::json sdmSubscriptionJson;
  to_json(sdmSubscriptionJson, sdmSubscription);
  bsoncxx::document::value update_doc =
      bsoncxx::from_json(sdmSubscriptionJson.dump());

  try {
    // Execute update query
    auto result = coll.update_one(
        filter.view(),
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$set", update_doc.view())));

    // Check for update success
    if (result) {
      if (result->modified_count() == 1) {
        json_data = sdmSubscriptionJson;
        Logger::udr_db().info(
            "Successfully updated SDM subscription in MongoDB. UE ID: %s, Subs "
            "ID: %s: %s",
            ue_id, subs_id, json_data.dump());

        return true;
      } else {
        Logger::udr_db().error(
            "Failed to update SDM subscription. SDM Subscription UE ID: %s, "
            "Subs ID: %s not found",
            ue_id, subs_id);
        return false;
      }
    } else {
      Logger::udr_db().error(
          "Failed to update SDM subscription in MongoDB. UE ID: %s, Subs ID: "
          "%s",
          ue_id, subs_id);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while update sdm subscription in MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::create_sdm_subscriptions(
    const std::string& ue_id, oai::udr::model::SdmSubscription& sdmSubscription,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to the MySQL is currently inactive");
    return false;
  }

  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];

  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;
  bsoncxx::document::view_or_value filter = filter_builder.view();

  nlohmann::json tmp_json;
  to_json(tmp_json, sdmSubscription);
  tmp_json["ueid"] = ue_id;

  try {
    auto cursor = coll.find(filter.view());

    std::list<int32_t> subsIds;
    for (auto doc : cursor) {
      bsoncxx::document::element subs_id_elem = doc["subsId"];
      if (subs_id_elem) {
        int32_t subs_id = subs_id_elem.get_int32().value;
        subsIds.push_back(subs_id);
      }
    }
    subsIds.sort();
    int count   = 0;
    int subs_id = 0;
    for (auto el : subsIds) {
      if (el != count) {
        break;
      }
      count++;
    }
    subs_id = count;

    tmp_json["subsId"] = subs_id;
    auto document      = bsoncxx::from_json(tmp_json.dump());

    auto result = coll.insert_one(document.view());
    if (result) {
      json_data = tmp_json;
      Logger::udr_db().debug(
          "SdmSubscriptions POST: %s", json_data.dump().c_str());
      return true;
    } else {
      Logger::udr_db().error("Failed to insert document into MongoDB");
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while create sdm subscription in MongoDB: %s", e.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool mongo_db::query_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  bsoncxx::document::view_or_value query =
      bsoncxx::builder::stream::document{}
      << "ueid" << ue_id << bsoncxx::builder::stream::finalize;

  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];

  try {
    mongocxx::cursor result = coll.find(query.view());
    nlohmann::json j        = {};
    nlohmann::json tmp      = {};

    for (const bsoncxx::document::view& doc : result) {
      SdmSubscription sdmsubscriptions = {};
      tmp.clear();
      from_json(bsoncxx::to_json(doc), sdmsubscriptions);
      to_json(tmp, sdmsubscriptions);
      j.push_back(tmp);
    }
    json_data = j;
    Logger::udr_db().debug(
        "SdmSubscriptions GET: %s", json_data.dump().c_str());
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query sdm subscription from MongoDB: %s", e.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool mongo_db::query_sm_data(nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SessionManagementSubscriptionData"];

  try {
    auto result = coll.find({});

    auto row = result.begin();
    if (row == result.end()) {
      Logger::udr_db().error(
          "Empty document in MongoDB Collection "
          "SessionManagementSubscriptionData");
      return false;
    }

    for (auto&& view : result) {
      nlohmann::json j = query_sm_data_helper(view);
      json_data += j;
      Logger::udr_db().debug(
          "SessionManagementSubscriptionData: %s", j.dump().c_str());
    }
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query sm data from MongoDB: %s", e.what());
    return false;
  } catch (const std::exception& e) {
    Logger::udr_db().error(
        "Exception while query sm data from MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data,
    const std::optional<oai::model::common::Snssai>& snssai,
    const std::optional<std::string>& dnn) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  auto db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll   = db["SessionManagementSubscriptionData"];
  auto filter = bsoncxx::builder::stream::document{};
  filter << "ueid" << ue_id << "servingPlmnid" << serving_plmn_id;

  if (snssai.value().getSst() > 0) {
    filter << "singleNssai.sst" << snssai.value().getSst();
  }

  if (dnn.has_value()) {
    filter << "dnnConfigurations." + dnn.value()
           << bsoncxx::builder::stream::open_document << "$exists" << true
           << bsoncxx::builder::stream::close_document;
  }

  try {
    auto result = coll.find(filter.view());

    auto row = result.begin();
    if (row == result.end()) {
      Logger::udr_db().error(
          "Empty document in MongoDB: ueid=%s, servingPlmnid=%s", ue_id.c_str(),
          serving_plmn_id.c_str());
      return false;
    }

    for (auto&& view : result) {
      nlohmann::json j = query_sm_data_helper(view);
      json_data += j;
      Logger::udr_db().debug(
          "SessionManagementSubscriptionData: %s", j.dump().c_str());
    }
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query sm data from MongoDB: %s", e.what());
    return false;
  } catch (const std::exception& e) {
    Logger::udr_db().error(
        "Exception while query sm data from MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
nlohmann::json mongo_db::query_sm_data_helper(
    const bsoncxx::v_noabi::document::view& view) {
  // check if at least one document can be found
  SessionManagementSubscriptionData sessionmanagementsubscriptiondata = {};
  from_json(
      nlohmann::json::parse(bsoncxx::to_json(view)),
      sessionmanagementsubscriptiondata);

  nlohmann::json j;
  to_json(j, sessionmanagementsubscriptiondata);

  return j;
}

//------------------------------------------------------------------------------
bool mongo_db::insert_smf_context_non_3gpp(
    const std::string& ue_id, const int32_t& pdu_session_id,
    const oai::udr::model::SmfRegistration& smfRegistration,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];

  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << ue_id << "subpduSessionId" << pdu_session_id;
  auto filter = filter_builder.view();

  try {
    auto result = coll.find_one(filter);

    nlohmann::json tmp_json;
    to_json(tmp_json, smfRegistration);
    tmp_json["ueid"]                    = ue_id;
    tmp_json["pdu_session_id"]          = pdu_session_id;
    bsoncxx::document::value update_doc = bsoncxx::from_json(tmp_json.dump());

    if (result) {
      auto update_result = coll.update_one(
          filter, bsoncxx::builder::basic::make_document(
                      bsoncxx::builder::basic::kvp("$set", update_doc.view())));

      if (!update_result) {
        Logger::udr_db().error("Failed to update SmfRegistration document.");
        return false;
      }
    } else {
      auto insert_result = coll.insert_one(update_doc.view());

      if (!insert_result) {
        Logger::udr_db().error("Failed to insert SmfRegistration document.");
        return false;
      }
    }

    to_json(json_data, smfRegistration);
    Logger::udr_db().debug("SmfRegistration PUT: %s", json_data.dump().c_str());

    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while insert smf context in MongoDB: %s", e.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool mongo_db::delete_smf_context(
    const std::string& ue_id, const int32_t& pdu_session_id) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];

  bsoncxx::builder::stream::document query_builder{};
  query_builder << "ueid" << ue_id << "subpduSessionId" << pdu_session_id;

  try {
    auto result = coll.delete_one(query_builder.view());
    if (result->deleted_count() == 0) {
      Logger::udr_db().warn(
          "No documents matching query found. Query: %s",
          bsoncxx::to_json(query_builder.view()).c_str());
      return false;
    }
    Logger::udr_db().debug("SmfRegistration DELETE - successful");
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while delete smf context from MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_smf_registration(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];

  try {
    bsoncxx::stdx::optional<bsoncxx::document::value> result = coll.find_one(
        bsoncxx::builder::stream::document{}
        << "ueid" << ue_id << "subpduSessionId" << pdu_session_id
        << bsoncxx::builder::stream::finalize);

    if (result) {
      auto doc_view                   = result->view();
      SmfRegistration smfregistration = {};
      from_json(bsoncxx::to_json(doc_view), smfregistration);
      to_json(json_data, smfregistration);

      Logger::udr_db().debug("SmfRegistration GET: %s", json_data.dump());
      return true;
    } else {
      Logger::udr_db().error(
          "SmfRegistration no data！ue_id: %s, pdu_session_id: %d",
          ue_id.c_str(), pdu_session_id);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query smf context from MongoDB: %s", e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_smf_reg_list(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];

  bsoncxx::builder::stream::document filter{};
  filter << "ueid" << ue_id;

  try {
    mongocxx::cursor result = coll.find(filter.view());

    nlohmann::json j   = {};
    nlohmann::json tmp = {};

    for (const bsoncxx::document::view& doc : result) {
      SmfRegistration smfregistration = {};

      tmp.clear();
      from_json(bsoncxx::to_json(doc), smfregistration);
      to_json(tmp, smfregistration);
      j += tmp;
    }
    json_data = j;

    Logger::udr_db().debug("SmfRegistrations GET: %s", j.dump().c_str());
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query smf registration list from MongoDB: %s",
        e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_smf_select_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    return false;
  }
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfSelectionSubscriptionData"];

  auto query_filter = bsoncxx::builder::stream::document{}
                      << "ueid" << ue_id << "servingPlmnid" << serving_plmn_id
                      << bsoncxx::builder::stream::finalize;

  try {
    auto query_result = coll.find_one(query_filter.view());

    if (!query_result) {
      Logger::udr_db().error(
          "SmfSelectionSubscriptionData no data！Query: %s",
          bsoncxx::to_json(query_filter.view()).c_str());
      return false;
    }

    auto smfselectionsubscriptiondata = SmfSelectionSubscriptionData();
    from_json(
        nlohmann::json::parse(bsoncxx::to_json(query_result.value().view())),
        smfselectionsubscriptiondata);

    to_json(json_data, smfselectionsubscriptiondata);

    Logger::udr_db().debug(
        "SmfSelectionSubscriptionData GET: %s", json_data.dump().c_str());

    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query smf selection subscription data from MongoDB: "
        "%s",
        e.what());
    return false;
  }
}
