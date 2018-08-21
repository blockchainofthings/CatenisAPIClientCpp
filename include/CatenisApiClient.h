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
#include <memory>

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
    std::string id;
    bool isProdUniqueId;
    
    Device(std::string device_id, bool is_prod_uniqueid = false)
        : id(device_id), isProdUniqueId(is_prod_uniqueid) {}
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

// Dictionary holding permission event description by permission event name
typedef std::map<std::string, std::string> PermissionEventDictionary;

/*
* List Permission Events API method response structure
*
* @member permissionEvents : The permission events
*/
struct ListPermissionEventsResult
{
    PermissionEventDictionary permissionEvents;
};


/*
* Permission rights at device level structure
*
* @member allowed : list of allowed virtual devices
* @member denied  : list of denied virtual devices
*
*/
struct PermissionRightsDevice
{
    std::list< std::shared_ptr<DeviceInfo> > allowed;
    std::list< std::shared_ptr<DeviceInfo> > denied;

    PermissionRightsDevice(
        std::list< std::shared_ptr<DeviceInfo> > allowedDevices,
        std::list< std::shared_ptr<DeviceInfo> > deniedDevices)
        : allowed(allowedDevices), denied(deniedDevices) {}
    ~PermissionRightsDevice() {}
};

/*
* Permission rights at Catenis node level structure
*
* @member allowed : List of allowed Catenis Nodes
* @member denied : List of denied Catenis Nodes
*
*/
struct PermissionRightsCatenisNode
{
    std::list<std::string> allowed;
    std::list<std::string> denied;

    PermissionRightsCatenisNode(
        std::list<std::string> allowedRights,
        std::list<std::string> deniedRights)
        : allowed(allowedRights), denied(deniedRights) {}
    ~PermissionRightsCatenisNode() {}
};

/*
* Permission rights at client level structure
*
* @member allowed : List of allowed clients
* @member denied : List of denied clients
*
*/
struct PermissionRightsClient
{
    std::list<std::string> allowed;
    std::list<std::string> denied;

    PermissionRightsClient(
        std::list<std::string> allowedRights,
        std::list<std::string> deniedRights)
        : allowed(allowedRights), denied(deniedRights) {}
    ~PermissionRightsClient() {}
};

/*
* Retrieve Permission Rights API method response structure
*
* @member system : Permission right set at the system level.
* @member catenisNode : Permission rights set at catenisNodes level
* @member client : Permission rights set at client level
* @member device : Permission rights set at device level
*/
struct RetrievePermissionRightsResult
{
    std::string system;
    std::shared_ptr<PermissionRightsCatenisNode> catenisNode;
    std::shared_ptr<PermissionRightsClient> client;
    std::shared_ptr<PermissionRightsDevice> device;
};

/*
* Set Permission Rights at Device Level structure (Array of Objects)
*
* @member allowed : Object structure of allowed virtual devices
* @member denied  : Object structure of denied virtual devices
* @member none    : Object structure of virtual devices for which Rights should be removed
*
*/
struct SetRightsDevice
{
    std::list<Device> allowed;
    std::list<Device> denied;
    std::list<Device> none;
};

/*
* Set Permission Rights Catenis Node Level structure
*
* @member allowed : List of allowed Catenis Nodes
* @member denied : List of denied Catenis Nodes
* @member none : List of Catenis Nodes for which Rights should be removed
*
*/
struct SetRightsCtnNode
{
    std::list<std::string> allowed;
    std::list<std::string> denied;
    std::list<std::string> none;
};

/*
* Set Permission Rights Client Level structure
*
* @member allowed : List of allowed clients
* @member denied : List of denied clients
* @member none : List of clients for which rights should be removed
*
*/
struct SetRightsClient
{
    std::list<std::string> allowed;
    std::list<std::string> denied;
    std::list<std::string> none;
};

// Dictionary holding Set Permission Rights description result
typedef std::map<std::string, std::string> SetPermissionRightsDictionary;

/*
* Set Permission Rights API method response structure
*
* @member status : The result of the 
*/
struct SetPermissionRightsResult
{
    bool success;
};

// Dictionary holding notification event description by notification event name
typedef std::map<std::string, std::string> NotificationEventDictionary;

/*
* List Notification Events API method response structure
*
* @member notificationEvents : The notification events
*/
struct ListNotificationEventsResult
{
    NotificationEventDictionary notificationEvents;
};

// Dictionary holding effective permission right description by check_device_ID
typedef std::map<std::string, std::string> EffectivePermissionRightDictionary;

/*
* Check Effective Permission Right API method response structure
*
* @member Effective Permission Right : Permission right for the specified device
*/
struct CheckEffectivePermissionRightResult
{
    EffectivePermissionRightDictionary effectivePermissionRight;
};

/*
* Catenis Node info structure
*
* @member index :  index of the Catenis node.
* @member name  :  name of the Catenis node.
* @member description  :  short description about the Catenis node.
*/
struct CatenisNodeInfo
{
    int index;
    std::string name;
    std::string description;

    CatenisNodeInfo(int node_index, std::string node_name, std::string node_description)
        : index(node_index), name(node_name), description(node_description) {}
    ~CatenisNodeInfo() {}
};

/*
* Device's Client info structure
*
* @member clientId    :  client id
* @member clientName  :  client name
*/
struct ClientInfo
{
    std::string clientId;
    std::string name;

    ClientInfo(std::string client_id, std::string client_name)
        : clientId(client_id), name(client_name) {}
    ~ClientInfo() {}
};

/*
* Retrieve Device Identification Info API method response structure
*
* @member catenisNode : Information about the Catenis node where the client to which the specified virtual device belongs is defined.
* @member catenisNode : Information about the client to which the specified virtual device belongs.
* @member catenisNode : Information about the specified virtual device itself.
*/
struct DeviceIdInfoResult
{
    std::shared_ptr<CatenisNodeInfo> catenisNode;
    std::shared_ptr<ClientInfo> client;
    std::shared_ptr<DeviceInfo> device;
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
     * ["prod"|"sandbox"]
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
     * @see ctn::ListMessagesResult
     *
     */
    void listMessages(ListMessagesResult &data, std::string action = "any", std::string direction = "any", std::string from_device_ids = "", std::string to_device_ids = "", std::string from_device_prod_ids = "", std::string to_device_prod_ids = "", std::string read_state = "any", std::string start_date = "", std::string endDate = "");

    /*
    * List Permission Events
    *
    * @param[out] data : The data to parse response into
    *
    * @see ctn::ListPermissionEventsResult
    *
    */
    void listPermissionEvents(ListPermissionEventsResult &data);

    /*
    * Retrieve Permission Rights
    *
    * @param[out] data : The data to parse response into
    *
    * @param[in] eventName : Name of the permission event to lookup
    *
    * @see ctn::RetrievePermissionRightsResult
    *
    */
    void retrievePermissionRights(RetrievePermissionRightsResult &data, std::string eventName);

    /*
    * Set Permission Rights
    *
    * @param[out] data : The data to parse response into
    *
    * @param[in] eventName : Name of the permission event to lookup
    * @param[in] systemRight : The permission right at the system level to set
    * @param[in] cntNodesRights : The permission rights at the Catenis node level to set
    * @param[in] clientRights : The permission rights at the client level to set
    * @param[in] deviceRights : The permission rights at the device level to set
    *
    * @see ctn::SetPermissionRightsResult
    *
    */
    void setPermissionRights(SetPermissionRightsResult &data, std::string eventName, std::string systemRight, SetRightsCtnNode *cntNodesRights, SetRightsClient *clientRights, SetRightsDevice *deviceRights);

    /*
    * List Notification Events
    *
    * @param[out] data : The data to parse response into
    *
    * @return true if no error has occurred.
    *
    * @see ctn::ListNotificationEventsResult
    *
    */
    void listNotificationEvents(ListNotificationEventsResult &data);

    /*
    * Check Effective Permission Right
    *
    * @param[out] data : The data to parse response into
    *
    * @param[in] event name : Name of the permission event to lookup
    *
    * @param[in] deviceId   : ID of the virtual device the permission right applied to which should be retrieved.
    *
    * @param[in] isProdUniqueId   : Flag indicating whether the supplied ID is a product unique ID (false as default)
    *
    * @return true if no error has occurred.
    *
    * @see ctn::CheckEffectivePermissionRightResult
    *
    */
    void checkEffectivePermissionRight(CheckEffectivePermissionRightResult &data, std::string eventName, Device device);

    /*
    * Retrieve Device Identification Info
    *
    * @param[out] data : The data to parse response into
    *
    * @param[in] deviceId   : ID of the virtual device the permission right applied to which should be retrieved.
    *
    * @param[in] isProdUniqueId   : Flag indicating whether the supplied ID is a product unique ID (false as default)
    *
    * @return true if no error has occurred.
    *
    * @see ctn::DeviceIdInfoResult
    *
    */
    void retrieveDeviceIdInfo(DeviceIdInfoResult &data, Device device);
};

}

#endif  // __CATENISAPICLIENT_H__
