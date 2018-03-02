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
#include <map>

// Version specific constants
const std::string DEFAULT_API_VERSION = "0.5";

namespace ctn
{
    
/*
 * Message options structure
 *
 * @member encoding : Value identifying the encoding of the message
 * ["utf8"|"base64"|"hex"]
 * @member encrypt : Indicates whether message should be encrypted before storing it
 * @member storage : Value identifying where the message should be stored
 * [“auto"|"embedded"|"external"]
 */
struct MessageOptions
{
    std::string encoding;
    bool encrypt;
    std::string storage;
    bool readConfirmation;
    
    // Default constructor with default values for members
    MessageOptions()
    {
        encoding = "utf8";
        encrypt = true;
        storage = "auto";
        readConfirmation = false;
    }

    MessageOptions(std::string encoding, bool encrypt, std::string storage, bool read_confirmation = false)
    {
        this->encoding = encoding;
        this->encrypt = encrypt;
        this->storage = storage;
        this->readConfirmation = read_confirmation;
    }
};
    
/*
 * Catenis virtual device structure
 *
 * @member id : ID of device. Should be Catenis device ID unless isProdUniqueId is true
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
 * Log Message API method response structure
 *
 * @member messageId : ID of logged message.
 */
struct LogMessageResult
{
	std::string messageId;
};

/*
 * Send Message API method response structure
 *
 * @member messageId : ID of sent message.
 */
struct SendMessageResult
{
	std::string messageId;
};

/*
 * Device info structure
 *
 * @member deviceId : Catenis ID of device.
 * @member name : Device name.
 * @member prodUniqueId : Device's product unique ID.
 */

struct DeviceInfo
{
    std::string deviceId;
    std::string name;
    std::string prodUniqueId;

    DeviceInfo(std::string device_id, std::string device_name, std::string prod_unique_id)
        : deviceId(device_id), name(device_name), prodUniqueId(prod_unique_id) {}
    ~DeviceInfo() {}
};

/*
 * Read Message API method response structure
 *
 * @member action : the action performed on the message: 'log' or 'send'.
 * @member from : Catenis ID/Name/ProdUniqueId of the origin device.
 * @member message : the message read.
 */
struct ReadMessageResult
{
	std::string action;
    std::shared_ptr<DeviceInfo> from;
	std::string message;
};

/*
 * Blockchain transaction info structure
 *
 * @member txid : ID of blockchain transaction.
 * @member isConfirmed : Indicates whether transaction is confirmed.
 */
struct TransactionInfo
{
    std::string txid;
    bool isConfirmed;
};

// Dictionary holding external storage reference by storage provider name
typedef std::map<std::string, std::string> StorageProviderDictionary;

/*
 * Read Message API method response structure
 *
 * @member blockchain: TXID of blockchain transaction where the message is recorded.
 * @member externalStorage : [optional] only returned if message is stored in an external storage.
 * @member storageProviderName : Key: storage provider name.
 */
struct RetrieveMessageContainerResult
{
    TransactionInfo blockchain;
    std::shared_ptr<StorageProviderDictionary> externalStorage;
};

/*
 * Message description structure
 *
 * @member messageId : ID of message.
 * @member action : Action performed: 'log' or 'send'.
 * @member direction : Direction of 'send' message: 'inbound' or 'outbound'.
 * @member from : Catenis ID/Name/ProdUniqueId of the sending device.
 * @member to : Catenis ID/Name/ProdUniqueId of the target device.
 * @member readConfirmationEnabled : Indicates whether the message had been sent with read-confirmation enabled.
 * @member read : Indicates whether the message had already been read.
 * @member date : ISO 8601 formatted date and time when message was logged, sent or received.
 */
struct MessageDescription
{
    std::string messageId;
    std::string action;
    std::string direction;
    std::shared_ptr<DeviceInfo> from;
    std::shared_ptr<DeviceInfo> to;
    std::shared_ptr<bool> readConfirmationEnabled;
    std::shared_ptr<bool> read;
    std::string date;

    MessageDescription(std::string message_id, std::string action_arg, std::string direction_arg, 
        std::shared_ptr<DeviceInfo> from_arg, std::shared_ptr<DeviceInfo> to_arg,
        std::shared_ptr<bool> &read_confirmation_enabled, std::shared_ptr<bool> read_arg, std::string date_arg)
            : messageId(message_id), action(action_arg), direction(direction_arg), from(from_arg), to(to_arg),
            readConfirmationEnabled(read_confirmation_enabled), read(read_arg), date(date_arg) {}
    ~MessageDescription() {}
};

/*
 * List Messages API method response structure
 *
 * @member messageList : List of structures.
 * @member msgCount : Number of messages for which information is returned.
 * @member countExceeded : Was the actual number of messages greater than the max returnable.
 */
struct ListMessagesResult
{
    std::list< std::shared_ptr<MessageDescription> > messageList; 
	int msgCount;
	bool countExceeded;
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
     * @see ctn::MessageOptions
     */
    void logMessage(LogMessageResult &data, std::string message, const MessageOptions &option = MessageOptions());
    
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
     * @see ctn::MessageOptions
     */
    void sendMessage(SendMessageResult &data, const Device &device, std::string message, const MessageOptions &option = MessageOptions());
    
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
};

}


#endif  // __CATENISAPICLIENT_H__
