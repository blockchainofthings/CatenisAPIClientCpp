//
//  CatenisApiClient.h
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 5/25/17.
//
#ifndef __CATENISAPICLIENT_H__
#define __CATENISAPICLIENT_H__

#include <string>
#include <map>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>

// Version specific constants
const std::string API_PATH = "/api/";
const std::string SIGN_VERSION_ID = "CTN1";
const std::string SIGN_METHOD_ID = "CTN1-HMAC-SHA256";
const std::string  SCOPE_REQUEST = "ctn1_request";
const std::string TIME_STAMP_HDR = "x-bcot-timestamp";
const int SIGN_VALID_DAYS = 7;

namespace ctn
{

struct MethodOption
{
    std::string encoding;
    bool encrypt;
    std::string storage;

    MethodOption()
    {
        encoding = "utf8";
        encrypt = true;
        storage = "auto";
    }

    MethodOption(std::string encoding, bool encrypt, std::string storage)
    {
        this->encoding = encoding;
        this->encrypt = encrypt;
        this->storage = storage;
    }
};

struct Device
{
    std::string id;
    bool is_prod_uniqueid;

    Device(std::string id, bool is_prod_uniqueid = false)
    {
        this->id = id;
        this->is_prod_uniqueid = is_prod_uniqueid;
    }
};

class CtnApiClient
{

private:

    std::string device_id_;
    std::string api_access_secret_;

    std::string host_;
    std::string subdomain_;
    bool secure_;
    std::string version_;

    std::string uri_prefix_;
    std::string root_api_endpoint_;
    time_t lastSignDate;
    std::string lastSignKey;
    //reqParams

    //TODO: need to look more into boost.asio to be sure about the return type + parameters
    
    void signRequest(std::string verb, std::string endpoint, std::map<std::string, std::string> &headers, std::string payload, time_t now);
    
    std::string hashData(const std::string str);
    // default hex_encode is false
    std::string signData(const std::string key, const std::string data, bool hex_encode = false);

public:
    
    //TODO move to private
    bool sendRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, boost::property_tree::ptree &request_data, std::string &response_data);
    
    //TODO: add comments specifying return + parameter for all functions
    CtnApiClient(std::string device_id, std::string api_access_secret, std::string host = "catenis.io", std::string environment = "prod", bool secure = true, std::string version = "0.2");

    bool logMessage(std::string message, std::string &data, const MethodOption &option = MethodOption());
    bool sendMessage(const Device &device, std::string message, std::string &data, const MethodOption &option = MethodOption());
    bool readMessage(std::string message_id, std::string &data, const MethodOption &option = MethodOption());
    bool retrieveMessageContainer(std::string message_id, std::string &data);
    
};

}


#endif  // __CATENISAPICLIENT_H__
