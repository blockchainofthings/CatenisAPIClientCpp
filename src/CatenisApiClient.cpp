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

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

int main()
{
    ctn::CtnApiClient *client = new ctn::CtnApiClient("abc", "def");
    
    
    return 0;
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
