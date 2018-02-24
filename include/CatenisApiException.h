//  CatenisApiException.h
//
//  Created by R. Benson Evans on 2/20/2018.
//
#ifndef __CATENISAPIEXCEPTION_H__
#define __CATENISAPIEXCEPTION_H__

#include <string>
#include <iostream>
#include <sstream>

/*
 * Catenis Exceptions to be thrown on errors.
 *
 * @member errorMessage : A description of the error.
 * @member isClientError : Is this a client or server error?
 * @member httpStatusCode : Is this an http server error; the error code.
 */
class CatenisAPIException
{
    public:
    CatenisAPIException(std::string error_message, bool is_client_error=false, int http_status_code=0) 
        : errorMessage(error_message), isClientError(is_client_error), httpStatusCode(http_status_code) {}
    ~CatenisAPIException() {}

    bool getIsClientError() {return(isClientError);}
    int getHttpStatusCode() {return(httpStatusCode);}
    std::string getErrorMessage() {return(errorMessage);}

    std::string getErrorDescription() 
    {
        if (isClientError)
            return("Client Error: " + errorMessage + ".");

        std::ostringstream oss; 
        oss << "Error Message: " << errorMessage << "; Http status code: " << httpStatusCode << ".";
        return(oss.str());
    }

    private:
	std::string errorMessage;
	bool isClientError;
	int httpStatusCode;
};

/*
 * Catenis Exceptions to be thrown on errors by the API internals methods.
 *
 * @member errorMessage : A description of the error.
 * @member isClientError : Is this a client or server error?
 * @member httpStatusCode : Is this an http server error; the error code.
 */
class CatenisAPIInternalsError : public CatenisAPIException
{
    public:
    CatenisAPIInternalsError(std::string error_message, bool is_client_error=false, int http_status_code=0) 
        : CatenisAPIException(error_message, is_client_error, http_status_code) {} 
    ~CatenisAPIInternalsError() {}
};

/*
 * Catenis Exceptions to be thrown on errors by the API client methods.
 *
 * @member errorMessage : A description of the error.
 * @member isClientError : Is this a client or server error?
 * @member httpStatusCode : Is this an http server error; the error code.
 */
class CatenisAPIClientError : public CatenisAPIException
{
    public:
    CatenisAPIClientError(std::string error_message, bool is_client_error=false, int http_status_code=0) 
        : CatenisAPIException(error_message, is_client_error, http_status_code) {} 
    ~CatenisAPIClientError() {}
};
#endif
