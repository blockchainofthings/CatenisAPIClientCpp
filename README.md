# Catenis API C++ Client
This library is used to make it easier to access the Catenis API services for C++ developers.
This is the initial version made by Sungwoo Bae || sungwoo@blockchainofthings.com || swbae31@gmail.com

## Development

### Build

From the project's root directory, issue the following commands (it is assumed that Cmake is installed) to build.

```shell
cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Release
cmake --build _builds --config Release
```

All build files will be located in the _builds directory. An execute file CatenisAPIClient will be created (will later change build commands to create static library file).

## Usage

To test code:

```shell
cd _builds
./CatenisAPIClient
```

