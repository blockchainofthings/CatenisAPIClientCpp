/**
 * @author Sungwoo Bae
 * @createAt 25/05/2017
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

namespace ctn {

struct ClientOption {

    std::string host;
    std::string environment;
    bool secure;
    std::string version;

    ClientOption()
    {
        host = "catenis.io";
        environment = "prod";
        secure = true;
        version = "0.2";
    }

    ClientOption(std::string host, std::string environment, bool secure, std::string version)
    {
        this->host = host;
        this->environment = environment;
        this->secure = secure;
        this->version = version;
    }
};

struct MethodOption {

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

struct Device {

    std::string id;
    bool isProdUniqueId;

    Device(std::string id, bool isProdUniqueId = false)
    {
        this->id = id;
        this->isProdUniqueId = isProdUniqueId;
    }
};

struct MessageContainer {

    std::string txid;
    bool isConfirmed;
    std::string ipfs;
};

class CtnApiClient
{

private:

    std::string deviceId;
    std::string apiAccessSecret;

    std::string host;
    std::string subdomain;
    bool secure;
    std::string version;

    std::string uriPrefix;
    std::string rootApiEndPoint;
    //std::string lastSignDate;
    //std::string lastSignKey;
    //reqParams

    //TODO: need to look more into boost.asio to be sure about the return type + parameters
    boost::asio::streambuf postRequest(std::string methodPath, boost::property_tree::ptree data);
    boost::asio::streambuf getRequest(std::string methodPath, std::unordered_map<std::string, std::string> &params);
    std::string hash_sha256(const std::string str);
    std::string sign_hmac_sha256(std::string key, std::string data);

public:
    
    //TODO: add comments specifying return + parameter for all functions
    //TODO: not sure about the third parameter where it passes temporary object created by contructor to
    // const reference --> need testing
    CtnApiClient(std::string deviceId, std::string apiAccessSecret, const ClientOption &option = ClientOption());

    bool logMessage(std::string message, const MethodOption &option = MethodOption());
    bool sendMessage(std::string message, const MethodOption &option = MethodOption());
    std::string readMessage(std::string message, const MethodOption &option = MethodOption());
    MessageContainer& retrieveMessageContainer(std::string messageId);
};

}


#endif  // __CATENISAPICLIENT_H__
