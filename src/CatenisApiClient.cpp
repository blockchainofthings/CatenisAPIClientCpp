//
//  CatenisApiClient.cpp
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 5/25/17.
//
#include <CatenisApiClient.h>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <iterator>
#include <map>
#include <ctime>

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>

using boost::asio::ip::tcp;

int main()
{
    ctn::CtnApiClient client("d4XwmaAyRJdSYp4CZuTA", "86ccbaeaff0e6fc9cff989dde3f1b2c2e92761202c40873b07166b7f20b4d7cb682be163305cae4a2f8df557536163416f539b2f6f17fb5f6a20e309fdbec9ce", "beta.catenis.io");
    
    std::string result;
    client.readMessage("mAsM3cF27vLk6KA4fZoG", result);
    //client->logMessage("hey1234", result);
    //client->retrieveMessageContainer("mAsM3cF27vLk6KA4fZoG", result);
    client.sendMessage(ctn::Device("dcdHMFcZh4qmGAmiMg75", false), "hey whats up",result);
    std::cout << result;
    
    return 0;
}

bool ctn::CtnApiClient::logMessage(std::string message, std::string &data, const MethodOption &option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    // write request body
    request_data.put("message", message);
    request_data.put("options.encoding", option.encoding);
    request_data.put("options.encrypt", option.encrypt);
    request_data.put("options.storage", option.storage);
    
    return httpRequest("POST", "messages/log", params, queries, request_data, data);
}

bool ctn::CtnApiClient::sendMessage(const Device &device, std::string message, std::string &data, const MethodOption &option)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    // write request body
    request_data.put("targetDevice.id", device.id);
    request_data.put("targetDevice.isProdUniqueId", device.is_prod_uniqueid);
    request_data.put("options.encoding", option.encoding);
    request_data.put("message", message);
    request_data.put("options.encoding", option.encoding);
    request_data.put("options.encrypt", option.encrypt);
    request_data.put("options.storage", option.storage);
    
    return httpRequest("POST", "messages/send", params, queries, request_data, data);
}

bool ctn::CtnApiClient::readMessage(std::string message_id, std::string &data, std::string encoding)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    params[":messageId"] = message_id;
    queries["encoding"] = encoding;
    
    return httpRequest("GET", "messages/:messageId", params, queries, request_data, data);
}

bool ctn::CtnApiClient::retrieveMessageContainer(std::string message_id, std::string &data)
{
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> queries;
    boost::property_tree::ptree request_data;
    
    params[":messageId"] = message_id;
    
    return httpRequest("GET", "messages/:messageId/container", params, queries, request_data, data);
}

// http request
bool ctn::CtnApiClient::httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, boost::property_tree::ptree &request_data, std::string &response_data)
{
    bool success = true;
    
    // Add entire path
    methodpath = this->root_api_endpoint_ + "/" + methodpath;
    
    // Process methodpath from params and queries
    for(auto const &data : params)
    {
        methodpath.replace(methodpath.find(data.first), data.first.length(), data.second);
    }
    for(auto const &data : queries)
    {
        // if not first query add "&", else add "?"
        if(data != *queries.begin()) methodpath += "&";
        else methodpath += "?";
        methodpath += data.first + "=" + data.second;
    }
    
    // Create neccesary headers
    time_t now = std::time(0);
    char iso_time[17];
    strftime(iso_time, sizeof iso_time, "%Y%m%dT%H%M%SZ", gmtime(&now));
    std::map<std::string, std::string> headers;
    
    headers["host"] = this->host_;
    headers[TIME_STAMP_HDR] = std::string(iso_time);
    
    std::string payload_json = "";
    // if POST, process payload
    if(verb == "POST")
    {
        std::ostringstream payload_buf;
        write_json (payload_buf, request_data, false);
        
        // fix boost.ptree bug where it treats all types as string
        boost::regex exp("\"(null|true|false|[0-9]+(\\.[0-9]+)?)\"");
        payload_json = boost::regex_replace(payload_buf.str(), exp, "$1");
    }
    
    // Create signature and add to header
    signRequest(verb, methodpath, headers, payload_json, now);
    
    // Set up TCP/IP connection with server and send request
    try
    {
        std::string prefix = (this->secure_ ? "https" : "http");
        
        // Get a list of endpoints corresponding to the server name
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(headers["host"], prefix);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        
        // Declare both socket type pointers to reuse code for ssl and http
        tcp::socket *http_socket;
        boost::asio::ssl::stream<tcp::socket> *ssl_socket;
        
        if(this->secure_)
        {
            // HTTPS
            // Create context
            boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
            ssl_socket = new boost::asio::ssl::stream<tcp::socket>(io_service, ctx);
            
            // Try each endpoint until successful connection
            boost::asio::connect(ssl_socket->lowest_layer(), endpoint_iterator);
            
            // Handshake with server
            ssl_socket->set_verify_mode(boost::asio::ssl::verify_none);
            ssl_socket->handshake(boost::asio::ssl::stream_base::handshake_type::client);
        }
        else
        {
            // HTTP
            // Try each endpoint until we successfully establish a connection
            http_socket = new tcp::socket(io_service);
            boost::asio::connect(*http_socket, endpoint_iterator);
        }
        
        // Form the request
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        
        // Use HTTP 1.0 for now, since persistent connections not needed yet
        request_stream << verb << " " << methodpath << " HTTP/1.0\r\n";
        for(auto const &data : headers)
        {
            request_stream << data.first << ": " << data.second << "\r\n";
        }
        // Add extra headers if POST
        if(verb == "POST")
        {
            request_stream << "Content-Type: application/json; charset=utf-8\r\n";
            request_stream << "Content-Length: " << payload_json.length() << "\r\n";
        }
        
        // "Connection: close" header so that the server will close the socket after transmitting the response
        request_stream << "Connection: close\r\n\r\n";
        
        // add payload
        request_stream << payload_json;
        
        // Send the request
        if(this->secure_) boost::asio::write(*ssl_socket, request);
        else boost::asio::write(*http_socket, request);
        
        // Check that response is OK.
        boost::asio::streambuf response_buf;
        boost::system::error_code error;
        if(this->secure_) boost::asio::read_until(*ssl_socket, response_buf, "\r\n");
        else boost::asio::read_until(*http_socket, response_buf, "\r\n");
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
        int headerSize;
        if(this->secure_) headerSize = boost::asio::read_until(*ssl_socket, response_buf, "\r\n\r\n");
        else headerSize = boost::asio::read_until(*http_socket, response_buf, "\r\n\r\n");
        response_buf.consume(headerSize);
        
        // Read until EOF or Short read,
        if(this->secure_) boost::asio::read(*ssl_socket, response_buf, boost::asio::transfer_all(), error);
        else boost::asio::read(*http_socket, response_buf, boost::asio::transfer_all(), error);
        if (error != boost::asio::error::eof && error !=  boost::system::error_code(ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ), boost::asio::error::get_ssl_category()))
            throw boost::system::system_error(error);
        
        // Write response data
        response_data = std::string(std::istreambuf_iterator<char>(response_stream), std::istreambuf_iterator<char>());
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
    
    this->root_api_endpoint_ = API_PATH + this->version_;
}

// Generate Signature
void ctn::CtnApiClient::signRequest(std::string verb, std::string endpoint, std::map<std::string, std::string> &headers, std::string payload, time_t now)
{
    std::string timestamp = headers[TIME_STAMP_HDR];
    char date_buffer[9];
    std::string signdate;
    bool use_same_signkey;
    
    // Use last signkey if date < valid days
    if(this->last_signkey_.length() != 0 && std::difftime(now, this->last_signdate_)/(3600 * 24) < SIGN_VALID_DAYS)
    {
        use_same_signkey = true;
        strftime(date_buffer, sizeof date_buffer, "%Y%m%d", gmtime(&last_signdate_));
        signdate = std::string(date_buffer);
    }
    else
    {
        use_same_signkey = false;
        this->last_signdate_ = now;
        strftime(date_buffer, sizeof date_buffer, "%Y%m%d", gmtime(&now));
        signdate = std::string(date_buffer);
    }
    
    // 1) Compute conformed request
    std::string conf_req = verb + "\n";
    conf_req += endpoint + "\n";
    for(auto const &data : headers)
    {
        // All header must be in lower case
        conf_req += data.first + ":" + data.second + "\n";
    }
    conf_req += "\n" + hashData(payload) + "\n";
    
    // 2) Assemble string to sign
    std::string str_to_sign = SIGN_METHOD_ID + "\n";
    str_to_sign += timestamp + "\n";
    std::string scope = signdate + "/" + SCOPE_REQUEST;
    str_to_sign += scope + "\n";
    str_to_sign += hashData(conf_req) + "\n";

    // 3) Generate signature
    std::string datekey = signData(SIGN_VERSION_ID + this->api_access_secret_, signdate);
    std::string signkey;
    // check bool and use last sign key
    if(use_same_signkey) signkey = this->last_signkey_;
    else signkey = this->last_signkey_ = signData(datekey, SCOPE_REQUEST);
    std::string signature = signData(signkey, str_to_sign, true);
    
    // 4) add auth header
    headers["authorization"] = SIGN_METHOD_ID + " Credential=" + this->device_id_ + "/" + scope + ", Signature=" + signature;

    return;
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
