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

#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
/*#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>*/
#include <json-spirit/json_spirit_reader_template.h>
#include <json-spirit/json_spirit_writer_template.h>

using boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
#elif defined(COM_SUPPORT_LIB_POCO)
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/Context.h>
#endif

#include <CatenisApiException.h>
#include <CatenisApiInternals.h>

// http request
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
void ctn::CtnApiInternals::httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, json_spirit::mValue const &request_data, std::string &response_data)
#elif defined(COM_SUPPORT_LIB_POCO)
void ctn::CtnApiInternals::httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, Poco::JSON::Object &request_data, std::string &response_data)
#endif
{
    // Assemble complete path
    methodpath = this->root_api_endpoint_ + "/" + methodpath;

    // Add path parameters if required
    for(auto const &data : params)
    {
        methodpath.replace(methodpath.find(data.first), data.first.length(), data.second);
    }

    // Add query string if required
    for(auto const &data : queries)
    {
        // if not first query add "&", else add "?"
        if(data != *queries.begin()) methodpath += "&";
        else methodpath += "?";
        methodpath += data.first + "=" + data.second;
    }

    // Add request payload if required
    std::string payload_json;

    if(verb == "POST")
    {
        std::ostringstream payload_buf;
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        payload_json = json_spirit::write_string(request_data, json_spirit::Output_options::raw_utf8);
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Stringifier::stringify(request_data, payload_buf);
        payload_json = payload_buf.str();
#endif
    }

    // Create necessary headers
    time_t now = std::time(0);
    char iso_time[17];
    strftime(iso_time, sizeof iso_time, "%Y%m%dT%H%M%SZ", gmtime(&now));
    std::map<std::string, std::string> headers;
    
    headers["host"] = this->host_;
    headers[TIME_STAMP_HDR] = std::string(iso_time);
    
    // Create signature and add to header
    signRequest(verb, methodpath, headers, payload_json, now);


    // Set up TCP/IP connection with server and send request
    try
    {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        boost::asio::io_context ioc;
        ssl::context ctx(ssl::context::sslv23_client);

        tcp::socket socket(ioc);
        ssl::stream<tcp::socket> ssl_stream(ioc, ctx);

        // Look up the domain name
        tcp::resolver resolver(ioc);
        auto const results = resolver.resolve(host_, !port_.empty() ? port_ : (secure_ ? "https" : "http"));

        if (secure_) {
            // Set SNI Hostname (many hosts need this to handshake successfully)
            if(! SSL_set_tlsext_host_name(ssl_stream.native_handle(), host_.c_str()))
            {
                boost::system::error_code ec(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
                throw boost::system::system_error(ec);
            }

            // Open connection
            boost::asio::connect(ssl_stream.lowest_layer(), results.begin(), results.end());

            // Perform the SSL handshake
            ssl_stream.set_verify_mode(boost::asio::ssl::verify_none);
            ssl_stream.handshake(ssl::stream_base::client);
        }
        else {
            // Open the connection
            boost::asio::connect(socket, results.begin(), results.end());
        }

        // Prepare HTTP request
        http::request<http::string_body> req(verb == "POST" ? http::verb::post : http::verb::get, methodpath, 11);

        // Add headers
        for (auto const &header : headers)
        {
            req.set(header.first, header.second);
        }

        req.set(http::field::content_type, "application/json; charset=utf-8");
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::connection, "close");

        // add payload
        req.body() = payload_json;
        req.prepare_payload();

        // Send the HTTP request
        if (secure_)
            http::write(ssl_stream, req);
        else
            http::write(socket, req);

        // Prepare to receive response.
        boost::beast::flat_buffer buffer;
        http::response<http::string_body> res;

        // Receive the HTTP response
        if (secure_)
            http::read(ssl_stream, buffer, res);
        else
            http::read(socket, buffer, res);

        // Get response and copy to response_data
        response_data = res.body();

        unsigned int status_code = res.result_int();
        std::string status_message = res.reason().to_string();

        // Close connection
        if (secure_) {
            // Gracefully close the stream
            boost::system::error_code ec;
            ssl_stream.shutdown(ec);

            if(ec == boost::asio::error::eof || ec == ssl::error::stream_errors::stream_truncated) {
                //std::cerr << ">>>>>> SSL stream shutdown failed: " << ec.message() << std::endl;
                // Rationale: http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                ec.assign(0, ec.category());
            }

            if(ec)
                throw boost::system::system_error(ec);
        }
        else {
            // Gracefully close the socket
            boost::system::error_code ec;
            socket.shutdown(tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes so don't bother reporting it.
            if (ec && ec != boost::system::errc::not_connected)
                throw boost::system::system_error(ec);
        }
#elif defined(COM_SUPPORT_LIB_POCO)
        // Contruct url
        std::string prefix = (this->secure_ ? "https://" : "http://");
        std::string url = prefix + this->host_;
        if(this->port_ != "") url = url + ":" + this->port_;
        url = url + methodpath;

        Poco::URI uri(url);
        // HTTP
        Poco::Net::HTTPClientSession http_session(uri.getHost(), uri.getPort());
        // HTTPS
        const Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", Poco::Net::Context::VERIFY_NONE);
        Poco::Net::HTTPSClientSession ssl_session(uri.getHost(), uri.getPort(), context);

        // Prepare path
        std::string path(uri.getPathAndQuery());
        if (path.empty()) path = "/";

        // Make request and add header
        Poco::Net::HTTPRequest request(verb, path, Poco::Net::HTTPMessage::HTTP_1_1);
        for(auto const &data : headers)
        {
            request.add(data.first, data.second);
        }
        request.setContentType("application/json; charset=utf-8");
        request.setContentLength(payload_json.length());

        // Send Request
        if(this->secure_) ssl_session.sendRequest(request) << payload_json;
        else http_session.sendRequest(request) << payload_json;

        // Get response and copy to response_data
        Poco::Net::HTTPResponse res;
        if(this->secure_) Poco::StreamCopier::copyToString(ssl_session.receiveResponse(res), response_data);
        else Poco::StreamCopier::copyToString(http_session.receiveResponse(res), response_data);

        unsigned int status_code = res.getStatus();
        std::string status_message = res.getReason();
#endif

        if (status_code != 200) {
            ApiErrorResponse errorResponse;
            parseApiErrorResponse(errorResponse, response_data);

            throw CatenisAPIError(status_message, status_code, errorResponse);
        }
    }
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
    catch (std::exception& e)
    {
        throw CatenisClientError(e.what());
#elif defined(COM_SUPPORT_LIB_POCO)
    catch (Poco::Exception &ex)
    {
        throw CatenisClientError(ex.displayText());
#endif
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
    
    //if sandbox, make host sandbox.catenis.io
    this->subdomain_ = (environment == "sandbox" ? "sandbox." : "");
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
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "error") {
            error_response.status = status;
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            error_response.message = retObj["message"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
            error_response.message = retObj->getValue<std::string>("message");
#endif
        } else {
            throw CatenisClientError("Unexpected API error response");
        }
    }
    catch (...) {
        throw CatenisClientError("Unexpected API error response");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseLogMessage(LogMessageResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            user_return_data.messageId = data["messageId"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            user_return_data.messageId = data->getValue<std::string>("messageId");
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Log Message API method");
        }
    }
    catch(...) {
        throw CatenisClientError("Unexpected returned data from Log Message API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseSendMessage(SendMessageResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            user_return_data.messageId = data["messageId"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            user_return_data.messageId = data->getValue<std::string>("messageId");
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Send Message API method");
        }
    }
    catch(...) {
        throw CatenisClientError("Unexpected returned data from Send Message API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseReadMessage(ReadMessageResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            user_return_data.action = data["action"].get_str();

            if (data.find("from") != data.end()) {
                json_spirit::mObject &from = data["from"].get_obj();

                std::string const &from_deviceId = from["deviceId"].get_str();

                std::string from_name;
                if (from.find("name") != from.end()) {
                    from_name = from["name"].get_str();
                }

                std::string from_prodUniqueId;
                if (from.find("prodUniqueId") != from.end()) {
                    from_prodUniqueId = from["prodUniqueId"].get_str();
                }

                std::shared_ptr<DeviceInfo> from_obj(new DeviceInfo(from_deviceId, from_name, from_prodUniqueId));
                user_return_data.from = from_obj;
            }
            else {
                user_return_data.from = nullptr;
            }

            user_return_data.message = data["message"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            user_return_data.action = data->getValue<std::string>("action");

            if (data->has("from")) {
                Poco::JSON::Object::Ptr from = data->getObject("from");

                std::string from_deviceId = from->getValue<std::string>("deviceId");

                std::string from_name;
                if (from->has("name")) {
                    from_name = from->getValue<std::string>("name");
                }

                std::string from_prodUniqueId;
                if (from->has("prodUniqueId")) {
                    from_prodUniqueId = from->getValue<std::string>("prodUniqueId");
                }

                std::shared_ptr<DeviceInfo> from_obj(new DeviceInfo(from_deviceId, from_name, from_prodUniqueId));
                user_return_data.from = from_obj;
            }
            else {
                user_return_data.from = nullptr;
            }

            user_return_data.message = data->getValue<std::string>("message");
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Read Message API method");
        }
    }
    catch(...) {
        throw CatenisClientError("Unexpected returned data from Read Message API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseRetrieveMessageContainer(RetrieveMessageContainerResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            json_spirit::mObject &blockchain = data["blockchain"].get_obj();

            user_return_data.blockchain.txid = blockchain["txid"].get_str();
            user_return_data.blockchain.isConfirmed = blockchain["isConfirmed"].get_bool();

            if (data.find("externalStorage") != data.end()) {
                json_spirit::mObject &externalStorage = data["externalStorage"].get_obj();

                std::shared_ptr<StorageProviderDictionary> map_objPtr(new StorageProviderDictionary());

                for (auto const &storageProvider : externalStorage) {
                    (*map_objPtr)[storageProvider.first] = storageProvider.second.get_str();
                }

                user_return_data.externalStorage = map_objPtr;
            }
            else {
                user_return_data.externalStorage = nullptr;
            }
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            Poco::JSON::Object::Ptr blockchain = data->getObject("blockchain");

            user_return_data.blockchain.txid = blockchain->getValue<std::string>("txid");
            user_return_data.blockchain.isConfirmed = blockchain->getValue<bool>("isConfirmed");

            if (data->has("externalStorage")) {
                Poco::JSON::Object::Ptr externalStorage = data->getObject("externalStorage");

                std::shared_ptr<StorageProviderDictionary> map_objPtr(new StorageProviderDictionary());

                std::vector<std::string> storageProviders;
                externalStorage->getNames(storageProviders);

                for (std::vector<std::string>::iterator storageProvider = storageProviders.begin(); storageProvider != storageProviders.end(); storageProvider++) {
                    (*map_objPtr)[*storageProvider] = externalStorage->getValue<std::string>(*storageProvider);
                }

                user_return_data.externalStorage = map_objPtr;
            }
            else {
                user_return_data.externalStorage = nullptr;
            }
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Retrieve Message Container API method");
        }
    }
    catch(...) {
        throw CatenisClientError("Unexpected returned data from Retrieve Message Container API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseListMessages(ListMessagesResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success")
        {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();
            
            json_spirit::mArray &messages = data["messages"].get_array();
            
            for (json_spirit::mValue &entry : messages) {
                json_spirit::mObject &message = entry.get_obj();

                std::string const &messageId = message["messageId"].get_str();
                std::string const &action = message["action"].get_str();

                std::string direction;
                if (message.find("direction") != message.end()) {
                    direction = message["direction"].get_str();
                }

                std::shared_ptr<DeviceInfo> from_device_obj;
                if (message.find("from") != message.end()) {
                    json_spirit::mObject &from = message["from"].get_obj();

                    std::string const &from_deviceId = from["deviceId"].get_str();

                    std::string from_name;
                    if (from.find("name") != from.end()) {
                        from_name = from["name"].get_str();
                    }

                    std::string from_prodUniqueId;
                    if (from.find("prodUniqueId") != from.end()) {
                        from_prodUniqueId = from["prodUniqueId"].get_str();
                    }

                    from_device_obj.reset(new DeviceInfo(from_deviceId, from_name, from_prodUniqueId));
                }

                std::shared_ptr<DeviceInfo> to_device_obj;
                if (message.find("to") != message.end()) {
                    json_spirit::mObject &to = message["to"].get_obj();

                    std::string const &to_deviceId = to["deviceId"].get_str();

                    std::string to_name;
                    if (to.find("name") != to.end()) {
                        to_name = to["name"].get_str();
                    }

                    std::string to_prodUniqueId;
                    if (to.find("prodUniqueId") != to.end()) {
                        to_prodUniqueId = to["prodUniqueId"].get_str();
                    }

                    to_device_obj.reset(new DeviceInfo(to_deviceId, to_name, to_prodUniqueId));
                }

                std::shared_ptr<bool> read_confirmation_enabled;
                if (message.find("readConfirmationEnabled") != message.end()) {
                    read_confirmation_enabled.reset(new bool(message["readConfirmationEnabled"].get_bool()));
                }

                std::shared_ptr<bool> read;
                if (message.find("read") != message.end()) {
                    read.reset(new bool(message["read"].get_bool()));
                }

                std::string const &date = message["date"].get_str();

                std::shared_ptr<MessageDescription> msg_obj(new MessageDescription(messageId, action, direction, from_device_obj, to_device_obj, read_confirmation_enabled, read, date));

                user_return_data.messageList.push_back(msg_obj);
            }

            user_return_data.msgCount = data["msgCount"].get_int();
            user_return_data.countExceeded = data["countExceeded"].get_bool();
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            Poco::JSON::Array::Ptr messages = data->getArray("messages");

            for (std::size_t idx = 0, limit = messages->size(); idx < limit; idx++) {
                Poco::JSON::Object::Ptr message = messages->getObject(idx);

                std::string messageId = message->getValue<std::string>("messageId");
                std::string action = message->getValue<std::string>("action");

                std::string direction;
                if (message->has("direction")) {
                    direction = message->getValue<std::string>("direction");
                }

                std::shared_ptr<DeviceInfo> from_device_obj;
                if (message->has("from")) {
                    Poco::JSON::Object::Ptr from = message->getObject("from");
                
                    std::string from_deviceId = from->getValue<std::string>("deviceId");
                    
                    std::string from_name;
                    if (from->has("name")) {
                        from_name = from->getValue<std::string>("name");
                    }
                    
                    std::string from_prodUniqueId;
                    if (from->has("prodUniqueId")) {
                        from_prodUniqueId = from->getValue<std::string>("prodUniqueId");
                    }
                    
                    from_device_obj.reset(new DeviceInfo(from_deviceId, from_name, from_prodUniqueId));
                }

                std::shared_ptr<DeviceInfo> to_device_obj;
                if (message->has("to")) {
                    Poco::JSON::Object::Ptr to = message->getObject("to");
                
                    std::string to_deviceId = to->getValue<std::string>("deviceId");
                    
                    std::string to_name;
                    if (to->has("name")) {
                        to_name = to->getValue<std::string>("name");
                    }
                    
                    std::string to_prodUniqueId;
                    if (to->has("prodUniqueId")) {
                        to_prodUniqueId = to->getValue<std::string>("prodUniqueId");
                    }
                    
                    to_device_obj.reset(new DeviceInfo(to_deviceId, to_name, to_prodUniqueId));
                }

                std::shared_ptr<bool> read_confirmation_enabled;
                if (message->has("readConfirmationEnabled")) {
                    read_confirmation_enabled.reset(new bool(message->getValue<bool>("readConfirmationEnabled")));
                }

                std::shared_ptr<bool> read;
                if (message->has("read")) {
                    read.reset(new bool(message->getValue<bool>("read")));
                }

                std::string date = message->getValue<std::string>("date");

                std::shared_ptr<MessageDescription> msg_obj(new MessageDescription(messageId, action, direction, from_device_obj, to_device_obj, read_confirmation_enabled, read, date));

                user_return_data.messageList.push_back(msg_obj);
            }

            user_return_data.msgCount = data->getValue<int>("msgCount");
            user_return_data.countExceeded = data->getValue<bool>("countExceeded");
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from List Messages API method");
        }
    }
    catch(...) {
        throw CatenisClientError("Unexpected returned data from List Messages API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseListPermissionEvents(ListPermissionEventsResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            for (auto const &permissionEvent : data) {
                user_return_data.permissionEvents[permissionEvent.first] = permissionEvent.second.get_str();
            }
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            std::vector<std::string> eventNameList;
            data->getNames(eventNameList);

            for (std::vector<std::string>::iterator eventNameIdx = eventNameList.begin(); eventNameIdx != eventNameList.end(); eventNameIdx++) {
                user_return_data.permissionEvents[*eventNameIdx] = data->getValue<std::string>(*eventNameIdx);
            }
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from List Permission Events API method");
        }
    }
    catch (...) {
        throw CatenisClientError("Unexpected returned data from List Permission Events API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseRetrievePermissionRights(RetrievePermissionRightsResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            /* ############### SYSTEM LEVEL PERMISSION RIGHTS ###############*/
            user_return_data.system = data["system"].get_str();

            /* ############### CATENIS NODE PERMISSION RIGHTS ############### */
            if (data.find("catenisNode") != data.end()) {
                json_spirit::mObject &ctnNodeObj= data["catenisNode"].get_obj();

                std::list<std::string> allowed;
                std::list<std::string> denied;
                std::string ctnNodeId;

                if (ctnNodeObj.find("allow") != ctnNodeObj.end()) {
                    json_spirit::mArray &ctnNodeRights = ctnNodeObj["allow"].get_array();

                    for (json_spirit::mValue &entry : ctnNodeRights) {
                        ctnNodeId = entry.get_str();
                        allowed.push_back(ctnNodeId);
                    }
                }

                if (ctnNodeObj.find("deny") != ctnNodeObj.end()) {
                    json_spirit::mArray &ctnNodeRights = ctnNodeObj["deny"].get_array();

                    for (json_spirit::mValue &entry : ctnNodeRights) {
                        ctnNodeId = entry.get_str();
                        denied.push_back(ctnNodeId);
                    }
                }

                std::shared_ptr<PermissionRightsCatenisNode> node_obj(new PermissionRightsCatenisNode(allowed, denied));
                user_return_data.catenisNode = node_obj;
            }
            else {
                user_return_data.catenisNode = nullptr;
            }

            /* ############### CLIENT LEVEL PERMISSION RIGHTS ###############*/
            if (data.find("client") != data.end()) {
                json_spirit::mObject &clientObj = data["client"].get_obj();

                std::list<std::string> allowed;
                std::list<std::string> denied;

                if (clientObj.find("allow") != clientObj.end()) {
                    json_spirit::mArray &clientRights = clientObj["allow"].get_array();

                    for (json_spirit::mValue &entry : clientRights) {
                        std::string clientId = entry.get_str();
                        allowed.push_back(clientId);
                    }
                }

                if (clientObj.find("deny") != clientObj.end()) {
                    json_spirit::mArray &clientRights = clientObj["deny"].get_array();

                    for (json_spirit::mValue &entry : clientRights) {
                        std::string clientId = entry.get_str();
                        denied.push_back(clientId);
                    }
                }

                std::shared_ptr<PermissionRightsClient> node_obj(new PermissionRightsClient(allowed, denied));
                user_return_data.client = node_obj;
            }
            else {
                user_return_data.client = nullptr;
            }

            /* ############### DEVICE LEVEL PERMISSION RIGHTS ###############*/
            if (data.find("device") != data.end()) {
                json_spirit::mObject &deviceObj = data["device"].get_obj();

                std::list< std::shared_ptr<DeviceInfo> > allowed;
                std::list< std::shared_ptr<DeviceInfo> > denied;

                if (deviceObj.find("allow") != deviceObj.end()) {
                    json_spirit::mArray &virtualDevices = deviceObj["allow"].get_array();

                    for (json_spirit::mValue &entry : virtualDevices) {
                        json_spirit::mObject &thisDevice = entry.get_obj();

                        std::string const &deviceId = thisDevice["deviceId"].get_str();

                        std::string name;
                        if (thisDevice.find("name") != thisDevice.end()) {
                            name = thisDevice["name"].get_str();
                        }

                        std::string prodUniqueId;
                        if (thisDevice.find("prodUniqueId") != thisDevice.end()) {
                            prodUniqueId = thisDevice["prodUniqueId"].get_str();
                        }

                        std::shared_ptr<DeviceInfo> deviceRightsObj(new DeviceInfo(deviceId, name, prodUniqueId));
                        allowed.push_back(deviceRightsObj);
                    }
                }

                if (deviceObj.find("deny") != deviceObj.end()) {
                    json_spirit::mArray &virtualDevices = deviceObj["deny"].get_array();

                    for (json_spirit::mValue &entry : virtualDevices) {
                        json_spirit::mObject &thisDevice = entry.get_obj();

                        std::string const &deviceId = thisDevice["deviceId"].get_str();

                        std::string name;
                        if (thisDevice.find("name") != thisDevice.end()) {
                            name = thisDevice["name"].get_str();
                        }

                        std::string prodUniqueId;
                        if (thisDevice.find("prodUniqueId") != thisDevice.end()) {
                            prodUniqueId = thisDevice["prodUniqueId"].get_str();
                        }

                        std::shared_ptr<DeviceInfo> deviceRightsObj(new DeviceInfo(deviceId, name, prodUniqueId));
                        denied.push_back(deviceRightsObj);
                    }
                }

                std::shared_ptr<PermissionRightsDevice> deviceInfoObj(new PermissionRightsDevice(allowed, denied));
                user_return_data.device = deviceInfoObj;
            }
            else {
                    user_return_data.device= nullptr;
            }
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Object::Ptr data = retObj->getObject("data");

        /* ############### SYSTEM LEVEL PERMISSION RIGHTS ################*/
        user_return_data.system = data->getValue<std::string>("system");

        /* ############### CATENIS NODE PERMISSION RIGHTS ////###############*/
        if (data->has("catenisNode")) {
            Poco::JSON::Object::Ptr ctnNodeObj = data->getObject("catenisNode");

            std::list<std::string> allowed;
            if (ctnNodeObj->has("allow")) {
                Poco::JSON::Array::Ptr ctnNodeRights = ctnNodeObj->getArray("allow");
                for (std::size_t idx = 0, limit = ctnNodeRights->size(); idx < limit; idx++) {
                    std::string ctnNodeId = ctnNodeRights->get(idx);
                    allowed.push_back(ctnNodeId);
                }
            }

            std::list<std::string> denied;
            if (ctnNodeObj->has("deny")) {
                Poco::JSON::Array::Ptr ctnNodeRights = ctnNodeObj->getArray("deny");
                for (std::size_t idx = 0, limit = ctnNodeRights->size(); idx < limit; idx++) {
                    std::string ctnNodeId = ctnNodeRights->get(idx);
                    denied.push_back(ctnNodeId);
                }
            }

            std::shared_ptr<PermissionRightsCatenisNode> node_obj(new PermissionRightsCatenisNode(allowed, denied));
            user_return_data.catenisNode = node_obj;
        }
        else {
            user_return_data.catenisNode= nullptr;
        }

        /* ############### CLIENT LEVEL PERMISSION RIGHTS ////###############*/
        if (data->has("client")) {
            Poco::JSON::Object::Ptr clientObj = data->getObject("client");

            std::list<std::string> allowed;
            if (clientObj->has("allow")) {
                Poco::JSON::Array::Ptr clientRights = clientObj->getArray("allow");
                for (std::size_t idx = 0, limit = clientRights->size(); idx < limit; idx++) {
                    std::string clientId = clientRights->get(idx);
                    allowed.push_back(clientId);
                }
            }

            std::list<std::string> denied;
            if (clientObj->has("deny")) {
                Poco::JSON::Array::Ptr clientRights = clientObj->getArray("deny");
                for (std::size_t idx = 0, limit = clientRights->size(); idx < limit; idx++) {
                    std::string clientId = clientRights->get(idx);
                    denied.push_back(clientId);
                }
            }

            std::shared_ptr<PermissionRightsClient> node_obj(new PermissionRightsClient(allowed,denied));
            user_return_data.client = node_obj;
        }
        else {
            user_return_data.client = nullptr;
        }

        /* ############### DEVICE LEVEL PERMISSION RIGHTS ////###############*/
        if (data->has("device")) {
            Poco::JSON::Object::Ptr deviceObj = data->getObject("device");

            std::list< std::shared_ptr<DeviceInfo> > allowed;
            std::list< std::shared_ptr<DeviceInfo> > denied;

            if (deviceObj->has("allow")) {
                Poco::JSON::Array::Ptr virtualDevices = deviceObj->getArray("allow");

                for (std::size_t idx = 0, limit = virtualDevices->size(); idx < limit; idx++) {
                    Poco::JSON::Object::Ptr thisDevice = virtualDevices->getObject(idx);
                    std::string deviceId = thisDevice->getValue<std::string>("deviceId");

                    std::string name;
                    if (thisDevice->has("name")) {
                        name = thisDevice->getValue<std::string>("name");
                    }

                    std::string prodUniqueId;
                    if (thisDevice->has("prodUniqueId")) {
                        prodUniqueId = thisDevice->getValue<std::string>("prodUniqueId");
                    }

                    std::shared_ptr<DeviceInfo> deviceRightsObj(new DeviceInfo(deviceId, name, prodUniqueId));
                    allowed.push_back(deviceRightsObj);
                }
            }

            if (deviceObj->has("deny")) {
                Poco::JSON::Array::Ptr virtualDevices = deviceObj->getArray("deny");

                for (std::size_t idx = 0, limit = virtualDevices->size(); idx < limit; idx++) {
                    Poco::JSON::Object::Ptr thisDevice = virtualDevices->getObject(idx);
                    std::string deviceId = thisDevice->getValue<std::string>("deviceId");

                    std::string name;
                    if (thisDevice->has("name")) {
                        name = thisDevice->getValue<std::string>("name");
                    }

                    std::string prodUniqueId;
                    if (thisDevice->has("prodUniqueId")) {
                        prodUniqueId = thisDevice->getValue<std::string>("prodUniqueId");
                    }

                    std::shared_ptr<DeviceInfo> deviceInfoObj(new DeviceInfo(deviceId, name, prodUniqueId));
                    denied.push_back(deviceInfoObj);
                }
            }

            std::shared_ptr<PermissionRightsDevice> permissionRightsDeviceObj(new PermissionRightsDevice(allowed, denied));
            user_return_data.device = permissionRightsDeviceObj;
        }
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Retrieve Permission Rights API method");
        }
    }
    catch (...) {
        throw CatenisClientError("Unexpected returned data from Retrieve Permission Rights API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseSetPermissionRights(SetPermissionRightsResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();
            user_return_data.success = data["success"].get_bool();

#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");
            user_return_data.success = data->getValue<bool>("success");
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Set Permission Rights API method");
        }
    }
    catch (...) {
        throw CatenisClientError("Unexpected returned data from Set Permission Rights API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseListNotificationEvents(ListNotificationEventsResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            for (auto const &notificationEvent : data) {
                user_return_data.notificationEvents[notificationEvent.first] = notificationEvent.second.get_str();
            }
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            std::vector<std::string> eventNameList;
            data->getNames(eventNameList);

            for (std::vector<std::string>::iterator eventNameIdx = eventNameList.begin(); eventNameIdx != eventNameList.end(); eventNameIdx++) {
                user_return_data.notificationEvents[*eventNameIdx] = data->getValue<std::string>(*eventNameIdx);
            }
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from List Notification Events API method");
        }
    }
    catch (...) {
        throw CatenisClientError("Unexpected returned data from List Notification Events API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseCheckEffectivePermissionRight(CheckEffectivePermissionRightResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            for (auto const &entry : data) {
                user_return_data.effectivePermissionRight[entry.first] = entry.second.get_str();
            }
#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            std::vector<std::string> entries;
            data->getNames(entries);

            for (std::vector<std::string>::iterator entryIdx = entries.begin(); entryIdx != entries.end(); entryIdx++) {
                user_return_data.effectivePermissionRight[*entryIdx] = data->getValue<std::string>(*entryIdx);
            }
#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Check Effective Permission Right API method");
        }
    }
    catch (...) {
        throw CatenisClientError("Unexpected returned data from Check Effective Permission Right API method");
    }
}

// Private Method.
void ctn::CtnApiInternals::parseRetrieveDeviceIdInfo(DeviceIdInfoResult &user_return_data, std::string json_data)
{
    try {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
        json_spirit::mValue result;
        json_spirit::read_string_or_throw(json_data, result);

        json_spirit::mObject &retObj = result.get_obj();

        std::string const &status = retObj["status"].get_str();
#elif defined(COM_SUPPORT_LIB_POCO)
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json_data);

        Poco::JSON::Object::Ptr retObj = result.extract<Poco::JSON::Object::Ptr>();

        std::string status = retObj->getValue<std::string>("status");
#endif

        if (status == "success") {
#if defined(COM_SUPPORT_LIB_BOOST_ASIO)
            json_spirit::mObject &data = retObj["data"].get_obj();

            // CATENIS NODE INFO
            if (data.find("catenisNode") != data.end()) {
                json_spirit::mObject &ctnNode = data["catenisNode"].get_obj();

                int ctnNodeIdx = ctnNode["ctnNodeIndex"].get_int();
                //int ctnNodeIdx = 112;

                std::string ctnNodeName;
                if (ctnNode.find("name") != ctnNode.end()) {
                    ctnNodeName = ctnNode["name"].get_str();
                }

                std::string ctnNodeInfo;
                if (ctnNode.find("description") != ctnNode.end()) {
                    ctnNodeInfo = ctnNode["description"].get_str();
                }

                std::shared_ptr<CatenisNodeInfo> ctnNodeObj(new CatenisNodeInfo(ctnNodeIdx, ctnNodeName, ctnNodeInfo));
                user_return_data.catenisNode = ctnNodeObj;
            }
            else {
                user_return_data.catenisNode = nullptr;
            }

            // CLIENT INFO
            if (data.find("client") != data.end()) {
                json_spirit::mObject &client = data["client"].get_obj();

                std::string const &clientId = client["clientId"].get_str();

                std::string clientName;
                if (client.find("name") != client.end()) {
                    clientName = client["name"].get_str();
                }

                std::shared_ptr<ClientInfo> clientObj(new ClientInfo(clientId, clientName));
                user_return_data.client = clientObj;
            }
            else {
                user_return_data.client = nullptr;
            }

            // DEVICE INFO
            if (data.find("device") != data.end()) {
                json_spirit::mObject &thisDevice = data["device"].get_obj();

                std::string const &deviceId = thisDevice["deviceId"].get_str();

                std::string deviceName;
                if (thisDevice.find("name") != thisDevice.end()) {
                    deviceName = thisDevice["name"].get_str();
                }

                std::string prodUniqueId;
                if (thisDevice.find("prodUniqueId") != thisDevice.end()) {
                    prodUniqueId = thisDevice["prodUniqueId"].get_str();
                }
                std::shared_ptr<DeviceInfo> deviceObj(new DeviceInfo(deviceId, deviceName, prodUniqueId));
                user_return_data.device = deviceObj;
            }
            else {
                user_return_data.device = nullptr;
            }

#elif defined(COM_SUPPORT_LIB_POCO)
            Poco::JSON::Object::Ptr data = retObj->getObject("data");

            // CATENIS NODE INFO
            if (data->has("catenisNode")) {
                Poco::JSON::Object::Ptr ctnNode = data->getObject("catenisNode");

                int ctnNodeIdx = ctnNode->getValue<int>("ctnNodeIndex");

                std::string ctnNodeName;
                if (ctnNode->has("name")) {
                    ctnNodeName = ctnNode->getValue<std::string>("name");
                }

                std::string ctnNodeInfo;
                if (ctnNode->has("description")) {
                    ctnNodeInfo = ctnNode->getValue<std::string>("description");
                }

                std::shared_ptr<CatenisNodeInfo> ctnNodeObj(new CatenisNodeInfo(ctnNodeIdx, ctnNodeName, ctnNodeInfo));
                user_return_data.catenisNode = ctnNodeObj;
            }
            else {
                user_return_data.catenisNode = nullptr;
            }

            // CLIENT INFO
            if (data->has("client")) {
                Poco::JSON::Object::Ptr client = data->getObject("client");

                std::string clientId = client->getValue<std::string>("clientId");

                std::string clientName;
                if (client->has("name")) {
                    clientName = client->getValue<std::string>("name");
                }

                std::shared_ptr<ClientInfo> clientObj(new ClientInfo(clientId, clientName));
                user_return_data.client = clientObj;
            }
            else {
                user_return_data.client = nullptr;
            }

            // DEVICE INFO
            if (data->has("device")) {
                Poco::JSON::Object::Ptr device = data->getObject("device");

                std::string deviceId = device->getValue<std::string>("deviceId");

                std::string deviceName;
                if (device->has("name")) {
                    deviceName = device->getValue<std::string>("name");
                }

                std::string prodUniqueId;
                if (device->has("prodUniqueId")) {
                    prodUniqueId = device->getValue<std::string>("prodUniqueId");
                }

                std::shared_ptr<DeviceInfo> deviceObj(new DeviceInfo(deviceId, deviceName, prodUniqueId));
                user_return_data.device = deviceObj;
            }
            else {
                user_return_data.device = nullptr;
            }

#endif
        }
        else {
            throw CatenisClientError("Unexpected returned data from Retrive Device ID Information API method");
        }
    }
    catch (...) {
        throw CatenisClientError("Unexpected returned data from Retrive Device ID Information API method");
    }
}
