//
//  CatenisApiClient.cpp
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 5/25/17.
//  Modifications by R. Benson Evans on 2/20/2018.
//
#include <CatenisApiException.h>
#include <CatenisApiClient.h>
#include <CatenisApiInternals.h>

#include <iostream>
#include <string>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

// API Method: Log Message
void ctn::CtnApiClient::logMessage(LogMessageResult &data, std::string message, const MethodOption &option)
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
    parseLogMessage(data, http_return_data);
}

// API Method: Send Message
void ctn::CtnApiClient::sendMessage(SendMessageResult &data, const Device &device, std::string message, const MethodOption &option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    // write request body
    request_data.put("targetDevice.id", device.device_id);
    request_data.put("targetDevice.isProdUniqueId", device.is_prod_uniqueid);
    request_data.put("options.encoding", option.encoding);
    request_data.put("message", message);
    request_data.put("options.encoding", option.encoding);
    request_data.put("options.encrypt", option.encrypt);
    request_data.put("options.storage", option.storage);
    
    std::string http_return_data;
    this->internals_->httpRequest("POST", "messages/send", params, queries, request_data, http_return_data);
    parseSendMessage(data, http_return_data); 
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
    parseReadMessage(data, http_return_data); 
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
    parseRetrieveMessageContainer(data, http_return_data); 
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
    parseListMessages(data, http_return_data);
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

// Private Method.
void ctn::CtnApiClient::parseLogMessage(LogMessageResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            user_return_data.messageId = pt.get<std::string>("data.messageId"); 
            return;
        }
        catch(...)
        {
            throw(new CatenisAPIClientError("Server error: messageId not returned", false, 0));
        }     
    }

    // Server error encountered.
    // args: [arg 2: false] for-not-a-client-side error; [arg 3: 0] for http status code.
    throw(new CatenisAPIClientError("Server side error encountered", false, 0));
}

// Private Method.
void ctn::CtnApiClient::parseSendMessage(SendMessageResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            user_return_data.messageId = pt.get<std::string>("data.messageId"); 
            return;
        }
        catch(...)
        {
            throw(new CatenisAPIClientError("Server error: messageId not returned", false, 0));
        }     
    }

    // Server error encountered.
    // args: [arg 2: false] for-not-a-client-side error; [arg 3: 0] for http status code.
    throw(new CatenisAPIClientError("Server side error encountered", false, 0));
}

// Private Method.
void ctn::CtnApiClient::parseReadMessage(ReadMessageResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            user_return_data.action = pt.get<std::string>("data.action");
            user_return_data.fromDeviceId = pt.get<std::string>("data.from.deviceId","");
            user_return_data.fromName = pt.get<std::string>("data.from.name","");
            user_return_data.fromProdUniqueId = pt.get<std::string>("data.from.prodUniqueId","");
            user_return_data.toDeviceId = pt.get<std::string>("data.to.deviceId","");
            user_return_data.toName = pt.get<std::string>("data.to.name","");
            user_return_data.toProdUniqueId = pt.get<std::string>("data.to.prodUniqueId","");
            user_return_data.message = pt.get<std::string>("data.message");
            return;
        }
        catch(...)
        {
            throw(new CatenisAPIClientError("Server error: data element not returned", false, 0));
        }     
    }

    // Server error encountered.
    // args: [arg 2: false] for-not-a-client-side error; [arg 3: 0] for http status code.
    throw(new CatenisAPIClientError("Server side error encountered", false, 0));
}

// Private Method.
void ctn::CtnApiClient::parseRetrieveMessageContainer(RetrieveMessageContainerResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            user_return_data.txid = pt.get<std::string>("data.blockchain.txid");
            user_return_data.isConfirmed = pt.get<std::string>("data.blockchain.isConfirmed");
            user_return_data.externalStorage = pt.get<std::string>("data.externalStorage","");
            user_return_data.storageProviderName = pt.get<std::string>("data.storageProviderName","");
            return;
        }
        catch(...)
        {
            throw(new CatenisAPIClientError("Server error: data element not returned", false, 0));
        }     
    }

    // Server error encountered.
    // args: [arg 2: false] for-not-a-client-side error; [arg 3: 0] for http status code.
    throw(new CatenisAPIClientError("Server side error encountered", false, 0));
}

// Private Method.
void ctn::CtnApiClient::parseListMessages(ListMessagesResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("data.messages"))
            {
                // std::cout << "First data: " << v.first.data() << std::endl;
                boost::property_tree::ptree subtree = (boost::property_tree::ptree) v.second;
                
                MessageDescription *msgElement = new MessageDescription();
                msgElement->messageId = subtree.get<std::string>("messageId");
                msgElement->action = subtree.get<std::string>("action");
                msgElement->read = subtree.get<std::string>("read");
                msgElement->date = subtree.get<std::string>("date");
                msgElement->direction = subtree.get<std::string>("direction","");
                msgElement->fromDeviceId = subtree.get<std::string>("from.deviceId","");
                msgElement->fromName = subtree.get<std::string>("from.name","");
                msgElement->fromProdUniqueId = subtree.get<std::string>("from.prodUniqueId","");
                msgElement->toDeviceId = subtree.get<std::string>("to.deviceId","");
                msgElement->toName = subtree.get<std::string>("to.name","");
                msgElement->toProdUniqueId = subtree.get<std::string>("to.prodUniqueId","");
                user_return_data.messageList.push_back(msgElement);
            }
            user_return_data.msgCount = pt.get<std::string>("data.msgCount","");
            user_return_data.countExceeded = pt.get<std::string>("data.countExceeded");
            return;
       }
       catch(...)
       {
            throw(new CatenisAPIClientError("Server error: data element not returned", false, 0));
       }     
    }
    // Server error encountered.
    // args: [arg 2: false] for-not-a-client-side error; [arg 3: 0] for http status code.
    throw(new CatenisAPIClientError("Server side error encountered", false, 0));
}
