/**
 * @author Sungwoo Bae
 * @createdAt 25/05/2017
 */
#ifndef __CATENISAPICLIENT_H__
#define __CATENISAPICLIENT_H__

#include <string>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>

// Version specific macro
#define API_PATH "/api/"
#define SIGN_VERSION_ID "CTN1"
#define SIGN_METHOD_ID "CTN1-HMAC-SHA256"
#define SCOPE_REQUEST "ctn1_request"
#define TIME_STAMP_HDR "X-BCoT-Timestamp"
#define SIGN_VALID_DAYS 7

namespace ctn
{

struct MethodOption
{

    std::string encoding;
    bool crypt;
    std::string storage;

    MethodOption()
    {
        encoding = "utf-8";
        crypt = true;
        storage = "auto";
    }

    MethodOption(std::string encoding, bool crypt, std::string storage)
    {
        this->encoding = encoding;
        this->crypt = crypt;
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

struct MessageContainer
{

    std::string txid;
    bool is_confirmed;
    std::string ipfs;
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
    //std::string lastSignDate;
    //std::string lastSignKey;
    //reqParams

    //TODO: need to look more into boost.asio to be sure about the return type + parameters
    boost::asio::streambuf postRequest(std::string methodpath, boost::property_tree::ptree data);
    boost::asio::streambuf getRequest(std::string methodpath, std::unordered_map<std::string, std::string> &params);
    std::string hashData(const std::string str);
    // default hex_encode is false
    std::string signData(const std::string key, const std::string data, bool hex_encode = false);

public:
    
    //TODO: add comments specifying return + parameter for all functions
    CtnApiClient(std::string device_id, std::string api_access_secret, std::string host = "catenis.io", std::string environment = "prod", bool secure = true, std::string version = "0.2");

    bool logMessage(std::string message, const MethodOption &option = MethodOption());
    bool sendMessage(const Device &device, std::string message, const MethodOption &option = MethodOption());
    std::string readMessage(std::string message, const MethodOption &option = MethodOption());
    MessageContainer& retrieveMessageContainer(std::string message_id);
    
};

}


#endif  // __CATENISAPICLIENT_H__
