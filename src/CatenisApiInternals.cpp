//
//  CatenisApiInternals.cpp
//  CatenisAPIClientCpp
//
//  Created by Sungwoo Bae on 6/21/17.
//

#include <CatenisApiInternals.h>

#include <iostream>
#include <string>
#include <ctime>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <iomanip>

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/Context.h>


// http request
bool ctn::CtnApiInternals::httpRequest(std::string verb, std::string methodpath, std::map<std::string, std::string> &params, std::map<std::string, std::string> &queries, Poco::JSON::Object &request_data, std::string &response_data)
{
    bool success = true;
    
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
    
    // Add entire method path
    methodpath = this->root_api_endpoint_ + "/" + methodpath;
    
    // Contruct url
    std::string url = "";
    std::string prefix = (this->secure_ ? "https://" : "http://");
    url = prefix + this->host_;
    if(this->port_ != "") url = url + ":" + this->port_;
    url = url + methodpath;
    
    // Contruct Request JSON if POST
    std::string payload_json = "";
    if(verb == "POST")
    {
        std::ostringstream oss;
        Poco::JSON::Stringifier::stringify(request_data, oss);
        payload_json = oss.str();
    }
    
    std::cout << payload_json << std::endl;
    
    // Create neccesary headers
    time_t now = std::time(0);
    char iso_time[17];
    strftime(iso_time, sizeof iso_time, "%Y%m%dT%H%M%SZ", gmtime(&now));
    std::map<std::string, std::string> headers;
    
    headers["host"] = this->host_;
    headers[TIME_STAMP_HDR] = std::string(iso_time);
    
    // Create signature and add to header
    signRequest(verb, methodpath, headers, payload_json, now);
    
    try
    {
        Poco::URI uri(url);
        // HTTP
        Poco::Net::HTTPClientSession http_session(uri.getHost(), uri.getPort());
        // HTTPS
        const Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_NONE, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
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
        request.setKeepAlive(true);
        
        // Send Request
        request.setContentLength(payload_json.length());
        if(this->secure_) ssl_session.sendRequest(request) << payload_json;
        else http_session.sendRequest(request) << payload_json;
        
        request.write(std::cout);
        
        // Get response
        Poco::Net::HTTPResponse res;
        std::string responseStr;
        if(this->secure_) Poco::StreamCopier::copyToString(ssl_session.receiveResponse(res), responseStr);
        else Poco::StreamCopier::copyToString(http_session.receiveResponse(res), responseStr);
        
        unsigned int status_code = res.getStatus();
        if (status_code != 200)
        {
            std::cerr << "Error: " << status_code << " ";
            std::cerr << res.getReason() << std::endl;
            success = false;
        }
        
        response_data = responseStr;
    }
    catch (Poco::Exception &ex)
    {
        success = false;
        std::cerr << ex.displayText() << std::endl;
    }
    
    return success;
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
