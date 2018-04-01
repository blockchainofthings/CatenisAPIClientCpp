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

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
#include <json-spirit/json_spirit_value.h>
#elif defined(COM_SUPPORT_LIB_POCO)
#include <Poco/JSON/Object.h>
#endif

#include <CatenisApiException.h>
#include <CatenisApiInternals.h>
#include <CatenisApiClient.h>


// API Method: Log Message
void ctn::CtnApiClient::logMessage(LogMessageResult &data, std::string message, const MessageOptions &option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;

    // write request body
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
    json_spirit::mObject objData;

    objData["message"] = message;

    json_spirit::mObject objOptions;

    objOptions["encoding"] = option.encoding;
    objOptions["encrypt"] = option.encrypt;
    objOptions["storage"] = option.storage;

    objData["options"] = objOptions;

    json_spirit::mValue request_data(objData);
#elif defined(COM_SUPPORT_LIB_POCO)
    Poco::JSON::Object request_data;

    request_data.set("message", message);

    Poco::JSON::Object options;
    options.set("encoding", option.encoding);
    options.set("encrypt", option.encrypt);
    options.set("storage", option.storage);
    request_data.set("options", options);
#endif

    std::string http_return_data;
    this->internals_->httpRequest("POST", "messages/log", params, queries, request_data, http_return_data);
    this->internals_->parseLogMessage(data, http_return_data);
}

// API Method: Send Message
void ctn::CtnApiClient::sendMessage(SendMessageResult &data, const Device &device, std::string message, const MessageOptions&option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;

    // write request body
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
    json_spirit::mObject objData;

    json_spirit::mObject objTargetDevice;

    objTargetDevice["id"] = device.device_id;
    objTargetDevice["isProdUniqueId"] = device.is_prod_uniqueid;

    objData["targetDevice"] = objTargetDevice;

    objData["message"] = message;

    json_spirit::mObject objOptions;

    objOptions["readConfirmation"] = option.readConfirmation;
    objOptions["encoding"] = option.encoding;
    objOptions["encrypt"] = option.encrypt;
    objOptions["storage"] = option.storage;

    objData["options"] = objOptions;

    json_spirit::mValue request_data(objData);
#elif defined(COM_SUPPORT_LIB_POCO)
    Poco::JSON::Object request_data;

    Poco::JSON::Object targetD;
    targetD.set("id", device.device_id);
    targetD.set("isProdUniqueId", device.is_prod_uniqueid);
    request_data.set("targetDevice", targetD);

    request_data.set("message", message);

    Poco::JSON::Object options;
    options.set("readConfirmation", option.readConfirmation);
    options.set("encoding", option.encoding);
    options.set("encrypt", option.encrypt);
    options.set("storage", option.storage);
    request_data.set("options", options);
#endif

    std::string http_return_data;
    this->internals_->httpRequest("POST", "messages/send", params, queries, request_data, http_return_data);
    this->internals_->parseSendMessage(data, http_return_data); 
}

// API Method: Read Message
void ctn::CtnApiClient::readMessage(ReadMessageResult &data, std::string message_id, std::string encoding)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;

    params[":messageId"] = message_id;
    queries["encoding"] = encoding;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
    json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
    Poco::JSON::Object request_data;
#endif

    std::string http_return_data;
    this->internals_->httpRequest("GET", "messages/:messageId", params, queries, request_data, http_return_data);
    this->internals_->parseReadMessage(data, http_return_data); 
}

// API Method: Retreive Message Containter
void ctn::CtnApiClient::retrieveMessageContainer(RetrieveMessageContainerResult &data, std::string message_id)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;

    params[":messageId"] = message_id;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
    json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
    Poco::JSON::Object request_data;
#endif

    std::string http_return_data;
    this->internals_->httpRequest("GET", "messages/:messageId/container", params, queries, request_data, http_return_data);
    this->internals_->parseRetrieveMessageContainer(data, http_return_data); 
}

// API Method: List Messages
void ctn::CtnApiClient::listMessages(ListMessagesResult &data, std::string action, std::string direction, std::string from_device_ids, std::string to_device_ids, std::string from_device_prod_ids, std::string to_device_prod_ids, std::string read_state, std::string start_date, std::string endDate)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;

    queries["action"] = action;
    queries["direction"] = direction;
    
    if(!from_device_ids.empty()) queries["fromDeviceIds"] = from_device_ids;
    if(!to_device_ids.empty()) queries["toDeviceIds"] = to_device_ids;
    if(!from_device_prod_ids.empty()) queries["fromDeviceProdUniqueIds"] = from_device_prod_ids;
    if(!to_device_prod_ids.empty()) queries["toDeviceProdUniqueIds"] = to_device_prod_ids;
    
    queries["readState"] = read_state;
    
    if(!start_date.empty()) queries["startDate"] = start_date;
    if(!endDate.empty()) queries["endDate"] = endDate;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
    json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
    Poco::JSON::Object request_data;
#endif

    std::string http_return_data;
    this->internals_->httpRequest("GET", "messages", params, queries, request_data, http_return_data);
    this->internals_->parseListMessages(data, http_return_data);
}

// API Method: List Permission Events
void ctn::CtnApiClient::listPermissionEvents(ListPermissionEventsResult &data)
{
	std::map<std::string, std::string> params;
	std::map<std::string, std::string> queries;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
	json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
	Poco::JSON::Object request_data;
#endif
	std::string http_return_data;
	this->internals_->httpRequest("GET", "permission/events", params, queries, request_data, http_return_data);
	this->internals_->parseListPermissionEvents(data, http_return_data);
}

// API Method: Retrieve Permission Rights
void ctn::CtnApiClient::retrievePermissionRights(RetrievePermissionRightsResult &data, std::string eventName)
{
	std::map<std::string, std::string> params;
	std::map<std::string, std::string> queries;

	params[":eventName"] = eventName;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
	json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
	Poco::JSON::Object request_data;
#endif
	std::string http_return_data;
	this->internals_->httpRequest("GET", "permission/events/:eventName/rights", params, queries, request_data, http_return_data);
	this->internals_->parseRetrievePermissionRights(data, http_return_data);
}

// API Method: Set Permission Rights
void ctn::CtnApiClient::setPermissionRights(SetPermissionRightsResult &data, std::string eventName, std::string system, std::shared_ptr<SetRightsCtnNode> ctnNodeObj = nullptr, std::shared_ptr<SetRightsClient> clientObj = nullptr)
{
	std::map<std::string, std::string> params;
	std::map<std::string, std::string> queries;

	params[":eventName"] = eventName;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
	json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
	Poco::JSON::Object request_data;

	std::list<std::string> allowed;
	std::list<std::string> denied;

	//if (!system.empty())
	//	request_data.set("system", "deny");

	// //////////////////////////// Prepare JSON for Catenis Node ////////////////////////////////
	if (ctnNodeObj != nullptr)
	{
		std::cout << "------------ JSON Catenis Node -----------------" << std::endl;
		Poco::JSON::Object ctnNode;
		if (ctnNodeObj->allowed.size() > 0)
		{
			Poco::JSON::Array allowCtnNode;
			for (std::list<std::string>::const_iterator i = ctnNodeObj->allowed.begin(); i != ctnNodeObj->allowed.end(); ++i)
			{
				std::cout << "allow: \t" + *i << std::endl; // ############ TESTING
				allowCtnNode.add(*i);
			}
			ctnNode.set("allow", allowCtnNode);
		}
		if (ctnNodeObj->denied.size() > 0)
		{
			Poco::JSON::Array denyCtnNode;
			for (std::list<std::string>::const_iterator i = ctnNodeObj->denied.begin(); i != ctnNodeObj->denied.end(); ++i)
			{
				std::cout << "deny \t" + *i << std::endl; // ############ TESTING
				denyCtnNode.add(*i);
			}
			ctnNode.set("deny", denyCtnNode);
		}
		if (ctnNodeObj->revoked.size() > 0)
		{
			Poco::JSON::Array revokedCtnNode;
			for (std::list<std::string>::const_iterator i = ctnNodeObj->revoked.begin(); i != ctnNodeObj->revoked.end(); ++i)
			{
				std::cout << "none: \t" + *i << std::endl; // ############ TESTING
				revokedCtnNode.add(*i);
			}
			ctnNode.set("none", revokedCtnNode);
		}
		request_data.set("catenisNode", ctnNode);
	}

	// //////////////////////////// Prepare JSON for Client ////////////////////////////////
	if (clientObj != nullptr)
	{
		std::cout << "------------ JSON Clients -----------------" << std::endl;
		Poco::JSON::Object client;
		if (clientObj->allowed.size() > 0)
		{
			Poco::JSON::Array allowClient;
			for (std::list<std::string>::const_iterator i = clientObj->allowed.begin(); i != clientObj->allowed.end(); ++i)
			{
				std::cout << "allow \t" + *i << std::endl; // ############ TESTING
				allowClient.add(*i);
			}
			client.set("allow", allowClient);
		}
		if (clientObj->denied.size() > 0)
		{
			Poco::JSON::Array denyClient;
			for (std::list<std::string>::const_iterator i = clientObj->denied.begin(); i != clientObj->denied.end(); ++i)
			{
				std::cout << "deny \t" + *i << std::endl;  // ############ TESTING
				denyClient.add(*i);
			}
			client.set("deny", denyClient);
}
		if (clientObj->revoked.size() > 0)
		{
			Poco::JSON::Array revokedClient;
			for (std::list<std::string>::const_iterator i = clientObj->revoked.begin(); i != clientObj->revoked.end(); ++i)
			{
				std::cout << "none: \t" + *i << std::endl;  // ############ TESTING
				revokedClient.add(*i);
			}
			client.set("none", revokedClient);
		}
		request_data.set("client", client);
	}

#endif
	std::string http_return_data;
	this->internals_->httpRequest("POST", "permission/events/:eventName/rights", params, queries, request_data, http_return_data);
	this->internals_->parseSetPermissionRights(data, http_return_data);
}

// API Method: List Notification Events
void ctn::CtnApiClient::listNotificationEvents(ListNotificationEventsResult &data)
{
	std::map<std::string, std::string> params;
	std::map<std::string, std::string> queries;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
	json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
	Poco::JSON::Object request_data;
#endif
	std::string http_return_data;
	this->internals_->httpRequest("GET", "notification/events", params, queries, request_data, http_return_data);
	this->internals_->parseListNotificationEvents(data, http_return_data);
}

// API Method: Check Effective Permission Events
void ctn::CtnApiClient::checkEffectivePermissionRight(CheckEffectivePermissionRightResult &data, std::string eventName, std::string deviceId, std::string isProdUniqueId = "false")
{

	std::map<std::string, std::string> params;
	std::map<std::string, std::string> queries;

	queries["isProdUniqueId"] = "false"; // isProdUniqueId;

	params[":eventName"] = eventName;
	params[":deviceId"] = deviceId;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
	json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
	Poco::JSON::Object request_data;
#endif
	std::string http_return_data;
	this->internals_->httpRequest("GET", "permission/events/:eventName/rights/:deviceId", params, queries, request_data, http_return_data);
	this->internals_->parseCheckEffectivePermissionRight(data, http_return_data);
}

// API Method: Check Effective Permission Events
void ctn::CtnApiClient::retrieveDeviceIdInfo(DeviceIdInfoResult &data, std::string deviceId, std::string isProdUniqueId = "false")
{

	std::map<std::string, std::string> params;
	std::map<std::string, std::string> queries;

	queries["isProdUniqueId"] = isProdUniqueId;

	params[":deviceId"] = deviceId;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
	json_spirit::mValue request_data;
#elif defined(COM_SUPPORT_LIB_POCO)
	Poco::JSON::Object request_data;
#endif
	std::string http_return_data;
	this->internals_->httpRequest("GET", "devices/:deviceId", params, queries, request_data, http_return_data);
	this->internals_->parseRetrieveDeviceIdInfo(data, http_return_data);
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
