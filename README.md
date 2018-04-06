# Catenis API C++ Client

This C++ library is used to make it easier to access the Catenis Enterprise API services from C++ programs.

This current release (1.0.0) targets version 0.5 of the Catenis Enterprise API.

## Communication support library

This library is available in two varieties depending on the supporting library used for handling communication related 
operations: [Boost Asio](http://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio.html), or [Poco](https://pocoproject.org/docs/index.html).

* **NOTE**: the Boost Asio library is not used directly by the Catenis API C++ client library, but rather via the new [Boost
Beast](http://www.boost.org/doc/libs/1_66_0/libs/beast/doc/html/index.html) library, which provides support for both HTTP and WebSocket communication.

## Development

### Build prerequisites

* CMake (available [here](https://cmake.org/download/))
    - Version 3.0.0 or later.
* C++ compiler
    - macOS: [Xcode](https://developer.apple.com/xcode/)
        - Note: Xcode command line tools must be installed, which can be done by issuing the following command:
        ```shell
        xcode-select --install
        ```
    - Linux: [Gnu GCC](http://gcc.gnu.org/)
    - Windows: [Visual Studio](https://www.visualstudio.com/vs/community/)
* Perl (available [here](https://www.perl.org/get.html))
    - Perl is needed on WINDOWS for building the OpenSSL library

### External libraries

The C++ package manager [Hunter](https://github.com/ruslo/hunter) is used to download and build required external libraries.

You may set the environment variable ```HUNTER_ROOT``` to select where the downloaded external libraries will be stored.
If not specified, the default directory ```~/.hunter``` is used.

* **NOTE**: the path of the specified directory **MUST NOT** have spaces.

### Build steps

From the project's root directory, issue the following commands to build.

* **NOTE**: when building for the first time, it will take several minutes to download and build the external libraries. 

```shell
cmake -H. -B<build_dir> -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=<build_type> -DCOM_SUPPORT_LIB=<com_support_lib> -DBUILD_SAMPLES=<samples_opt>
cmake --build <build_dir> --config <build_type>
```
Where:
* ```<build_dir>``` - Designates a directory where the build outputs should be stored.
* ```<build_type>``` - Determines the type of build. Can be either ```Debug``` or ```Release```.
* ```<com_support_lib>``` - Selects the communication support library to use. Can be either ```BOOST_ASIO``` or ```POCO```.
* ```<samples_opt>``` - Controls whether the sample application (CmdSample) should be built. Can be either ```ON``` or ```OFF```.

The main product of the build is self-contained static library named CatenisAPIClient &mdash; the actual library filename
varies according to the target OS.

## Usage

Add the ```CatenisApiClient.h``` header file to your source code and link it with the CatenisAPIClient library.

* **NOTE**: when linking the library on Linux, the additional linker flags are required: 
    + ```-pthread``` 
    + ```-ldl```

### Instantiate the client

```cpp
#include "CatenisApiClient.h"

ctn::CtnApiClient ctnApiClient(device_id, api_access_secret, "catenis.io", "", "beta");
```

### Logging (storing) a message to the blockchain

Use the pre-created method option.

```cpp
// Define message options - encoding: utf8, encrypt: true, storage: embedded
ctn::MessageOptions msgOpts("utf8", true, "embedded");

// Define message string to record
std::string message = "This is only a test";

// Define structure to receive returned data
ctn::LogMessageResult data;

try {
    // Call the API method
    client.logMessage(data, message, msgOpts);
	cout << "ID of logged message:" << data.messageId << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Sending a message to another device

```cpp
// Define target virtual device
ctn::Device targetDevice("dnN3Ea43bhMTHtTvpytS")

// Define message string to send
std::string message = "This is only a test";

// Define message options - encoding: utf8, encrypt: true, storage: embedded, readConfirmation: true
ctn::MessageOptions msgOpts("utf8", true, "embedded", true);

// Define structure to receive returned data
ctn::SendMessageResult data;

try {
    // Call the API method
    client.sendMessage(data, message, targetDevice, msgOpts);
	cout << "Returned message ID: " << data.messageId << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Reading a message

```cpp
// Define message id to read
std::string message = "mDWPuD5kjCsEiNEEWwrW";

// Define structure to receive returned data
ctn::ReadMessageResult data;

try {
    // Call the API method
    client.readMessage(data, message_id, "utf8");

	if (data.from != nullptr) {
    cout << "  From: " << std::endl;
    cout << "    Device ID: " << data.from->deviceId << std::endl;

    if (!data.from->name.empty())
        cout << "    Name: " << data.from->name << std::endl;

    if (!data.from->prodUniqueId.empty())
        cout << "    Product unique ID: " << data.from->prodUniqueId << std::endl;
	}
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Retrieving information about a message's container

```cpp
// Define message id to read
std::string message = "mDWPuD5kjCsEiNEEWwrW";

// Define structure to receive returned data
ctn::RetrieveMessageContainerResult data;

try {
    // Call the API method
    client.retrieveMessageContainer(data, message_id);

    if (data.externalStorage != nullptr)
    {
        for (auto it = data.externalStorage->begin(); it != data.externalStorage->end(); it++)
        {
            cout << "  External storage (" << it->first << ") reference: " << it->second << std::endl;
        }
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### List messages

```cpp
// Define structure to receive returned data
ctn::ListMessagesResult data;

try {
    // Call the API method - list unread messages received since January 1st, 2018
    client.listMessages(data, "send", "inbound", "", "", "", "", "unread", "2018-01-01T00:00:00Z");

    for (auto it = data.messageList.begin(); it != data.messageList.end(); it++)
    {
        MessageDescription msgD = *(*it);
        cout << "--------------------------------Message";
        cout << "--------------------------------" << endl;
        cout << "MessageId\t\t\t: " << msgD.messageId << endl;
        cout << "Action\t\t\t\t: " << msgD.action << endl;

        if (!msgD.direction.empty())
            cout << "Direction\t\t\t: " << msgD.direction << endl;

        if (msgD.from != nullptr)
        {
            cout << "FromDeviceId\t\t\t: " << msgD.from->deviceId << endl;

            if (!msgD.from->name.empty())
                cout << "FromName\t\t\t: " << msgD.from->name << endl;

            if (!msgD.from->prodUniqueId.empty())
                cout << "FromProdUniqueId\t\t: " << msgD.from->prodUniqueId << endl;
        }

        if (msgD.to != nullptr)
        {
            cout << "ToDeviceId\t\t\t: " << msgD.to->deviceId << endl;

            if (!msgD.to->name.empty())
                cout << "ToName\t\t\t\t: " << msgD.to->name << endl;

            if (!msgD.to->prodUniqueId.empty())
                cout << "ToProdUniqueId\t\t\t: " << msgD.to->prodUniqueId << endl;
        }

        if (msgD.readConfirmationEnabled != nullptr)
            cout << "ReadConfirmationEnabled\t\t: " << *msgD.readConfirmationEnabled << endl;

        if (msgD.read != nullptr)
            cout << "Read\t\t\t\t: " << *msgD.read << endl;

        cout << "Date\t\t\t\t: " << msgD.date << endl;
    }
    cout << "MsgCount\t\t\t: " << data.msgCount << std::endl;
    cout << "CountExceeded\t\t\t: " << data.countExceeded << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### List permission events

```cpp
// Define structure to receive returned data
ctn::ListPermissionEventsResult data;

try {
    // Call the API method - Retrieves a list of all system defined permission events
    client.listPermissionEvents(data);

    for (auto it = data.permissionEvents.begin(); it != data.permissionEvents.end(); it++)
    {
        cout << "Permission event (" << it->first << "): " << it->second << std::endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Retrieve permission rights

```cpp
// Define the name of the permission event the permission rights for which should be retrieved
std::string eventName = "receive-msg";

// Define structure to receive returned data
ctn::RetrievePermissionRightsResult data;

try {
    // Call the API method - Retrieves a list of all system defined permission events
    client.retrievePermissionRights(data, eventName);

    /* SYSTEM LEVEL PERMISSION RIGHTS */
    cout << "System right: " << data.system << std::endl;

    /* CATENIS NODE PERMISSION RIGHTS */
    if (data.catenisNode != nullptr)
    {
        cout << "\nCatenis node rights:" << std::endl;

        std::list<string> allowed = data.catenisNode->allowed;

        if (allowed.size() > 0) {
            cout << "  Allowed Catenis nodes:" << endl;
            for (auto i = allowed.begin(); i != allowed.end(); ++i)
            {
                cout << "    " << *i << endl;
            }
        }

        std::list<string> denied = data.catenisNode->denied;
        if (denied.size() > 0) {
            cout << "  Denied Catenis nodes:" << endl;
            for (auto i = denied.begin(); i != denied.end(); ++i)
            {
                cout << "    " << *i << endl;
            }
        }
    }

    /* CLIENT NODE PERMISSION RIGHTS */
    if (data.client != nullptr)
    {
        cout << "\nClient rights:" << std::endl;

        std::list<string> allowed = data.client->allowed;

        if (allowed.size() > 0) {
            cout << "  Allowed clients:" << endl;
            for (auto i = allowed.begin(); i != allowed.end(); ++i)
            {
                cout << "    " << *i << endl;
            }
        }

        std::list<string> denied = data.client->denied;
        if (denied.size() > 0) {
            cout << "  Denied clients:" << endl;
            for (auto i = denied.begin(); i != denied.end(); ++i)
            {
                cout << "    " << *i << endl;
            }
        }
    }

    /* DEVICE LEVEL PERMISSION RIGHTS */
    if (data.device != nullptr)
    {
        cout << "\nDevice rights:" << std::endl;

        std::list<std::shared_ptr<DeviceInfo>> allowed = data.device->allowed;
        if (allowed.size() > 0) {
            cout << "  Allowed devices:" << endl;
            int devCount = 0;
            for (auto it = allowed.begin(); it != allowed.end(); it++)
            {
                cout << "    Device #" << ++devCount << ":" << endl;
                DeviceInfo okDev = *(*it);
                cout << "      deviceId: " << okDev.deviceId << endl;
                if (!okDev.name.empty())
                    cout << "      name: " << okDev.name << endl;
                if (!okDev.prodUniqueId.empty())
                    cout << "      prodUniqueId: " << okDev.prodUniqueId << endl;
            }
        }

        std::list<std::shared_ptr<DeviceInfo>> denied = data.device->denied;
        if (denied.size() > 0) {
            cout << "  Denied devices:" << endl;
            int devCount = 0;
            for (auto it = denied.begin(); it != denied.end(); it++)
            {
                cout << "    Device #" << ++devCount << ":" << endl;
                DeviceInfo ngDev = *(*it);
                cout << "      deviceId: " << ngDev.deviceId << endl;
                if (!ngDev.name.empty())
                    cout << "      name: " << ngDev.name << endl;
                if (!ngDev.prodUniqueId.empty())
                    cout << "      prodUniqueId: " << ngDev.prodUniqueId << endl;
            }
        }
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Set permission rights

```cpp
// Define the name of the permission event for which the permission rights should be set.
std::string eventName = "receive-msg";

// Set permission right to be set at the System Level
std::string systemRight = "allow";

// Define lists of catenis node indices identifying catenis nodes to which allow right should be given, denied, or removed
std::list<string> allowedCtnNodes;
std::list<string> deniedCtnNodes;
std::list<string> noneCtnNodes;

// Define lists of client IDs identifying clients to which allow right should be given, denied, or removed
std::list<string> allowedClients;
std::list<string> deniedClients;
std::list<string> noneClients;

// Define lists of virtual device info objects identifying virtual devices to which allow right should be given, denied, or removed.
std::list<Device> allowedDevices;
std::list<Device> deniedDevices;
std::list<Device> noneDevices;

// Insert catenis node indices, client IDs, and device info objects into respective arrays
allowedClients.push_back("self");
deniedClients.push_back("cjNhuvGMUYoepFcRZadP");
allowedDevices.push_back(Device("dv3htgvK7hjnKx3617Re", false));
allowedDevices.push_back(Device("XYZ0001", true));

// Define structure for permission rights to be set at the Catenis Node Level
ctn::SetRightsCtnNode ctnNodeRights(allowedCtnNodes,deniedCtnNodes,noneCtnNodes);

// Define structure for permission rights to be set at the Client Level
ctn::SetRightsClient ClientsRights(allowedClients, deniedClients, noneClients);

// Define structure for permission rights to be set at the Device Level
ctn::SetRightsDevice deviceRights(allowedDevices, deniedDevices, noneDevices);

try {
    // Call the API method - Sets permission rights at different levels for a specific permission event.
    client.client.setPermissionRights(data, eventName, systemRight, &ctnNodeRights, &clientRights, &deviceRights);
	cout << "Permission rights successfully set" << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Check effective permission rights

```cpp
// Define the name of the permission event the permission right set for which should be retrieved.
std::string eventName = "receive-msg";

// Define the ID of the virtual device the permission right applied to which should be retrieved.
std::string deviceId  = "dv3htgvK7hjnKx3617Re";

// Indicate whether the supplied ID is a product unique ID.
bool isProdUniqueId   = false;

// Define structure to receive returned data
ctn::CheckEffectivePermissionRightResult data;

try {
    // Call the API method -Checks the effective permission right that is currently applied to a given virtual device for a specified permission event.
    client.checkEffectivePermissionRight(data, eventName, Device(deviceId, isProdUniqueId));

    for (auto it = data.effectivePermissionRight.begin(); it != data.effectivePermissionRight.end(); it++)
    {
        cout << "Effective right for device (\"" << it->first << "\"): " << it->second << std::endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Retrieve device identification info

```cpp
// Define the ID of the virtual device the permission right applied to which should be retrieved.
std::string deviceId  = "dv3htgvK7hjnKx3617Re";

// Indicate whether the supplied ID is a product unique ID.
bool isProdUniqueId   = false;

// Define structure to receive returned data
ctn::DeviceIdInfoResult data;

try {
    // Call the API method - Gets the identification information of a given virtual device.
    client.retrieveDeviceIdInfo(data, Device(deviceId, isProdUniqueId));

    // Print out CatenisNodeInfo
    if (data.catenisNode != nullptr)
    {
		cout << "Device\'s Catenis node ID info:" << std::endl;
        cout << "  index: " << data.catenisNode->index << endl;

        if (!data.catenisNode->name.empty())
			cout << "  name: " << data.catenisNode->name << endl;

        if (!data.catenisNode->description.empty())
			cout << "  description: " << data.catenisNode->description << endl;
    }

    // Print out ClientInfo
    if (data.client != nullptr)
    {
        cout << "\nDevice\'s client ID info:" << std::endl;
        cout << "  clientId: " << data.client->clientId << endl;

        if (!data.device->name.empty())
            cout << "  name: " << data.client->name << endl;
    }

    // Print out DeviceInfo
    if (data.device != nullptr)
    {
        cout << "\nDevice\'s own ID info:" << endl;
        cout << "  deviceId: " << data.device->deviceId << endl;

        if (!data.device->name.empty())
            cout << "  name: " << data.device->name << endl;

        if (!data.device->prodUniqueId.empty())
            cout << "  prodUniqueId: " << data.device->prodUniqueId << endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### List notification events

```cpp
// Define structure to receive returned data
ctn::ListNotificationEventsResult data;

try {
    // Call the API method - Retrieves a list of all system defined permission events
    client.listNotificationEvents(data);

    for (auto it = data.notificationEvents.begin(); it != data.notificationEvents.end(); it++)
    {
        cout << "Notification event (" << it->first << "): " << it->second << std::endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

## Error handling

Two types of error can take place when calling API methods: client or API error.

They can be differentiated by the type of object thrown.

#### Client error

```cpp
class CatenisClientError : public CatenisAPIException {
    std::string getErrorMessage();
    std::string getErrorDescription();
}
```

The ```getErrorMessage()``` method can be used to retrieve the associated error message, whilst the ```getErrorDescription()```
method can be used to get a complete error description.

#### API error

```cpp
class CatenisAPIError : public CatenisAPIException {
    int getHttpStatusCode();
    std::string getErrorMessage();
    std::string getErrorDescription();
}
```

It has one additional method, ```getHttpStatusCode()```, which can be used to get the returned HTTP status code.

## License

This C++ library is released under the [MIT License](LICENSE). Feel free to fork, and modify!

Copyright © 2018, Blockchain of Things Inc.
