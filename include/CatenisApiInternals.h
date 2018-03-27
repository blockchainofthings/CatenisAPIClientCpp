//
//  CatenisApiInternals.h
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 6/21/17.
//
#ifndef __CATENISAPIINTERNALS_H__
#define __CATENISAPIINTERNALS_H__

#if defined(COM_SUPPORT_LIB_POCO)
// Add libs for windows
#ifdef _WIN32
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#endif
#endif

#include <map>
#include <string>

#include <CatenisApiClient.h>

// Internal constants
const std::string API_PATH = "/api/";
const std::string SIGN_VERSION_ID = "CTN1";
const std::string SIGN_METHOD_ID = "CTN1-HMAC-SHA256";
const std::string SCOPE_REQUEST = "ctn1_request";
const std::string TIME_STAMP_HDR = "x-bcot-timestamp";
const int SIGN_VALID_DAYS = 7;

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
#include <json-spirit/json_spirit_value.h>
#elif defined(COM_SUPPORT_LIB_POCO)
// Forward declare Poco JSON object
namespace Poco
{
    namespace JSON
    {
        class Object;
    }
}
#endif

namespace ctn
{
// Forward declaration of ApiErrorResponse structure
struct ApiErrorResponse;

class CtnApiInternals
{
private:
    std::string device_id_;
    std::string api_access_secret_;
    
    std::string host_;
    std::string port_;
    std::string subdomain_;
    bool secure_;
    std::string version_;
    
    std::string root_api_endpoint_;
    time_t last_signdate_;
    std::string last_signkey_;
    
    void signRequest(std::string verb, std::string endpoint, std::map<std::string, std::string> &headers, std::string payload, time_t now);
    std::string hashData(const std::string str);
    std::string signData(const std::string key, const std::string data, bool hex_encode = false);

    void parseApiErrorResponse(ApiErrorResponse &error_response, std::string &json_data);
    
public:
    
    CtnApiInternals(std::string device_id, std::string api_access_secret, std::string host, std::string port, std::string environment, bool secure, std::string version);
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
    void httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, json_spirit::mValue const &reqData, std::string &response_data);
#elif defined(COM_SUPPORT_LIB_POCO)
    void httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, Poco::JSON::Object &request_data, std::string &response_data);
#endif

    // Methods to parse the returned API Json string-messages.
    void parseLogMessage(LogMessageResult &user_return_data, std::string json_data);
    void parseSendMessage(SendMessageResult &user_return_data, std::string json_data);
    void parseReadMessage(ReadMessageResult &user_return_data, std::string json_data);
    void parseRetrieveMessageContainer(RetrieveMessageContainerResult &user_return_data, std::string json_data);
    void parseListMessages(ListMessagesResult &user_return_data, std::string json_data);
	void parseListPermissionEvents(ListPermissionEventsResult &user_return_data, std::string json_data);
	void parseRetrievePermissionRights(RetrievePermissionRightsResult &user_return_data, std::string json_data);
	void parseListNotificationEvents(ListNotificationEventsResult &user_return_data, std::string json_data);
};

}

#endif // __CATENISAPIINTERNALS_H__
