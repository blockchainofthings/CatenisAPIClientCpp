//
//  CatenisApiInternals.cpp
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 6/21/17.
//  Modifications by R. Benson Evans on 2/20/2018.
//


#include <utility>
#include <iostream>
#include <string>
#include <ctime>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include <list>
#include <memory>

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>

#include <boost/foreach.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::asio::ip::tcp;

#include <CatenisApiException.h>
#include <CatenisApiInternals.h>

// http request
void ctn::CtnApiInternals::httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, boost::property_tree::ptree &request_data, std::string &response_data)
{
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
        if(this->port_ != "") query = tcp::resolver::query(headers["host"], this->port_);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        
        // Declare and init both sockets, this is a fix due to boost.asio memory bug in windows
        // HTTPS
        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
        boost::asio::ssl::stream<tcp::socket> ssl_socket(io_service, ctx);
        // http
        tcp::socket http_socket(io_service);
        
        if(this->secure_)
        {
            // Try each endpoint until successful connection
            boost::asio::connect(ssl_socket.lowest_layer(), endpoint_iterator);
            
            // Handshake with server
            ssl_socket.set_verify_mode(boost::asio::ssl::verify_none);
            ssl_socket.handshake(boost::asio::ssl::stream_base::handshake_type::client);
        }
        else
        {
            // HTTP
            // Try each endpoint until we successfully establish a connection
            boost::asio::connect(http_socket, endpoint_iterator);
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
        if(this->secure_) boost::asio::write(ssl_socket, request);
        else boost::asio::write(http_socket, request);
        
        // Check that response is OK.
        boost::asio::streambuf response_buf;
        boost::system::error_code error;
        if(this->secure_) boost::asio::read_until(ssl_socket, response_buf, "\r\n");
        else boost::asio::read_until(http_socket, response_buf, "\r\n");
        std::istream response_stream(&response_buf);
        
        // Check the stream at each point.
        if (!response_stream) 
            throw CatenisClientError("Invalid HTTP response");

        std::string http_version;
        response_stream >> http_version;
        if (http_version.substr(0, 5) != "HTTP/")
            throw CatenisClientError("Invalid HTTP response");

        int status_code;
        if (!response_stream) 
            throw CatenisClientError("Invalid HTTP response");
        response_stream >> status_code;

        std::string status_message;
        if (!response_stream) 
            throw CatenisClientError("Invalid HTTP response");
        std::getline(response_stream, status_message, '\r');  // "protocol string ends with \r\n".

        // Erase white-space from beginning of string
        status_message.erase(0, 1);
        
        // Read all headers and flush buffer
        int headerSize;
        if(this->secure_) headerSize = boost::asio::read_until(ssl_socket, response_buf, "\r\n\r\n");
        else headerSize = boost::asio::read_until(http_socket, response_buf, "\r\n\r\n");
        response_buf.consume(headerSize);
        
        // Read until EOF or Short read,
        if(this->secure_) boost::asio::read(ssl_socket, response_buf, boost::asio::transfer_all(), error);
        else boost::asio::read(http_socket, response_buf, boost::asio::transfer_all(), error);
        if (error != boost::asio::error::eof && error !=  boost::system::error_code(ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ), boost::asio::error::get_ssl_category()))
            throw boost::system::system_error(error);
        
        // Write response data
        response_data = std::string(std::istreambuf_iterator<char>(response_stream), std::istreambuf_iterator<char>());

        if (status_code != 200) {
            ApiErrorResponse errorResponse;
            parseApiErrorResponse(errorResponse, response_data);

            throw CatenisAPIError(status_message, status_code, errorResponse);
        }
    }
    catch (std::exception& e)
    {
        throw CatenisClientError(e.what());
    }
}

// Generate Signature and add to request
void ctn::CtnApiInternals::signRequest(std::string verb, std::string endpoint, std::map<std::string, std::string> &headers, std::string payload, time_t now)
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

//Contructor
ctn::CtnApiInternals::CtnApiInternals(std::string device_id, std::string api_access_secret, std::string host, std::string port, std::string environment, bool secure, std::string version)
{
    this->device_id_ = device_id;
    this->api_access_secret_ = api_access_secret;
    
    this->port_ = port;
    this->secure_ = secure;
    this->version_ = version;
    
    //if beta, make host beta.catenis.io
    this->subdomain_ = (environment == "beta" ? "beta." : "");
    this->host_ = this->subdomain_ + host;
        
    this->root_api_endpoint_ = API_PATH + this->version_;
}

// SHA256 Hash
std::string ctn::CtnApiInternals::hashData(const std::string str)
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
std::string ctn::CtnApiInternals::signData(std::string key, std::string data, bool hex_encode)
{
    unsigned int len;
    unsigned char *raw = HMAC(EVP_sha256(), (unsigned char*) key.c_str(), key.length(),  (unsigned char*) data.c_str(), data.length(), NULL, &len);
    std::stringstream ss;
    
    if(hex_encode == false)
    {
        return std::string((char *)raw, (std::string::size_type)len);
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

void ctn::CtnApiInternals::parseApiErrorResponse(ApiErrorResponse &error_response, std::string &json_data) {
    ptree pt;
    std::istringstream is(json_data);

    read_json(is, pt);

    std::string status = pt.get<std::string>("status");

    if (status == "error")
    {
        try
        {
            error_response.status = status;
            error_response.message = pt.get<std::string>("message");
            return;
        }
        catch(...)
        {
            throw CatenisClientError("Unexpected API error response");
        }
    }
    else
    {
        throw CatenisClientError("Unexpected API error response");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseLogMessage(LogMessageResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            user_return_data.messageId = pt.get<std::string>("data.messageId"); 
            return;
        }
        catch(...)
        {
            throw CatenisClientError("Unexpected returned data from Log Message API method");
        }     
    }
    else
    {
        throw CatenisClientError("Unexpected returned data from Log Message API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseSendMessage(SendMessageResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            user_return_data.messageId = pt.get<std::string>("data.messageId"); 
            return;
        }
        catch(...)
        {
            throw CatenisClientError("Unexpected returned data from Send Message API method");
        }
    }
    else
    {
        throw CatenisClientError("Unexpected returned data from Send Message API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseReadMessage(ReadMessageResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            user_return_data.action = pt.get<std::string>("data.action");

            std::string from_deviceId = pt.get<std::string>("data.from.deviceId","");
            if (!from_deviceId.empty())
            {
                std::string from_name = pt.get<std::string>("data.from.name","");
                std::string from_prodUniqueId = pt.get<std::string>("data.from.prodUniqueId","");
                std::shared_ptr<DeviceInfo> from_obj (new DeviceInfo(from_deviceId, from_name, from_prodUniqueId));
                user_return_data.from = from_obj;
            }
            else
            { 
                user_return_data.from = nullptr;
            }

            user_return_data.message = pt.get<std::string>("data.message");
            return;
        }
        catch(...)
        {
            throw CatenisClientError("Unexpected returned data from Read Message API method");
        }
    }
    else
    {
        throw CatenisClientError("Unexpected returned data from Read Message API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseRetrieveMessageContainer(RetrieveMessageContainerResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            //StorageProviderDictionary;

            user_return_data.blockchain.txid = pt.get<std::string>("data.blockchain.txid");
            std::string val = pt.get<std::string>("data.blockchain.isConfirmed");
            user_return_data.blockchain.isConfirmed = true;
            if (val.compare("true") != 0)
                user_return_data.blockchain.isConfirmed = false;

            try
            {
                // if no elements throw an exception and return normally.
                std::string storage = pt.get<std::string>("data.externalStorage");
            }
            catch(...)
            {
                user_return_data.externalStorage = nullptr;
                return; // done parsing.
            }

            BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("data.externalStorage"))
            {
                std::shared_ptr<StorageProviderDictionary> map_objPtr (new StorageProviderDictionary());
                user_return_data.externalStorage = map_objPtr;
                (*user_return_data.externalStorage)[v.first.data()] = v.second.data();
            }
            return;
        }
        catch(...)
        {
            throw CatenisClientError("Unexpected returned data from Retrieve Message Container API method");
        }
    }
    else
    {
        throw CatenisClientError("Unexpected returned data from Retrieve Message Container API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseListMessages(ListMessagesResult &user_return_data, std::string json_data)
{
    ptree pt;
    std::istringstream is(json_data);
    read_json(is, pt);
    std::string status = pt.get<std::string>("status");
    if (status.compare("success") == 0)
    {
        try
        {
            BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("data.messages"))
            {
                // std::cout << "First data: " << v.first.data() << std::endl; // should be empty.
                boost::property_tree::ptree subtree = (boost::property_tree::ptree) v.second;
                
                std::string messageId = subtree.get<std::string>("messageId");
                std::string action = subtree.get<std::string>("action");

                std::shared_ptr<bool> read_confirmation_enabled;
                if (subtree.find("readConfirmationEnabled") != subtree.not_found()) {
                    read_confirmation_enabled = std::make_shared<bool>(subtree.get<bool>("readConfirmationEnabled",false));
                }

                std::shared_ptr<bool> read;
                if (subtree.find("read") != subtree.not_found()) {
                    read = std::make_shared<bool>(subtree.get<bool>("read",false));
                }
                
                std::string date = subtree.get<std::string>("date");
                std::string direction = subtree.get<std::string>("direction","");

                std::shared_ptr<DeviceInfo> from_device_obj = nullptr;
                std::string from_deviceId = subtree.get<std::string>("from.deviceId","");
                if (!from_deviceId.empty())
                {
                    std::string from_name = subtree.get<std::string>("from.name","");
                    std::string from_prodUniqueId = subtree.get<std::string>("from.prodUniqueId","");
                    std::shared_ptr<DeviceInfo> from_obj (new DeviceInfo(from_deviceId, from_name, from_prodUniqueId));
                    from_device_obj = from_obj;
                }

                std::shared_ptr<DeviceInfo> to_device_obj = nullptr;
                std::string to_deviceId = subtree.get<std::string>("to.deviceId","");
                if (!to_deviceId.empty())
                {
                    std::string to_name = subtree.get<std::string>("to.name","");
                    std::string to_prodUniqueId = subtree.get<std::string>("to.prodUniqueId","");
                    std::shared_ptr<DeviceInfo> to_obj (new DeviceInfo(to_deviceId, to_name, to_prodUniqueId));
                    to_device_obj = to_obj;
                }

                std::shared_ptr<MessageDescription> msg_obj(new MessageDescription(messageId, action, direction, from_device_obj, to_device_obj, read_confirmation_enabled, read, date));

                user_return_data.messageList.push_back(msg_obj);
            }
            user_return_data.msgCount = pt.get<int>("data.msgCount",0);
            user_return_data.countExceeded = pt.get<bool>("data.countExceeded",false);
            return;
       }
       catch(...)
       {
           throw CatenisClientError("Unexpected returned data from List Messages API method");
       }
    }
    else
    {
        throw CatenisClientError("Unexpected returned data from List Messages API method");
    }
}

// STATIC method; used by an exception.
std::string ctn::CtnApiInternals::parseJsonForKeyValue(std::string json_data, std::string key)
{
    try
    {
        ptree pt;
        std::istringstream is(json_data);
        read_json(is, pt);
        return(pt.get<std::string>(key,""));
    }
    catch(...)
    {
        return("");
    }
}

