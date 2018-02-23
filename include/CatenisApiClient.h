//
//  CatenisApiClient.h
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 5/25/17.
//  Modifications by R. Benson Evans on 2/20/2018.
//
#ifndef __CATENISAPICLIENT_H__
#define __CATENISAPICLIENT_H__

#include <string>
#include <ctime>

#include <list>

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

/*
 * Struct to contain the returned data.
 *
 * @member messageId : ID of logged message.
 */
struct LogMessageResult
{
	std::string messageId;
};

/*
 * Struct to contain the returned data.
 *
 * @member messageId : ID of sent message.
 */
struct SendMessageResult
{
	std::string messageId;
};

/*
 * Struct to contain the returned data.
 *
 * @member action : the action performed on the message: 'log' or 'send'.
 * @member fromDeviceId : Catenis ID of the sending device.
 * @member fromName : Device name.
 * @member fromProdUniqueId : Product unique ID.
 * @member toDeviceId : Catenis ID of the target device.
 * @member toName : Device name.
 * @member toProdUniqueId : Product unique ID.
 * @member message : the message read.
 */
struct ReadMessageResult
{
	std::string action;
	std::string fromDeviceId;
	std::string fromName;
	std::string fromProdUniqueId;
	std::string toDeviceId;
	std::string toName;
	std::string toProdUniqueId;
	std::string message;
};

/*
 * Struct to contain the returned data.
 *
 * @member txid : ID of blockchain transaction where messae is recorded.
 * @member isConfirmed : Indicates where the returned txid is confirmed.
 * @member externalStorage : [optional] only returned if message is stored in an external storage.
 * @member storageProviderName : Key: storage provider name.
 */
struct RetrieveMessageContainerResult
{
	std::string txid;
	std::string isConfirmed;
	std::string externalStorage; // optional.
	std::string storageProviderName;
};


/*
 * Struct to contain the returned data.
 *
 * @member messageId : ID of message.
 * @member action : Action performed: 'log' or 'send'.
 * @member direction : Direction of 'send' message: 'inbound' or 'outbound'.
 * @member fromDeviceId : Catenis ID of the sending device.
 * @member fromName : Device name.
 * @member fromProdUniqueId : Product unique ID.
 * @member toDeviceId : Catenis ID of the target device.
 * @member toName : Device name.
 * @member toProdUniqueId : Product unique ID.
 * @member readConfirmationEnabled : Indicates whether the message had been sent with read confirmation enabled.
 * @member read : Indicates whether the message had already been read.
 * @member date : ISO 8601 formatted date and time when message was logged, sent or received.
 */
struct MessageDescription
{
    std::string messageId;
    std::string action;
    std::string direction;
	std::string fromDeviceId;
	std::string fromName;
	std::string fromProdUniqueId;
	std::string toDeviceId;
	std::string toName;
	std::string toProdUniqueId;
    std::string readConfirmationEnabled;
    std::string read;
    std::string date;
};

/*
 * Struct to contain the returned data.
 *
 * @member messageList : List of structures.
 * @member msgCount : Number of messages for which information is returned.
 * @member countExceeded : Was the actual number of messages greater than the max returnable.
 */
struct ListMessagesResult
{
    std::list< MessageDescription * > messageList; 
	std::string msgCount;
	std::string countExceeded;
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
     * @param[out] data : The data to parse response into
     * @param[in] message : The messsage to store
     * @param[in] option (optional) :  Options to log message
     *
     * @return true if no error has occured.
     *
     * @see ctn::LogMessageResult
     * @see ctn::MethodOption
     */
    void logMessage(LogMessageResult &data, std::string message, const MethodOption &option = MethodOption());
    
    /*
     * Send a message
     *
     * @param[out] data : The data to parse response into
     * @param[in] device : Device that receives message
     * @param[in] message : The messsage to send
     * @param[in] option (optional) :  Options to send message
     *
     * @return true if no error has occured.
     *
     * @see ctn::SendMessageResult
     * @see ctn::Device
     * @see ctn::MethodOption
     */
    void sendMessage(SendMessageResult &data, const Device &device, std::string message, const MethodOption &option = MethodOption());
    
    /*
     * Read a message
     *
     * @param[out] data : The data to parse response into
     * @param[in] message_id : ID of message to read
     * @param[in] encoding (optional, default: "utf8") :  The encoding that should be used for the returned message
     * ["utf8"|"base64"|"hex"]
     *
     * @return true if no error has occured.
     *
     * @see ctn::ReadMessageResult
     *
     */
    void readMessage(ReadMessageResult &data, std::string message_id, std::string encoding = "utf8");
    
    /*
     * Retrieve message container
     *
     * @param[out] data : The data to parse response into
     * @param[in] message_id : ID of message to retrieve container info
     *
     * @return true if no error has occured.
     *
     * @see ctn::RetrieveMessageContainer
     *
     */
    void retrieveMessageContainer(RetrieveMessageContainerResult &data, std::string message_id);
    
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
     *
     * @see ctn::ListMessagesResult
     *
     */
    void listMessages(ListMessagesResult &data, std::string action = "any", std::string direction = "any", std::string from_device_ids = "", std::string to_device_ids = "", std::string from_device_prod_ids = "", std::string to_device_prod_ids = "", std::string read_state = "any", std::string start_date = "", std::string endDate = "");
    

    /*
     * Parse the server returned data and return it or throw CatenisAPIClientError.
     *
     * @param[out] userReturnData: The data object returned to the calling function.
     * @param[in] json_data: The data object returned from the server. 
     *
     */
    void parseLogMessage(LogMessageResult &user_return_data, std::string json_data);
    void parseSendMessage(SendMessageResult &user_return_data, std::string json_data);
    void parseReadMessage(ReadMessageResult &user_return_data, std::string json_data);
    void parseRetrieveMessageContainer(RetrieveMessageContainerResult &user_return_data, std::string json_data);
    void parseListMessages(ListMessagesResult &user_return_data, std::string json_data);
};

}


#endif  // __CATENISAPICLIENT_H__
