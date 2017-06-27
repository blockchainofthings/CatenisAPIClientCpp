# Catenis API C++ Client
This library lets C++ developers access the Catenis API services with ease. The current installation has been tested on Mac, Linux, and Windows systems.

## Available Branches
1. boostAsio : Uses the Boost.Asio network library to call API methods.
    - Recommended for projects that require low level I/O programming 
    (More flexibility but complicated)
2. poco : Uses the Poco network library to call API methods. **(Under Development in master branch)**
    - Recommended for normal projects that only uses the standard functionalities of the Catenis API Methods 
    (More reliable and easy to use)re low level I/O programming (More flexibility but complicated)

## Building

### Prerequisites:

* CMake (Available from https://cmake.org/download/)
    - Verified with versions 3.0.0 and up
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

## Contributers

Sungwoo Bae - sungwoo@blockchainofthings.com || sungwoo.bae@columbia.edu
