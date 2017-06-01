/**
 * @author Sungwoo Bae
 * @createdAt 25/05/2017
 */
#include <CatenisApiClient.h>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <iterator>
#include <map>

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::asio::ip::tcp;

int main()
{
    ctn::CtnApiClient *client = new ctn::CtnApiClient("abc", "def");
    
    std::map<std::string, std::string> params;
    params[":key1"] = "one";
    params[":value1"] = "two";
    params[":key2"] = "three";
    params[":value2"] = "four";
    
    boost::property_tree::ptree pt;
    
    client->getRequest(":key1/:value1/:key2/:value2", params, pt);
    
    for (auto it: pt)
        std::cout << it.first << "," << it.second.data() << std::endl;
    
    return 0;
}

// Get request
bool ctn::CtnApiClient::getRequest(std::string methodpath, std::map<std::string, std::string> &params, boost::property_tree::ptree &response_ptree)
{
    bool success = true;
    
    // Temp server
    std::string server = "echo.jsontest.com";
    
    // Process methodpath from params
    for(auto const &data : params)
    {
        methodpath.replace(methodpath.find(data.first), data.first.length(), data.second);
    }
    methodpath = "/" + methodpath;
    
    try
    {
        boost::asio::io_service io_service;
        
        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(server, "http");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        
        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);
        
        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << methodpath << " HTTP/1.0\r\n";
        request_stream << "Host: " << server << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";
        
        // Send the request.
        boost::asio::write(socket, request);
        
        // Check that response is OK.
        boost::asio::streambuf response_buf;
        boost::system::error_code error;
        boost::asio::read_until(socket, response_buf, "\r\n");
        
        std::istream response_stream(&response_buf);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            success = false;
        }
        if (status_code != 200)
        {
            std::cout << "Response returned with status code " << status_code << "\n";
            success = false;
        }
        
        // Read all headers and flush buffer
        int headerSize = boost::asio::read_until(socket, response_buf, "\r\n\r\n");
        response_buf.consume(headerSize);
        
        // Read until EOF
        boost::asio::read(socket, response_buf, boost::asio::transfer_all(), error);
        if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);
        
        // Write response payload json to ptree
        boost::property_tree::json_parser::read_json(response_stream, response_ptree);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
        success = false;
    }
    
    return success;
}

// Constructor
ctn::CtnApiClient::CtnApiClient(std::string device_id, std::string api_access_secret, std::string host, std::string environment, bool secure, std::string version)
{
    this->device_id_ = device_id;
    this->api_access_secret_ = api_access_secret;
    
    this->host_ = host;
    this->subdomain_ = environment;
    this->secure_ = secure;
    this->version_ = version;
}

// SHA256 Hash
std::string ctn::CtnApiClient::hashData(const std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    ss << std::hex;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// HMAC-SHA256 signing
std::string ctn::CtnApiClient::signData(std::string key, std::string data, bool hex_encode)
{
    unsigned int len;
    unsigned char *raw = HMAC(EVP_sha256(), (unsigned char*) key.c_str(), key.length(),  (unsigned char*) data.c_str(), data.length(), NULL, &len);
    std::stringstream ss;
    
    if(hex_encode == false)
    {
        ss << raw;
    }
    else
    {
        ss << std::hex;
        for(int i = 0; i < len; i++)
        {
            ss << std::setw(2) << std::setfill('0') << (int)raw[i];
        }
        
    }
    return ss.str();
}
