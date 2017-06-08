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

/*
 * Struct to contain options for main api calls
 *
 * @member encoding : Value identifying the encoding of the message
 * ["utf8"|"base64"|"hex"]
 * @member encrypt : Indicates whether message should be encrypted before storing it
 * @member storage : Value identifying where the message should be stored
 * [“auto"|"embedded"|"external"]
 */
struct MethodOption
{
    std::string encoding;
    bool encrypt;
    std::string storage;
    
    // Default contructor with default values for members
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
    
/*
 * Struct to contain device information
 *
 * @member id : ID of target device. Should be Catenis device ID unless isProdUniqueId is true
 * @member isProdUniqueId : Indicate whether supply ID is a product unique ID
 */
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

    std::string root_api_endpoint_;
    time_t last_signdate_;
    std::string last_signkey_;
    
    bool httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, boost::property_tree::ptree &request_data, std::string &response_data);
    
    void signRequest(std::string verb, std::string endpoint, std::map<std::string, std::string> &headers, std::string payload, time_t now);
    std::string hashData(const std::string str);
    std::string signData(const std::string key, const std::string data, bool hex_encode = false);

public:
    
    /* Constructor
     *
     * @param[in] device_id : Catenis device ID
     * @param[in] api_access_secret : Catenis device's API access secret
     * @param[in] host : (optional, default: catenis.io) Host name (with optional port) of target Catenis API server
     * @param[in] environment : (optional, default: 'prod') Environment of target Catenis API server
     * ["prod"|"beta"]
     * @param[in] secure : (optional, default: true) Indicates whether a secure connection (HTTPS) should be used
     * @param[in] version : (optional, default: 0.2) Version of Catenis API to target
     */
     CtnApiClient(std::string device_id, std::string api_access_secret, std::string host = "catenis.io", std::string environment = "prod", bool secure = true, std::string version = "0.2");
    
    /*
     * Log a message
     *
     * @param[in] message : The messsage to store
     * @param[out] data : The data to parse response into
     * @param[in] (optional) option : Options to log message
     *
     * @return true if no error has occured.
     *
     * @see ctn::MethodOption
     */
    bool logMessage(std::string message, std::string &data, const MethodOption &option = MethodOption());
    
    /*
     * Send a message
     *
     * @param[in] device : Device that receives message
     * @param[in] message : The messsage to send
     * @param[out] data : The data to parse response into
     * @param[in] (optional) option : Options to send message
     *
     * @return true if no error has occured.
     *
     * @see ctn::Device
     * @see ctn::MethodOption
     */
    bool sendMessage(const Device &device, std::string message, std::string &data, const MethodOption &option = MethodOption());
    
    /*
     * Read a message
     *
     * @param[in] message_id : ID of message to read
     * @param[out] data : The data to parse response into
     * @param[in] (optional, default: "utf8") encoding : The encoding that should be used for the returned message
     * ["utf8"|"base64"|"hex"]
     *
     * @return true if no error has occured.
     */
    bool readMessage(std::string message_id, std::string &data, std::string encoding = "utf8");
    
    /*
     * Retrieve message container
     *
     * @param[in] message_id : ID of message to retrieve container info
     * @param[out] data : The data to parse response into
     *
     * @return true if no error has occured.
     */
    bool retrieveMessageContainer(std::string message_id, std::string &data);
    
};

}


#endif  // __CATENISAPICLIENT_H__
