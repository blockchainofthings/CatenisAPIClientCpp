//
//  CatenisApiClient.h
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 5/25/17.
//
#ifndef __CATENISAPICLIENT_H__
#define __CATENISAPICLIENT_H__

#include <string>
#include <ctime>

// Version specific constants
const std::string DEFAULT_API_VERSION = "0.3";

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
    std::string device_id;
    bool is_prod_uniqueid;
    
    Device(std::string device_id, bool is_prod_uniqueid = false)
    {
        this->device_id = device_id;
        this->is_prod_uniqueid = is_prod_uniqueid;
    }
};
    
// Forward declare internals
class CtnApiInternals;

class CtnApiClient
{

private:
    
    /* Pointer to object that handles all internal functionalities of CtnApiClient
     *
     * @see ctn::CtnApiInternals
     */
    CtnApiInternals *internals_;
    
public:
    
    /* Constructor
     *
     * @param[in] device_id : Catenis device ID
     * @param[in] api_access_secret : Catenis device's API access secret
     * @param[in] host (optional, default: catenis.io) :  Host name of target Catenis API server
     * @param[in] port (optional, default: "") : Port of target to connect
     * @param[in] environment (optional, default: "prod") :  Environment of target Catenis API server
     * ["prod"|"beta"]
     * @param[in] secure (optional, default: true) :  Indicates whether a secure connection (HTTPS) should be used
     * @param[in] version (optional, default: DEFAULT_API_VERSION) :  Version of Catenis API to target
     */
    CtnApiClient(std::string device_id, std::string api_access_secret, std::string host = "catenis.io", std::string port = "",std::string environment = "prod", bool secure = true, std::string version = DEFAULT_API_VERSION);
    
    /* Destructor */
    ~CtnApiClient();
    
    /*
     * Log a message
     *
     * @param[in] message : The messsage to store
     * @param[out] data : The data to parse response into
     * @param[in] option (optional) :  Options to log message
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
     * @param[in] option (optional) :  Options to send message
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
     * @param[in] encoding (optional, default: "utf8") :  The encoding that should be used for the returned message
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
    
    /*
     * Retrieves a list of message entries filtered by a given criteria
     *
     * @param[out] data : The data to parse response into
     * @param[in] action (optional, default: "any") :  Value specifying the action originally performed on the messages
     * ["log"|"send"|"any"]
     * @param[in] direction (optional, default: "any") :  The direction of the sent messages
     * ["inbound"|"outbound"|"any"]
     * @param[in] from_device_ids (optional) : Comma separated list of sender device IDs
     * @param[in] to_device_ids (optional) : Comma separated list of receiver device IDs
     * @param[in] from_device_prod_ids (optional) : Comma separated list of sender unique product IDs
     * @param[in] to_device_prod_ids (optional) : Comma separated list of receiver unique product IDs
     * @param[in] read_state (optional, default: "any") :  Value specifying the current read state of the messages
     * ["read"|"unread"|"any"]
     * @param[in] start_date (optional) : ISO 8601 formatted date and time specifying the lower boundary
     * @param[in] end_date (optional) : ISO 8601 formatted date and time specifying the upper boundary
     *
     * @return true if no error has occured.
     */
    bool listMessages(std::string &data, std::string action = "any", std::string direction = "any", std::string from_device_ids = "", std::string to_device_ids = "", std::string from_device_prod_ids = "", std::string to_device_prod_ids = "", std::string read_state = "any", std::string start_date = "", std::string endDate = "");
    
};

}


#endif  // __CATENISAPICLIENT_H__
