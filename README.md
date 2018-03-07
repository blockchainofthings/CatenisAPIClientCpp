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

// Define structure to receive returned data
ctn::LogMessageResult data;

try {
    // Call the API method
    client.logMessage(data, message, msgOpts);
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Sending a message to another device

```shell
// Define target virtual device
ctn::Device targetDevice(target_device_id)

// Define message options - encoding: utf8, encrypt: true, storage: embedded, readConfirmation: true
ctn::MessageOptions msgOpts("utf8", true, "embedded", true);

// Define structure to receive returned data
ctn::SendMessageResult data;

try {
    // Call the API method
    client.sendMessage(data, message, targetDevice, msgOpts);
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Reading a message

```shell
// Define structure to receive returned data
ctn::ReadMessageResult data;

try {
    // Call the API method
    client.readMessage(data, message_id, "utf8");
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### Retrieving information about a message's container

```shell
// Define structure to receive returned data
ctn::RetrieveMessageContainerResult data;

try {
    // Call the API method
    client.retrieveMessageContainer(data, message_id);
}
catch (ctn::CatenisAPIException &errObject) {
    std::cerr << errObject.getErrorDescription() << std::endl;
}
```

### List messages

```shell
// Define structure to receive returned data
ctn::ListMessagesResult data;

try {
    // Call the API method - list unread messages received since January 1st, 2018
    client.listMessages(data, "send", "inbound", "", "", "", "", "unread", "2018-01-01T00:00:00Z");
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
