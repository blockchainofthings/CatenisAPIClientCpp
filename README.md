# Catenis API C++ Client: Boost Asio
This library lets C++ developers access the Catenis API services with ease. The current installation has been tested on Mac, Linux, and Windows systems.

## Available Branches
1. poco : Uses the Poco network library to call API methods.
    - Recommended for normal projects that only use the standard functionalities of the Catenis API Methods 
    (More reliable and easy to use)
2. boostAsio : Uses the Boost.Asio network library to call API methods.
    - Recommended for projects that require low level I/O programming 
    (More flexibility but complicated)

## Building

### Prerequisites:

* CMake (Available from https://cmake.org/download/)
    - Required versions 3.0.0 and up
* C++ compiler
    - Mac OSX https://developer.apple.com/xcode/
        - Must install Xcode command line tools afterwards by running
        ```shell
        xcode-select --install
        ```
    - Linux   http://gcc.gnu.org/
    - Windows https://www.visualstudio.com/vs/community/
* Perl (Available from https://www.perl.org/get.html)
    - Perl is needed for building the OpenSSL library

### Setting External Library Directory:

This library uses the C++ package manager [Hunter](https://github.com/ruslo/hunter) to download and build required external libraries. 
You may set the environment variable HUNTER_ROOT to direct where the external libraries will be downloaded.
If not specified, the external libraries will be downloaded in ~/.hunter
* If the directory is specified, the path MUST NOT have spaces.

### Build Steps:

From the project's root directory, issue the following commands to build.
Note that when building for the first time, it will take around 10 minutes to download and build the external libraries. 

```shell
cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Release
cmake --build _builds --config Release
```

All build files will be located in the _builds directory.

#### Building Sample Programs:

A command line sample program is located within the samples folder. To build this with the library, issue a command similar to the previous one but with the BUILD_SAMPLES flag.

```shell
cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON
cmake --build _builds --config Release
```

## Usage

Link the built static library and include the header.

```shell
#include <CatenisApiClient.h>
```

Please read into the header file for more detailed usage information.

### Instantiate the client

```shell
ctn::CtnApiClient ctnApiClient(device_id, api_access_secret, "catenis.io", "", "beta");
```

### Set method options and declare response data

```shell
ctn::MethodOption options("utf8", true, "auto");
std::string response_data; // this is where the http response will be stored
```

### Logging (storing) a message to the blockchain

Use the pre-created method option.

```shell
ctnApiClient.logMessage(message, response_data, options);
```

Or contruct one within the method call.

```shell
ctnApiClient.logMessage(message, response_data, ctn::MethodOption("utf8", true, "auto"));
```

### Sending a message to another device

```shell
ctn::Device device(device_id, false);
ctnApiClient.sendMessage(device, message, response_data, options);
```

You may also contruct the device within the method call.

```shell
ctnApiClient.sendMessage(ctn::Device(device_id), message, response_data, options);
```

### Reading a message

```shell
ctnApiClient.readMessage(message_id, response_data, "utf8")
```

### Retrieving information about a message's container

```shell
ctnApiClient.retrieveMessageContainer(message_id, response_data);
```

### List messages

```shell
ctnApiClient.listMessages(response_data);
```

## License

This C++ library is released under the [MIT License](LICENSE). Feel free to fork, and modify!

Copyright © 2017, Blockchain of Things Inc.
