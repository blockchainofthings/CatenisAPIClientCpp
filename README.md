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

```cpp
// Define message options - encoding: utf8, encrypt: true, storage: embedded
ctn::MessageOptions msgOpts("utf8", true, "embedded");

// Define structure to receive returned data
ctn::LogMessageResult data;

try {
    // Call the API method
    ctnApiClient.logMessage(data, "My message", msgOpts);
    
    std::cout << "ID of logged message: " << data.messageId << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Sending a message to another device

```cpp
// Define target virtual device
ctn::Device targetDevice(target_device_id)

// Define message options - encoding: utf8, encrypt: true, storage: embedded, readConfirmation: true
ctn::MessageOptions msgOpts("utf8", true, "embedded", true);

// Define structure to receive returned data
ctn::SendMessageResult data;

try {
    // Call the API method
    ctnApiClient.sendMessage(data, targetDevice, "My message to send", msgOpts);
    
    std::cout << "ID of sent message: " << data.messageId << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Reading a message

```cpp
// Define structure to receive returned data
ctn::ReadMessageResult data;

try {
    // Call the API method
    ctnApiClient.readMessage(data, message_id, "utf8");
    
    std::cout << "Message read: " << data.message << endl;

    if (data.action == "send"} {
        std::cout << "Message originally from: device ID: " << data.from->deviceId
            << (!data.from->name.empty() ? std::string(", name: ") + data.from->name : std::string(""))
            << (!data.from->prodUniqueId.empty() ? std::string(", product unique ID: ") + data.from->prodUniqueId : std::string(""));
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Retrieving information about a message's container

```cpp
// Define structure to receive returned data
ctn::RetrieveMessageContainerResult data;

try {
    // Call the API method
    ctnApiClient.retrieveMessageContainer(data, message_id);
    
    std::cout << "ID of blockchain transaction containing the message: " << data.blockchain.txid << endl;

    if (data.externalStorage != nullptr) {
       std::cout << "IPFS reference to message: " << (*data.externalStorage)["ipfs"] << endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Listing messages

```cpp
// Define structure to receive returned data
ctn::ListMessagesResult data;

try {
    // Call the API method - list unread messages received since January 1st, 2018
    ctnApiClient.listMessages(data, "send", "inbound", "", "", "", "", "unread", "2018-01-01T00:00:00Z");

    int msgCounter = 0;

    for (auto it = data.messageList.begin(); it != data.messageList.end(); it++) {
        std::cout << "Message #" << ++msgCounter << ":" << std::endl;

        std::shared_ptr<ctn::MessageDescription> msgDesc = *it;

        std::cout << "  message ID: " << msgDesc->messageId << std::endl;
        std::cout << "  action: " << msgDesc->action << std::endl;

        if (msgDesc->action == "send") {
            std::cout << "  direction: " << msgDesc->direction << std::endl;

            if (msgDesc->direction == "inbound") {
                std::cout << "  from device ID: " << msgDesc->from->deviceId << std::endl;
            }

            if (msgDesc->direction == "outbound") {
                std::cout << "  to device ID: " << msgDesc->to->deviceId << std::endl;
            }

            if (msgDesc->readConfirmationEnabled != nullptr) {
                std::cout << "  read confirmation enabled: " << (*(msgDesc->readConfirmationEnabled) ? "true" : "false") << std::endl;
            }
        }

        if (msgDesc->read != nullptr) {
            std::cout << "  message read: " << (*(msgDesc->read) ? "true" : "false") << std::endl;
        }

        std::cout << "  Date " << (msgDesc->action == "log" ? "logged: " : (msgDesc->direction == "inbound" ? "received: " : "sent: ")) << msgDesc->date << std::endl;
    }

    if (data.countExceeded) {
        std::cout << "Warning: not all messages fulfilling search criteria have been returned!" << std::endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Listing system defined permission events

```cpp
// Define structure to receive returned data
ctn::ListPermissionEventsResult data;

try {
    // Call the API method
    ctnApiClient.listPermissionEvents(data);

    for (auto it = data.permissionEvents.begin(); it != data.permissionEvents.end(); it++) {
        std::cout << "Event name: " << it->first << "; event description: " << it->second << std::endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Retrieving permission rights currently set for a specified permission event

```cpp
// Define structure to receive returned data
ctn::RetrievePermissionRightsResult data;

try {
    // Call the API method
    ctnApiClient.retrievePermissionRights(data, "receive-msg");

    std::cout << "Default (system) permission right: " << data.system << std::endl;

    if (data.catenisNode != nullptr) {
        if (data.catenisNode->allowed.size() > 0) {
            std::cout << "Index of Catenis nodes with 'allow' permission right:" << std::endl;
            
            for (auto i = data.catenisNode->allowed.begin(); i != data.catenisNode->allowed.end(); ++i) {
                std::cout << "  - " << *i << std::endl;
            }
        }

        if (data.catenisNode->denied.size() > 0) {
            std::cout << "Index of Catenis nodes with 'deny' permission right:" << std::endl;
            
            for (auto i = data.catenisNode->denied.begin(); i != data.catenisNode->denied.end(); ++i) {
                std::cout << "  - " << *i << std::endl;
            }
        }
    }

    if (data.client != nullptr) {
        if (data.client->allowed.size() > 0) {
            std::cout << "ID of clients with 'allow' permission right:" << std::endl;
            
            for (auto i = data.client->allowed.begin(); i != data.client->allowed.end(); ++i) {
                std::cout << "  - " << *i << std::endl;
            }
        }

        if (data.client->denied.size() > 0) {
            std::cout << "ID of clients with 'deny' permission right:" << std::endl;
            
            for (auto i = data.client->denied.begin(); i != data.client->denied.end(); ++i) {
                std::cout << "  - " << *i << std::endl;
            }
        }
    }

    if (data.device != nullptr) {
        if (data.device->allowed.size() > 0) {
            std::cout << "ID of devices with 'allow' permission right:" << std::endl;
            
            for (auto i = data.device->allowed.begin(); i != data.device->allowed.end(); ++i) {
                std::cout << "  - " << (*i)->deviceId << std::endl;
            }
        }

        if (data.device->denied.size() > 0) {
            std::cout << "ID of devices with 'deny' permission right:" << std::endl;
            
            for (auto i = data.device->denied.begin(); i != data.device->denied.end(); ++i) {
                std::cout << "  - " << (*i)->deviceId << std::endl;
            }
        }
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Setting permission rights at different levels for a specified permission event

```cpp
ctn::SetRightsCtnNode ctnNodeRights;
ctnNodeRights.allowed.push_back(std::string("self"));

ctn::SetRightsClient clientRights;
clientRights.allowed.push_back(std::string("self"));
clientRights.allowed.push_back(std::string(clientId));

ctn::SetRightsDevice deviceRights;
deviceRights.denied.push_back(ctn::Device(deviceId));
deviceRights.denied.push_back(ctn::Device("ABCD001", true));

try {
    // Call the API method
    ctnApiClient.setPermissionRights(data, "receive-msg", "deny", &ctnNodeRights, &clientRights, &deviceRights);
    
    std::cout << "Permission rights successfully set" << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Checking effective permission right applied to a given device for a specified permission event

```cpp
// Define structure to receive returned data
ctn::CheckEffectivePermissionRightResult data;

try {
    // Call the API method
    ctnApiClient.checkEffectivePermissionRight(data, "receive-msg", ctn::Device(deviceId));

    std::cout << "Effective right for device " << data.effectivePermissionRight.begin()->first << ": " << data.effectivePermissionRight.begin()->second << std::endl;
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Retrieving identification information of a given device

```cpp
// Define structure to receive returned data
ctn::DeviceIdInfoResult data;

try {
    // Call the API method
    ctnApiClient.retrieveDeviceIdInfo(data, ctn::Device(deviceId));

    std::cout << "Device\'s Catenis node info:" << std::endl;
    std::cout << "  index: " << data.catenisNode->index << std::endl;

    if (!data.catenisNode->name.empty()) {
        std::cout << "  name: " << data.catenisNode->name << std::endl;
    }

    if (!data.catenisNode->description.empty()) {
        std::cout << "  description: " << data.catenisNode->description << std::endl;
    }

    std::cout << "Device\'s client info:" << std::endl;
    std::cout << "  clientId: " << data.client->clientId << std::endl;

    if (!data.device->name.empty()) {
        std::cout << "  name: " << data.client->name << std::endl;
    }

    std::cout << "Device\'s own info:" << std::endl;
    std::cout << "  deviceId: " << data.device->deviceId << std::endl;

    if (!data.device->name.empty()) {
        std::cout << "  name: " << data.device->name << std::endl;
    }

    if (!data.device->prodUniqueId.empty()) {
        std::cout << "  prodUniqueId: " << data.device->prodUniqueId << std::endl;
    }
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Listing system defined notification events

```cpp
// Define structure to receive returned data
ctn::ListNotificationEventsResult data;

try {
    // Call the API method
    ctnApiClient.listNotificationEvents(data);

    for (auto it = data.notificationEvents.begin(); it != data.notificationEvents.end(); it++) {
        std::cout << "Event name: " << it->first << "; event description: " << it->second << std::endl;
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
