//
//  CatenisApiClient.cpp
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 5/25/17.
//
#include <CatenisApiClient.h>
#include <CatenisApiInternals.h>

#include <iostream>
#include <string>
#include <map>

#include <Poco/JSON/Object.h>

// API Method: Log Message
bool ctn::CtnApiClient::logMessage(std::string message, std::string &data, const MethodOption &option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    Poco::JSON::Object request_data;
    
    // write request body
    request_data.set("message", message);
    Poco::JSON::Object options;
    options.set("encoding", option.encoding);
    options.set("encrypt", option.encrypt);
    options.set("storage", option.storage);
    request_data.set("options", options);
    
    return this->internals_->httpRequest("POST", "messages/log", params, queries, request_data, data);
}

// API Method: Send Message
bool ctn::CtnApiClient::sendMessage(const Device &device, std::string message, std::string &data, const MethodOption &option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    Poco::JSON::Object request_data;
    
    // write request body
    request_data.set("message", message);
    Poco::JSON::Object targetD;
    targetD.set("id", device.id);
    targetD.set("isProdUniqueId", device.is_prod_uniqueid);
    Poco::JSON::Object options;
    options.set("encoding", option.encoding);
    options.set("encrypt", option.encrypt);
    options.set("storage", option.storage);
    request_data.set("targetDevice", targetD);
    request_data.set("options", options);
    
    return this->internals_->httpRequest("POST", "messages/send", params, queries, request_data, data);
}

// API Method: Read Message
bool ctn::CtnApiClient::readMessage(std::string message_id, std::string &data, std::string encoding)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    Poco::JSON::Object request_data;
    
    params[":messageId"] = message_id;
    queries["encoding"] = encoding;
    
    return this->internals_->httpRequest("GET", "messages/:messageId", params, queries, request_data, data);
}

// API Method: Retreive Message Containter
bool ctn::CtnApiClient::retrieveMessageContainer(std::string message_id, std::string &data)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    Poco::JSON::Object request_data;
    
    params[":messageId"] = message_id;
    
    return this->internals_->httpRequest("GET", "messages/:messageId/container", params, queries, request_data, data);
}

// API Method: List Messages
bool ctn::CtnApiClient::listMessages(std::string &data, std::string action, std::string direction, std::string from_device_ids, std::string to_device_ids, std::string from_device_prod_ids, std::string to_device_prod_ids, std::string read_state, std::string start_date, std::string endDate)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    Poco::JSON::Object request_data;
    
    queries["action"] = action;
    queries["direction"] = direction;
    
    if(from_device_ids != "") queries["fromDeviceIds"] = from_device_ids;
    if(to_device_ids != "") queries["toDeviceIds"] = to_device_ids;
    if(from_device_prod_ids != "") queries["fromDeviceProdUniqueIds"] = from_device_prod_ids;
    if(to_device_prod_ids != "") queries["toDeviceProdUniqueIds"] = to_device_prod_ids;
    
    queries["readState"] = read_state;
    
    if(start_date != "") queries["startDate"] = start_date;
    if(endDate != "") queries["endDate"] = endDate;
    
    return this->internals_->httpRequest("GET", "messages", params, queries, request_data, data);
}

// CtnApiClient Constructor
ctn::CtnApiClient::CtnApiClient(std::string device_id, std::string api_access_secret, std::string host, std::string port, std::string environment, bool secure, std::string version)
{
    // Init internals and pass param
    this->internals_ = new ctn::CtnApiInternals(device_id, api_access_secret, host, port, environment, secure, version);
}

// CtnApiClient Destructor
ctn::CtnApiClient::~CtnApiClient()
{
    delete this->internals_;
}
