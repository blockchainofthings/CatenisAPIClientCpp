//
//  CatenisApiClient.cpp
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 5/25/17.
//  Modifications by R. Benson Evans on 2/20/2018.
//

#include <iostream>
#include <string>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

#include <CatenisApiException.h>
#include <CatenisApiInternals.h>
#include <CatenisApiClient.h>

// API Method: Log Message
void ctn::CtnApiClient::logMessage(LogMessageResult &data, std::string message, const MessageOptions&option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    // write request body
    request_data.put("message", message);
    request_data.put("options.encoding", option.encoding);
    request_data.put("options.encrypt", option.encrypt);
    request_data.put("options.storage", option.storage);
    
    std::string http_return_data;
    this->internals_->httpRequest("POST", "messages/log", params, queries, request_data, http_return_data);
    this->internals_->parseLogMessage(data, http_return_data);
}

// API Method: Send Message
void ctn::CtnApiClient::sendMessage(SendMessageResult &data, const Device &device, std::string message, const MessageOptions&option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    // write request body
    request_data.put("targetDevice.id", device.device_id);
    request_data.put("targetDevice.isProdUniqueId", device.is_prod_uniqueid);
    request_data.put("options.encoding", option.encoding);
    request_data.put("message", message);
    request_data.put("options.readConfirmation", option.readConfirmation);
    request_data.put("options.encoding", option.encoding);
    request_data.put("options.encrypt", option.encrypt);
    request_data.put("options.storage", option.storage);
    
    std::string http_return_data;
    this->internals_->httpRequest("POST", "messages/send", params, queries, request_data, http_return_data);
    this->internals_->parseSendMessage(data, http_return_data); 
}

// API Method: Read Message
void ctn::CtnApiClient::readMessage(ReadMessageResult &data, std::string message_id, std::string encoding)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    params[":messageId"] = message_id;
    queries["encoding"] = encoding;
    
    std::string http_return_data;
    this->internals_->httpRequest("GET", "messages/:messageId", params, queries, request_data, http_return_data);
    this->internals_->parseReadMessage(data, http_return_data); 
}

// API Method: Retreive Message Containter
void ctn::CtnApiClient::retrieveMessageContainer(RetrieveMessageContainerResult &data, std::string message_id)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    params[":messageId"] = message_id;
    
    std::string http_return_data;
    this->internals_->httpRequest("GET", "messages/:messageId/container", params, queries, request_data, http_return_data);
    this->internals_->parseRetrieveMessageContainer(data, http_return_data); 
}

// API Method: List Messages
void ctn::CtnApiClient::listMessages(ListMessagesResult &data, std::string action, std::string direction, std::string from_device_ids, std::string to_device_ids, std::string from_device_prod_ids, std::string to_device_prod_ids, std::string read_state, std::string start_date, std::string endDate)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    queries["action"] = action;
    queries["direction"] = direction;
    
    if(from_device_ids != "") queries["fromDeviceIds"] = from_device_ids;
    if(to_device_ids != "") queries["toDeviceIds"] = to_device_ids;
    if(from_device_prod_ids != "") queries["fromDeviceProdUniqueIds"] = from_device_prod_ids;
    if(to_device_prod_ids != "") queries["toDeviceProdUniqueIds"] = to_device_prod_ids;
    
    queries["readState"] = read_state;
    
    if(start_date != "") queries["startDate"] = start_date;
    if(endDate != "") queries["endDate"] = endDate;
    
    std::string http_return_data;
    this->internals_->httpRequest("GET", "messages", params, queries, request_data, http_return_data);
    this->internals_->parseListMessages(data, http_return_data);
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
